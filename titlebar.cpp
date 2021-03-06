﻿#include <QDebug>

#include "titlebar.h"
#include "ui_titlebar.h"
#include "debug_log.h"
#include <QPainter>
#include "setdialog.h"
#include "consoledialog.h"
#include "consolewidget.h"
#include "blockchain.h"
#include "rpcthread.h"
#include "commondialog.h"

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    ui->setupUi(this);    

    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap(":/pic/cplpic/titleBg.png")));
    setPalette(palette);

    ui->newsBtn->hide();
    ui->newsBtn2->hide();

    ui->minBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/minimize2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                              "QToolButton:hover{background-image:url(:/pic/pic2/minimize_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
    ui->closeBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/close2.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}"
                                "QToolButton:hover{background-image:url(:/pic/pic2/close_hover.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
    ui->menuBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/set.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");
    ui->lockBtn->setStyleSheet("QToolButton{background-image:url(:/pic/pic2/lockBtn.png);background-repeat: repeat-xy;background-position: center;background-attachment: fixed;background-clip: padding;border-style: flat;}");

    connect( GDW::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));
    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    timer->setInterval(10000);
    timer->start();

    ui->versionLabel->setText( QString("GDW Wallet  v") + GDW::getInstance()->getVersion());

    onTimeOut();
	DLOG_QT_WALLET_FUNCTION_END;
}

TitleBar::~TitleBar()
{
	DLOG_QT_WALLET_FUNCTION_BEGIN;

    delete ui;

	DLOG_QT_WALLET_FUNCTION_END;
}

void TitleBar::on_minBtn_clicked()
{

#ifdef WIN32
    if( GDW::getInstance()->minimizeToTray)
    {
        emit tray();
        if( GDW::getInstance()->consoleWidget)
        {
            GDW::getInstance()->consoleWidget->hide();
        }
    }
    else
    {  
        emit minimum();
        if( GDW::getInstance()->consoleWidget)
        {
            GDW::getInstance()->consoleWidget->showMinimized();
        }
    }
#else
    emit minimum();
    if( GDW::getInstance()->consoleWidget)
    {
        GDW::getInstance()->consoleWidget->showMinimized();
    }
#endif
}

void TitleBar::on_closeBtn_clicked()
{
#ifdef WIN32
    if( GDW::getInstance()->closeToMinimize)
    {
        emit tray();
    }
    else
    {
        CommonDialog commonDialog(CommonDialog::OkAndCancel);
        commonDialog.setText( tr( "Sure to close GDW Wallet?"));
        bool choice = commonDialog.pop();

        if( choice)
        {
            emit closeWallet();
        }
        else
        {
            return;
        }

    }

#else
    CommonDialog commonDialog(CommonDialog::OkAndCancel);
    commonDialog.setText( tr( "Sure to close GDW Wallet?"));
    bool choice = commonDialog.pop();

    if( choice)
    {
        emit closeWallet();
    }
    else
    {
        return;
    }
#endif

}

void TitleBar::on_menuBtn_clicked()
{
    SetDialog setDialog;
    connect(&setDialog,SIGNAL(settingSaved()),this,SLOT(saved()));
    setDialog.pop();

}

void TitleBar::saved()
{
    emit settingSaved();
}

void TitleBar::retranslator()
{
    ui->retranslateUi(this);
}

