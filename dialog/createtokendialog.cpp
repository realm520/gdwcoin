#include "createtokendialog.h"
#include "ui_createtokendialog.h"

#include "blockchain.h"
#include "rpcthread.h"
#include "commondialog.h"
#include "extra/style.h"

#ifdef WIN32
#include "Windows.h"
#endif

#include <QDebug>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListView>

CreateTokenDialog::CreateTokenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateTokenDialog)
{
    ui->setupUi(this);

    setParent(GDW::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->containerWidget->installEventFilter(this);

    connect(GDW::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

#ifdef WIN32
    ui->accountComboBox->setStyleSheet("QComboBox {border: 1px solid gray;border-radius: 3px;padding: 1px 2px 1px 8px;}"
                                           "QComboBox::drop-down {subcontrol-origin: padding;subcontrol-position: top right;width: 20px;"
                                                                  "border-left-width: 1px;border-left-color: darkgray;border-left-style: solid;"
                                                                  "border-top-right-radius: 3px;border-bottom-right-radius: 3px;}"
                                           "QComboBox::down-arrow {image: url(:/pic/pic2/comboBoxArrow.png);}"
                                           );
#endif

    ui->contractNameLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->contractDescriptionLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->totalSupplyLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->gasLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");

    ui->contractNameLineEdit->setTextMargins(8,0,0,0);
    ui->contractDescriptionLineEdit->setTextMargins(8,0,0,0);
    ui->totalSupplyLineEdit->setTextMargins(8,0,0,0);
    ui->gasLineEdit->setTextMargins(8,0,0,0);


    QRegExp rx1("[a-zA-Z0-9]{0,20}");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->contractNameLineEdit->setValidator(pReg1);
    ui->contractNameLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->totalSupplyLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    setTotalSupplyValidator();

//    ui->okBtn->setStyleSheet(CLOSEBTN_STYLE);
    ui->okBtn2->setStyleSheet(OKBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);
    ui->issueBtn->setStyleSheet(OKBTN_STYLE);
    ui->backBtn->setStyleSheet(CANCELBTN_STYLE);

    init();
}

CreateTokenDialog::~CreateTokenDialog()
{
    delete ui;
}

void CreateTokenDialog::pop()
{
    move(0,0);
    exec();
}

void CreateTokenDialog::jsonDataUpdated(QString id)
{
    if( id ==  "id_check_normal_token_name_contract_get_info" )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        ui->issueBtn->setEnabled(true);
        if(result.startsWith( "\"result\":"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("This contract name has been used! Please use another name."));
            commonDialog.pop();
        }
        else
        {
            GDW::getInstance()->postRPC(toJsonFormat(QString("id_contract_register"), "contract_register",
                                                     QStringList() << ui->accountComboBox->currentText() << "contract/token.gpc"
                                                     << ASSET_NAME << "5" ));
            ui->stackedWidget->setCurrentIndex(1);
            ui->waitingInfoLabel->setText(tr("Contract registering ..."));
            qDebug() << "id_check_normal_token_name_contract_get_info: " << result;
        }

        return;
    }

    if( id ==  "id_check_smart_token_name_contract_get_info" )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        if( result.startsWith( "\"result\":"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("This contract name has been used! Please use another name."));
            commonDialog.pop();
        }
        else
        {
            ui->stackedWidget->setCurrentIndex(1);
            ui->waitingInfoLabel->setText(tr("Contract registering ..."));
            qDebug() << "id_check_smart_token_name_contract_get_info: " << result;
        }

        return;
    }

    if( id ==  "id_contract_register" )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        qDebug() << "id_contract_register: " << result;
        if( result.startsWith( "\"result\":"))
        {
            contractAddress = result.mid(9);
            contractAddress.remove("\"");
            GDW::getInstance()->ERC20TokenInfoMap[contractAddress].contractAddress = contractAddress;
            GDW::getInstance()->configFile->setValue("/AddedContractToken/" + contractAddress,1);
            timerForRegister->start(5000);
        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);
            ui->waitingInfoLabel->setText(tr("register failed: ") + errorMessage);
            ui->okBtn2->show();
        }

        return;
    }

    if( id ==  "id_contract_upgrade" )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        qDebug() << "id_contract_upgrade: " << result;
        if( result.startsWith( "\"result\":"))
        {
            timerForUpgrade->start(1000);
        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            ui->waitingInfoLabel->setText(tr("upgrade failed: ") + errorMessage);
            ui->okBtn2->show();
        }

        return;
    }

    if( id ==  "id_contract_call-init_token-nomal" )
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        qDebug() << "id_contract_call-init_token-nomal: " << result;
        if( result.startsWith( "\"result\":"))
        {
            timerForInitToken->start(1000);
        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            ui->waitingInfoLabel->setText(tr("init_token failed: ") + errorMessage);
            ui->okBtn2->show();
        }

        return;
    }

    if( id == "id_createtoken_contract_get_info+" + contractAddress)
    {
        QString result = GDW::getInstance()->jsonDataValue(id);
        if( result.startsWith( "\"result\":"))
        {
            timerForRegister->stop();
            {
                QString contractName = ui->contractNameLineEdit->text();
                QString description  = ui->contractDescriptionLineEdit->text();
                GDW::getInstance()->postRPC( toJsonFormat( QString("id_contract_upgrade"), "contract_upgrade",
                                                               QStringList() << contractAddress << ui->accountComboBox->currentText()
                                                               << contractName << description
                                                               << ASSET_NAME << "5" ));
                ui->waitingInfoLabel->setText(tr("Contract registering ...\n"
                                                 "Contract already registered.\n"
                                                 "Registered address: %1\n"
                                                 "Upgrading contract...").arg(contractAddress)
                                              );
                qDebug() << "id_createtoken_contract_get_info: " << contractAddress << ": " << result;
            }
        }
        return;
    }
}


