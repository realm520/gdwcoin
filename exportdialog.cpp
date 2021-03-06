#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "rpcthread.h"
#include "blockchain.h"
#include "commondialog.h"
#include "dialog/exportsetpwddialog.h"
#include "AES/aesencryptor.h"

#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QFile>


ExportDialog::ExportDialog( QString name, QWidget *parent) :
    QDialog(parent),
    accoutName(name),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);

    setParent(GDW::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet("#containerwidget{background-color: rgb(246, 246, 246);border:1px groove rgb(180,180,180);}");


    connect( GDW::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->nameLabel->setText( tr("Export ") + accoutName + tr(" \'s private key to:"));

    QDir dir;
#ifdef WIN32
    QString path = dir.currentPath().replace("/", "\\");
    ui->pathLineEdit->setText( path + "\\" + accoutName + ".hpk");
#else
    QString path = dir.currentPath();
    ui->pathLineEdit->setText( path + "/" + accoutName + ".hpk");
#endif
    ui->pathLineEdit->setStyleSheet("color:black;border:1px solid #CCCCCC;border-radius:3px;");
    ui->pathLineEdit->setTextMargins(8,0,0,0);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::pop()
{
    move(0,0);
    exec();

}

void ExportDialog::on_pathBtn_clicked()
{
    QString file = QFileDialog::getExistingDirectory(this,tr( "Select the path"));
    if( !file.isEmpty())
    {
#ifdef WIN32
        file.replace("/","\\");
        ui->pathLineEdit->setText( file + "\\" + accoutName + ".hpk");
#else
        ui->pathLineEdit->setText( file + "/"  + accoutName + ".hpk");
#endif
    }
}

void ExportDialog::on_cancelBtn_clicked()
{
    CommonDialog commonDialog(CommonDialog::OkAndCancel);
//    commonDialog.setText(QString::fromLocal8Bit("您还没有备份私钥，如果数据丢失/损坏 您将没有任何途径找回您的账户。是否确定取消备份?"));
    commonDialog.setText(tr("You have not backup this private key.If your wallet data is lost or corrupted, you will have no way to get your account back.Sure to cancel?"));
    if(commonDialog.pop())
    {
        close();
    }

//    emit accepted();
}

void ExportDialog::getPrivateKey()
{
//    RpcThread* rpcThread = new RpcThread;
//    connect(rpcThread,SIGNAL(finished()),rpcThread,SLOT(deleteLater()));
//    rpcThread->setLogin("a","b");
//    rpcThread->setWriteData( toJsonFormat( "id_wallet_dump_account_private_key_" + accoutName, "wallet_dump_account_private_key", QStringList() << accoutName << "0" ));
//    rpcThread->start();

    GDW::getInstance()->postRPC( toJsonFormat( "id_wallet_dump_account_private_key_" + accoutName, "wallet_dump_account_private_key", QStringList() << accoutName << "0" ));
}

void ExportDialog::jsonDataUpdated(QString id)
{
    if( id == "id_wallet_dump_account_private_key_" + accoutName)
    {
        QString result = GDW::getInstance()->jsonDataValue(id);

        if( result.mid(0,8) == "\"error\":")
        {
            qDebug() << "wallet_dump_account_private_key " + accoutName + " ERROR: " + result;
        }
        else
        {
            privateKey = result.mid(10,51);

            unsigned char key2[16] = {0};
            memcpy(key2,pwd.toLatin1().data(),pwd.toLatin1().size());
            AesEncryptor aes(key2);
            QString input = "privateKey=" + privateKey;
            QString output = QString::fromStdString( aes.EncryptString( input.toStdString()) );
            QFile file( ui->pathLineEdit->text());
            QString fName = file.fileName();
#ifdef WIN32
            fName.replace("/","\\");
            fName = fName.mid( fName.lastIndexOf("\\") + 1);
#else
            fName = fName.mid( fName.lastIndexOf("/") + 1 );
#endif
            if( file.exists())
            {
                CommonDialog tipDialog(CommonDialog::OkAndCancel);
                tipDialog.setText( fName + tr( " already exists!\nCover it or not?") );
                if ( !tipDialog.pop())  return;
            }

            if( !file.open(QIODevice::WriteOnly))
            {
                qDebug() << "privatekey file open " + fName + " ERROR";

                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr( "Wrong path!") );
                tipDialog.pop();
                return;
            }

            file.resize(0);
            QTextStream ts( &file);
            ts << output.toUtf8();
            file.close();

            close();
//            emit  accepted();

            CommonDialog tipDialog(CommonDialog::OkOnly);
//            tipDialog.setText( tr( "Export to ") + fName + tr(" succeeded!") + QString::fromLocal8Bit("请妥善保管您的私钥，绝对不要丢失或泄露给任何人!") );
            tipDialog.setText( tr( "Export to ") + fName + tr(" succeeded!") + tr("Please keep your private key properly.Never lose or leak it to anyone!") );
            tipDialog.pop();

            QString path = ui->pathLineEdit->text();
#ifdef WIN32
            path.replace( "/", "\\");
            path = path.left( path.lastIndexOf("\\") );

            QProcess::startDetached("explorer \"" + path + "\"");
#else
            path = path.left( path.lastIndexOf("/") );
            QProcess::startDetached("open \"" + path + "\"");
#endif
        }


        return;
    }
}

void ExportDialog::on_exportBtn_clicked()
{
    if( !ui->pathLineEdit->text().endsWith(".hpk"))
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Wrong file format!"));
        commonDialog.pop();
        return;
    }

    ExportSetPwdDialog exportSetPwdDialog;
    if( !exportSetPwdDialog.pop())  return;

    pwd = exportSetPwdDialog.pwd;

    getPrivateKey();
}
