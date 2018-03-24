﻿#include "addtokendialog.h"
#include "ui_addtokendialog.h"

#include <QDebug>

#include "blockchain.h"
#include "rpcthread.h"
#include "../commondialog.h"


AddTokenDialog::AddTokenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTokenDialog)
{
    ui->setupUi(this);

    setParent(GDW::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");

    ui->contractAddressLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->contractAddressLineEdit->setTextMargins(8,0,0,0);
    ui->contractAddressLineEdit->setFocus();

    connect( GDW::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));
}

AddTokenDialog::~AddTokenDialog()
{
    delete ui;
}

void AddTokenDialog::pop()
{
    move(0,0);
    exec();
}

void AddTokenDialog::on_okBtn_clicked()
{
    if( GDW::getInstance()->addressMap.keys().size() < 1) return;
    QString contractAddress = ui->contractAddressLineEdit->text().simplified();
    if(contractAddress.isEmpty())   return;
    GDW::getInstance()->postRPC( toJsonFormat( "id_call_contract_offline_state_addtokendialog+" + contractAddress, "contract_call_offline", QStringList() << contractAddress
                                               << GDW::getInstance()->addressMap.keys().at(0) << "state" << ""
                                               ));
    qDebug() << "contract_call_offline " + contractAddress + " " + GDW::getInstance()->addressMap.keys().at(0) + " state \"\"";
}

void AddTokenDialog::on_cancelBtn_clicked()
{
    close();
}

void AddTokenDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith( "id_call_contract_offline_state_addtokendialog+") )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        qDebug() << result;

        if( result.startsWith( "\"error\":"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Wrong contract address!"));
            commonDialog.pop();
        }
        else if( result == "\"result\":\"NOT_INITED\"")
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Contract uninitialized!"));
            commonDialog.pop();
        }
        else
        {
            QString contractAddress = id.mid(46);
            if( !GDW::getInstance()->ERC20TokenInfoMap.keys().contains(contractAddress))
            {
                GDW::getInstance()->ERC20TokenInfoMap.insert(contractAddress,ERC20TokenInfo());
            }
            GDW::getInstance()->ERC20TokenInfoMap[contractAddress].contractAddress = contractAddress;
            GDW::getInstance()->configFile->setValue("/AddedContractToken/" + contractAddress,1);
            GDW::getInstance()->postRPC( toJsonFormat( "id_call_contract_offline_addtokendialog+tokenName+" + contractAddress, "call_contract_offline", QStringList() << contractAddress
                                                       << GDW::getInstance()->addressMap.keys().at(0) << "tokenName" << ""
                                                       ));

            GDW::getInstance()->postRPC( toJsonFormat( "id_call_contract_offline_addtokendialog+precision+" + contractAddress, "call_contract_offline", QStringList() << contractAddress
                                                       << GDW::getInstance()->addressMap.keys().at(0) << "precision" << ""
                                                       ));

            GDW::getInstance()->postRPC( toJsonFormat( "id_call_contract_offline_addtokendialog+admin+" + contractAddress, "call_contract_offline", QStringList() << contractAddress
                                                       << GDW::getInstance()->addressMap.keys().at(0) << "admin" << ""
                                                       ));

            GDW::getInstance()->postRPC( toJsonFormat( "id_call_contract_offline_addtokendialog+tokenSymbol+" + contractAddress, "call_contract_offline", QStringList() << contractAddress
                                                       << GDW::getInstance()->addressMap.keys().at(0) << "tokenSymbol" << ""
                                                       ));

            GDW::getInstance()->postRPC( toJsonFormat( "id_call_contract_offline_addtokendialog+totalSupply+" + contractAddress, "call_contract_offline", QStringList() << contractAddress
                                                       << GDW::getInstance()->addressMap.keys().at(0) << "totalSupply" << ""
                                                       ));

            CommonDialog commonDialog(CommonDialog::OkOnly);
//            commonDialog.setText(QString::fromLocal8Bit("合约资产添加成功!"));
            commonDialog.setText(tr("Token added!"));
            commonDialog.pop();

            close();
        }

        return;
    }

    if( id.startsWith( "id_call_contract_offline_addtokendialog+") )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);

        if( result.startsWith("\"result\":"))
        {
            int pos = 40;
            QString func = id.mid( pos, id.indexOf("+" , pos) - pos);
            QString contractAddress = id.mid(id.indexOf("+" , pos) + 1);

            if( func == "tokenName")
            {
                QString tokenName = result.mid(9);
                tokenName.remove('\"');
                GDW::getInstance()->ERC20TokenInfoMap[contractAddress].name = tokenName;
            }
            else if( func == "precision")
            {
                QString precision = result.mid(9);
                precision.remove('\"');
                GDW::getInstance()->ERC20TokenInfoMap[contractAddress].precision = precision.toULongLong();
            }
            else if( func == "admin")
            {
                QString admin = result.mid(9);
                admin.remove('\"');
                GDW::getInstance()->ERC20TokenInfoMap[contractAddress].admin = admin;
            }
            else if( func == "tokenSymbol")
            {
                QString tokenSymbol = result.mid(9);
                tokenSymbol.remove('\"');
                GDW::getInstance()->ERC20TokenInfoMap[contractAddress].symbol = tokenSymbol;
            }
            else if( func == "totalSupply")
            {
                QString totalSupply = result.mid(9);
                totalSupply.remove('\"');
                GDW::getInstance()->ERC20TokenInfoMap[contractAddress].totalSupply = totalSupply.toULongLong();
            }

        }

        return;
    }

}

void AddTokenDialog::on_contractAddressLineEdit_returnPressed()
{
    on_okBtn_clicked();
}