void CreateTokenDialog::on_backBtn_clicked()
{
    close();
}

void CreateTokenDialog::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    ui->balanceLabel->setText(getBigNumberString(GDW::getInstance()->accountBalanceMap.value(arg1).value(0),
                                                 GDW::getInstance()->assetInfoMap.value(0).precision) + " GDW");
}

void CreateTokenDialog::init()
{
    ui->stackedWidget->setCurrentIndex(0);

    QStringList keys = GDW::getInstance()->addressMap.keys();
    ui->accountComboBox->addItems( keys);
    ui->gasLineEdit->setReadOnly(true);
    ui->okBtn2->hide();

    timerForRegister = new QTimer(this);
    connect(timerForRegister,SIGNAL(timeout()),this,SLOT(onTimerForRegister()));

    timerForUpgrade = new QTimer(this);
    connect(timerForUpgrade,SIGNAL(timeout()),this,SLOT(onTimerForUpgrade()));

    timerForInitToken = new QTimer(this);
    connect(timerForInitToken,SIGNAL(timeout()),this,SLOT(onTimerForInitToken()));
}

void CreateTokenDialog::estimateGas()
{
}

QString CreateTokenDialog::intToPrecisionString(int precision)
{
    QString result = "1";
    while (precision > 0)
    {
        result.append("0");
        precision--;
    }

    return result;
}

QString CreateTokenDialog::addPrecisionString(QString supply, int precision)
{
    QString result = supply;
    while (precision > 0)
    {
        result.append("0");
        precision--;
    }

    return result;
}

QString CreateTokenDialog::getContractParamsString(QStringList params)
{
    QString result;
    foreach (QString param, params)
    {
        result += param;
        result += ",";
    }
    if( result.endsWith(","))
    {
        result.chop(1);
    }

    return result;
}

void CreateTokenDialog::setTotalSupplyValidator()
{
    int precisionNum = ui->precisionSpinBox->text().toInt();

    QRegExp rx(QString("^([1-9][0-9]{0,%1})?$|(^\\t?$)").arg(17 - precisionNum));
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    ui->totalSupplyLineEdit->setValidator(validator);
}


