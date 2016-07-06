// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "donatedialog.h"
#include "ui_donatedialog.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "addresstablemodel.h"
#include "main.h"
#include <QMessageBox>
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif

#include <QDebug>

DonateDialog::DonateDialog(QWidget *parent, BitScreen *screen) :
    QDialog(parent),
    ui(new Ui::DonateDialog), screen(screen)
{
    ui->setupUi(this);
    setWindowTitle(tr("Donate to ArtBoomerang"));

#ifdef USE_FULLSCREEN
    ui->buttonsLayout->removeItem(ui->horizontalSpacer);
    ui->buttonsLayout->removeItem(ui->horizontalSpacer_2);
    ui->verticalLayout_2->removeItem(ui->verticalSpacer);
    ActionBar *actionBar =new ActionBar(this);
    ui->actionBarLayout->addWidget(actionBar);
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));
    actionBar->setTitle(windowTitle(), true);

    screenOrientationChanged();
    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
#else
    ui->verticalLayout_2->removeItem(ui->verticalSpacer_2);

#endif
    ui->lineAddress->setText(QString(d_address));
    if(d_address.isEmpty()){
        ui->lineAddress->setText(tr("Loading address..."));
    }
    ui->btnSend->setEnabled(false);
    ui->donateAmount->setEnabled(false);
    connect(this, SIGNAL(windowReady()), this, SLOT(preloadDonationAddress()), Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(this, SIGNAL(addressReady()), this, SLOT(updateDonationAddress()));

}

DonateDialog::~DonateDialog()
{
    delete ui;
    m_networkAccessManager.deleteLater();
}

void DonateDialog::updateDonationAddress()
{
    ui->lineAddress->setText(d_address);
    ui->btnSend->setEnabled(true);
    ui->donateAmount->setEnabled(true);

}

void DonateDialog:: preloadDonationAddress()
{
    if(d_address.isEmpty()){
        QString url = "https://raw.githubusercontent.com/artboomerangwin/artboomerang/master/donation.txt";
        connect(&m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(readReply(QNetworkReply*)));
        m_networkAccessManager.get( QNetworkRequest( QUrl( url ) ) );
    }
    else{
        emit addressReady();
    }
}

void DonateDialog::readReply(QNetworkReply * reply)
{
    if( reply->error() == QNetworkReply::NoError )
    {
            QByteArray ba(reply->readAll());
            d_address = QString(ba);
            emit addressReady();
    }
    reply = NULL;
    //reply->deleteLater();
    delete reply;
}


void DonateDialog::screenOrientationChanged()
{
#ifdef USE_FULLSCREEN
    setFixedSize(screen->virtualSize());
    int w = (screen->size().width()-(ui->verticalLayout_2->contentsMargins().left()*2)-9)/2;
    ui->btnSend->setFixedWidth(w);
    ui->btnCancel->setFixedWidth(w);
#endif

}
void DonateDialog::setModel(WalletModel *model)
{
    this->model = model;
}

void DonateDialog::on_btnCancel_clicked(){
    reject();
}

void DonateDialog::on_btnSend_clicked()
{
    if(!model)
        return;

    if(d_address.isEmpty()){
        QMessageBox::information(this, tr("Send Coins"),
            tr("Please wait. Donation address not updated from the network."),
            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if(model->getBalance()<=0){
        QMessageBox::information(this, tr("Send Coins"),
                                 tr("Nothing to send. Wallet amount is empty."),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QList<SendCoinsRecipient> recipients;
    SendCoinsRecipient rv;
    rv.address = d_address;
    rv.narration = tr("Donation");
    rv.typeInd = AddressTableModel::AT_Normal;
    rv.amount = ui->donateAmount->value();
    recipients.append(rv);
    QMessageBox::StandardButton retval ;

    if(model->getBalance()<=ui->donateAmount->value())
    {
        QMessageBox::StandardButton sendAll = QMessageBox::question(this, tr("Confirm send coins"),
                         tr("Amount of %1 exceeds existing balance.\nDo you want to send all %2?")
                            .arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, rv.amount))
                            .arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, model->getBalance()),
              QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel));

         if(sendAll == QMessageBox::Yes){
             rv.amount = model->getBalance()-nTransactionFee;
             retval = QMessageBox::question(this, tr("Confirm send coins"),
                              tr("Please confirm if You want to donate %1?")
                                            .arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, rv.amount)),
                   QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);
         }else{
             reject();
             return;
         }
    }else{

        retval = QMessageBox::question(this, tr("Confirm send coins"),
                         tr("Please confirm if You want to donate %1?")
                                       .arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, rv.amount)),
              QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);
    }

    if(retval != QMessageBox::Yes)
    {
        reject();
        return;
    }
    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        reject();
        return;
    }
    WalletModel::SendCoinsReturn sendstatus;
    sendstatus = model->sendCoins(recipients);
    switch(sendstatus.status)
    {
    case WalletModel::DescriptionTooLong:
        reject();
        break;
    case WalletModel::DuplicateAddress:
        reject();
        break;
    case WalletModel::Aborted: // User aborted, nothing to do
        reject();
        break;
    case WalletModel::InvalidAddress:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The recipient address is not valid, please recheck."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::InvalidAmount:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The amount to pay must be larger than 0."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountExceedsBalance:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The amount exceeds your balance."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::AmountWithFeeExceedsBalance:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("The total exceeds your balance when the %1 transaction fee is included.").
            arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, sendstatus.fee)),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCreationFailed:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("Error: Transaction creation failed."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::TransactionCommitFailed:
        QMessageBox::warning(this, tr("Send Coins"),
            tr("Error: The transaction was rejected. This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here."),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    case WalletModel::OK:
        accept();
        QMessageBox::information(this, tr("Send Coins"),
            tr("Thank you for supporting ArtBoomerang project!"),
            QMessageBox::Ok, QMessageBox::Ok);
        break;
    }
}

void DonateDialog::accept()
{
    QDialog::accept();
}

void DonateDialog::keyPressEvent(QKeyEvent *event)
{
#ifdef USE_FULLSCREEN
    if(windowType() != Qt::Widget && event->key() == Qt::Key_Back)
    {
        close();
    }
#else
    if(windowType() != Qt::Widget && event->key() == Qt::Key_Escape)
    {
        close();
    }
#endif
}


void DonateDialog::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    emit windowReady();
}