void TitleBar::jsonDataUpdated(QString id)
{
    if( id == "id_blockchain_list_pending_transactions")
    {
        QString pendingTransactions = GDW::getInstance()->jsonDataValue(id);
        // 查询一遍 config中记录的交易ID
        mutexForConfigFile.lock();
        GDW::getInstance()->configFile->beginGroup("/recordId");
        QStringList keys = GDW::getInstance()->configFile->childKeys();
        GDW::getInstance()->configFile->endGroup();

        int numOfNews = 0;
        foreach (QString key, keys)
        {
            if( GDW::getInstance()->configFile->value("/recordId/" + key).toInt() == 2)
            {
                // 失效的交易
                numOfNews++;
                continue;
            }

            if( !pendingTransactions.contains(key))  // 如果不在pending区, 看看是否在链上
            {
                GDW::getInstance()->postRPC( toJsonFormat( "id_blockchain_get_transaction_" + key, "blockchain_get_transaction", QStringList() << key  ));
            }

            if( GDW::getInstance()->configFile->value("/recordId/" + key).toInt() == 1)
            {
                numOfNews++;
            }
        }
        mutexForConfigFile.unlock();

//        if( numOfNews == 0)
//        {
//            ui->newsBtn->hide();
//            ui->newsBtn2->hide();
//            ui->newsBtn2->setText( QString::number( numOfNews));
//        }
//        else
//        {
//            ui->newsBtn->show();
//            ui->newsBtn2->setText( QString::number( numOfNews));
//            ui->newsBtn2->show();
//        }

        return;
    }

    if( id.startsWith("id_blockchain_get_transaction"))
    {

        QString result = GDW::getInstance()->jsonDataValue(id);

        if( result.mid(0,22).contains("exception") || result.mid(0,22).contains("error"))
        {
            // 若不在pending区也不在链上  则为失效交易  recordId置为2
            mutexForConfigFile.lock();

            GDW::getInstance()->configFile->setValue("/recordId/" + id.right(40), 2);

            mutexForConfigFile.unlock();

            return;
        }
        else   //  如果已经被打包进区块，则将config中记录置为1
        {
            mutexForConfigFile.lock();

            GDW::getInstance()->configFile->setValue("/recordId/" + id.right(40), 1);

            mutexForConfigFile.unlock();
        }

        return;
    }

}

void TitleBar::onTimeOut()
{
//    RpcThread* rpcThread = new RpcThread;
//    connect(rpcThread,SIGNAL(finished()),rpcThread,SLOT(deleteLater()));
//    rpcThread->setWriteData( toJsonFormat( "id_blockchain_list_pending_transactions", "blockchain_list_pending_transactions", QStringList() << "" ));
//    rpcThread->start();
    GDW::getInstance()->postRPC( toJsonFormat( "id_blockchain_list_pending_transactions", "blockchain_list_pending_transactions", QStringList() << "" ));

    mutexForConfigFile.lock();
    GDW::getInstance()->configFile->beginGroup("/applyingForDelegateAccount");
    QStringList keys = GDW::getInstance()->configFile->childKeys();
    GDW::getInstance()->configFile->endGroup();
    foreach (QString key, keys)
    {
        // 如果申请代理的recordId 被删除了 或者被确认了（=1）或者失效了（=2） 则 删除applyingForDelegateAccount的记录
        if( !GDW::getInstance()->configFile->contains("/recordId/" + GDW::getInstance()->configFile->value("/applyingForDelegateAccount/" + key).toString())
            ||  GDW::getInstance()->configFile->value("/recordId/" + GDW::getInstance()->configFile->value("/applyingForDelegateAccount/" + key).toString()).toInt() != 0 )
        {
            GDW::getInstance()->configFile->remove("/applyingForDelegateAccount/" + key);
        }
    }

    GDW::getInstance()->configFile->beginGroup("/registeringAccount");
    keys = GDW::getInstance()->configFile->childKeys();
    GDW::getInstance()->configFile->endGroup();
    foreach (QString key, keys)
    {
        // 如果注册升级的recordId 被删除了 或者被确认了（=1）或者失效了（=2） 则 删除registeringAccount的记录
        if( !GDW::getInstance()->configFile->contains("/recordId/" + GDW::getInstance()->configFile->value("/registeringAccount/" + key).toString())
            ||  GDW::getInstance()->configFile->value("/recordId/" + GDW::getInstance()->configFile->value("/registeringAccount/" + key).toString()).toInt() != 0 )
        {
            GDW::getInstance()->configFile->remove("/registeringAccount/" + key);
        }
    }

    mutexForConfigFile.unlock();
}

void TitleBar::on_consoleBtn_clicked()
{
//    ConsoleDialog consoleDialog;
//    consoleDialog.pop();

    if( GDW::getInstance()->consoleWidget)
    {
        GDW::getInstance()->consoleWidget->show();
        GDW::getInstance()->consoleWidget->activateWindow();
    }
    else
    {
        GDW::getInstance()->consoleWidget = new ConsoleWidget;
        GDW::getInstance()->consoleWidget->setAttribute(Qt::WA_DeleteOnClose);
        GDW::getInstance()->consoleWidget->show();
    }


}

void TitleBar::paintEvent(QPaintEvent *)
{
//    QPainter painter(this);
//    painter.setBrush(QColor(51,38,87));
//    painter.setPen(QColor(51,38,87));
//    painter.drawRect(QRect(0,48,960,5));
}

void TitleBar::on_lockBtn_clicked()
{
    qDebug() << "TitleBar::on_lockBtn_clicked ";
    emit lock();
}