void CreateTokenDialog::on_issueBtn_clicked()
{
    if( ui->contractNameLineEdit->text().simplified().isEmpty())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Contract name can not be empty!"));
        commonDialog.pop();
        return;
    }

    if( ui->contractDescriptionLineEdit->text().simplified().isEmpty())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Contract description can not be empty!"));
        commonDialog.pop();
        return;
    }

    if( ui->totalSupplyLineEdit->text().simplified().isEmpty())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Total supply can not be empty!"));
        commonDialog.pop();
        return;
    }

    if( GDW::getInstance()->accountBalanceMap.value(ui->accountComboBox->currentText()).value(0) <
            20 * GDW::getInstance()->assetInfoMap.value(0).precision)
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("You don't have enough %1s!").arg(ASSET_NAME));
        commonDialog.pop();
        return;
    }

    // 先校验合约名能不能用
    GDW::getInstance()->postRPC( toJsonFormat( "id_check_normal_token_name_contract_get_info", "contract_get_info",
                                                   QStringList() << ui->contractNameLineEdit->text()));

    ui->issueBtn->setEnabled(false);

}

void CreateTokenDialog::onTimerForRegister()
{
    GDW::getInstance()->postRPC( toJsonFormat( "id_createtoken_contract_get_info+" + contractAddress, "contract_get_info",
                                                   QStringList() << contractAddress
                                                   ));
}

void CreateTokenDialog::onTimerForUpgrade()
{
    GDW::getInstance()->postRPC(toJsonFormat("id_contract_get_info+" + contractAddress,
                                             "contract_get_info",
                                             QStringList() << contractAddress
                                             ));
    if(contractAddress.isEmpty())   return;
    if(!GDW::getInstance()->ERC20TokenInfoMap.contains(contractAddress))    return;
    if(GDW::getInstance()->ERC20TokenInfoMap.value(contractAddress).level == "forever")
    {
        timerForUpgrade->stop();
        {
            QString name = ui->contractNameLineEdit->text();
            QString symbol = ui->contractNameLineEdit->text();
            QString totalSupply = addPrecisionString(ui->totalSupplyLineEdit->text(),ui->precisionSpinBox->text().toInt());
            QString precision  = intToPrecisionString(ui->precisionSpinBox->text().toInt());
            QString paramString = getContractParamsString(QStringList() << name << symbol << totalSupply << precision);

            GDW::getInstance()->postRPC(toJsonFormat( QString("id_contract_call-init_token-nomal"), "contract_call",
                                                           QStringList() << contractAddress << ui->accountComboBox->currentText()
                                                           << "init_token" << paramString
                                                           << ASSET_NAME << "5" ));
            ui->waitingInfoLabel->setText(tr("Contract registering ...\n"
                                             "Contract already registered.\n"
                                             "Registered address: %1\n"
                                             "Upgrading contract...\n"
                                             "Contract upgraded.\n"
                                             "Initing contract...").arg(contractAddress)
                                          );
        }
    }
}


void CreateTokenDialog::onTimerForInitToken()
{
    GDW::getInstance()->postRPC(toJsonFormat("id_contract_get_info+" + contractAddress,
                                             "contract_get_info",
                                             QStringList() << contractAddress
                                             ));

    if(contractAddress.isEmpty())   return;
    if(!GDW::getInstance()->ERC20TokenInfoMap.contains(contractAddress))    return;
    if( GDW::getInstance()->ERC20TokenInfoMap.value(contractAddress).level == "forever")
    {
        if( GDW::getInstance()->ERC20TokenInfoMap.value(contractAddress).state != "NOT_INITED")
        {
            timerForInitToken->stop();
            ui->waitingInfoLabel->setText(tr("Contract registering ...\n"
                                             "Contract already registered.\n"
                                             "Registered address: %1\n"
                                             "Upgrading contract...\n"
                                             "Contract upgraded.\n"
                                             "Initing contract...\n"
                                             "Contract inited\n"
                                             "Contract created successfully!").arg(contractAddress)
                                          );
            ui->okBtn2->show();
//            GDW::getInstance()->addConcernedContract(contractAddress);
        }
    }
}


void CreateTokenDialog::on_okBtn2_clicked()
{
    close();
}

void CreateTokenDialog::on_precisionSpinBox_valueChanged(const QString &arg1)
{
//    ui->totalSupplyLineEdit->clear();
//    setTotalSupplyValidator();
}

void CreateTokenDialog::on_closeBtn_clicked()
{
    close();
}
