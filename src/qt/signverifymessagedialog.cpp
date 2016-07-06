// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "signverifymessagedialog.h"
#include "ui_signverifymessagedialog.h"

#include "addressbookpage.h"
#include "base58.h"
#include "guiutil.h"
#include "init.h"
#include "main.h"
#include "optionsmodel.h"
#include "walletmodel.h"
#include "wallet.h"

#include <string>
#include <vector>

#include <QClipboard>
#include <QKeyEvent>
#include <QTouchDevice>
#include <QTouchEvent>
#include <QDebug>

SignVerifyMessageDialog::SignVerifyMessageDialog(QWidget *parent, BitScreen *screen) :
    QDialog(parent),
    ui(new Ui::SignVerifyMessageDialog),
    model(0), screen(screen)
{
    setStyleSheet("QTabWidget::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");
    ui->setupUi(this);
    this->setObjectName("SignVerifyMessage");
    this->clearAll();

    setWindowTitle(tr("ArtBoomerang Signatures"));



#ifdef USE_FULLSCREEN
    actionBar =new ActionBar(this);
    ui->actionBarLayout->addWidget(actionBar);
    QAction *clearAction = new QAction(QIcon(":/android_icons/action_undo"), tr("&Clear All"), this);
    connect(clearAction, SIGNAL(triggered()), qApp->inputMethod(), SLOT(hide()));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearAll()));
    actionBar->addButton(clearAction);
    actionBar->setTitle(windowTitle(), true);
    connect(actionBar, SIGNAL(up()), this, SLOT(clearAll()));
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));

    //  int em=fontMetrics().height();
    QFont font = ui->infoLabel_SM->font(); font.setPointSize(11);
    ui->infoLabel_SM->setFont(font);
    ui->infoLabel_VM->setFont(font);
    ui->addressBookButton_SM->setIconSize(screen->iconSize());
    ui->addressBookButton_VM->setIconSize(screen->iconSize());
    ui->pasteButton_SM->setIconSize(screen->iconSize());
    ui->pasteButton_VM->setIconSize(screen->iconSize());
    ui->pasteButtonVM2->setIconSize(screen->iconSize());
    ui->verifyMessageButton_VM->setIconSize(screen->iconSize());
    ui->signMessageButton_SM->setIconSize(screen->iconSize());
    ui->copySignatureButton_SM->setIconSize(screen->iconSize());

    ui->btnClearSM->hide();ui->btnClearVM->hide();
    ui->btnCloseSM->hide();ui->btnCloseVM->hide();
    //screenOrientationChanged();
    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));

#endif

#if (QT_VERSION >= 0x040700)
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->addressIn_SM->setPlaceholderText(tr("Enter an ArtBoomerang address (e.g. FfCDhekqN2ngKCfKLyxQodtAUfCQVPqFFt)"));
    ui->signatureOut_SM->setPlaceholderText(tr("Click \"Sign Message\" to generate signature"));

    ui->addressIn_VM->setPlaceholderText(tr("Enter an ArtBoomerang address (e.g. FfCDhekqN2ngKCfKLyxQodtAUfCQVPqFFt)"));
    ui->signatureIn_VM->setPlaceholderText(tr("Enter ArtBoomerang signature"));
#endif

    GUIUtil::setupAddressWidget(ui->addressIn_SM, this);
    GUIUtil::setupAddressWidget(ui->addressIn_VM, this);

    ui->addressIn_SM->installEventFilter(this);
    ui->messageIn_SM->installEventFilter(this);
    ui->signatureOut_SM->installEventFilter(this);
    ui->addressIn_VM->installEventFilter(this);
    ui->messageIn_VM->installEventFilter(this);
    ui->signatureIn_VM->installEventFilter(this);

    ui->signatureOut_SM->setFont(GUIUtil::bitcoinAddressFont());
    ui->signatureIn_VM->setFont(GUIUtil::bitcoinAddressFont());

}
void SignVerifyMessageDialog::screenOrientationChanged(){

#ifdef USE_FULLSCREEN
    setFixedSize(screen->virtualSize());
#endif

/*
 * The "Sign Message" feature is an advanced functionality of the Bitcoin client which allows you to sign arbitrary messages to prove to somebody that you are (were) in control of the funds of some Bitcoin address.
 * The signing mechanism is a way of proving that a particular message was signed by the holder of an address' private key. A merchant could ask that you sign a message stating where you want your order shipped to, using one of the addresses your payment originated from.
 * You should sign a statement saying "I, Jane Doe (jane.doe@email.com) sent 1.23 BTC to Acme Corp at 12:34pm, 1st Jan 2012 in payment for product XYZ for delivery to 456 High Street, Anytown, USA".

You shouldn't sign a vague statement saying "yes, I sent that money; send the product to the address I emailed you", because anyone seeing a copy of that signed message can then pass that on to the merchant with his own postal address and get the product you paid for, in the same way as you wouldn't put your signature to a piece of paper saying "I agree to the above" where the above was left blank. The postal address part won't be signed, but perhaps the merchant won't care.
  */
}
SignVerifyMessageDialog::~SignVerifyMessageDialog()
{
    delete ui;
}

void SignVerifyMessageDialog::setModel(WalletModel *model)
{
    this->model = model;
}

void SignVerifyMessageDialog::setAddress_SM(QString address)
{
    ui->addressIn_SM->setText(address);
    ui->messageIn_SM->setFocus();
}

void SignVerifyMessageDialog::setAddress_VM(QString address)
{
    ui->addressIn_VM->setText(address);
    ui->messageIn_VM->setFocus();
}

void SignVerifyMessageDialog::showTab_SM(bool fShow)
{
    ui->tabWidget->setCurrentIndex(0);
    if (fShow){
#ifdef USE_FULLSCREEN
        screenOrientationChanged();

#endif
        ui->tabWidget->setTabEnabled(1, false);
        ui->tabWidget->setTabEnabled(0, true);
        this->show();
    }

}

void SignVerifyMessageDialog::showTab_VM(bool fShow)
{
    ui->tabWidget->setCurrentIndex(1);
    if (fShow){
#ifdef USE_FULLSCREEN
        screenOrientationChanged();
#endif
        ui->tabWidget->setTabEnabled(0, false);
        ui->tabWidget->setTabEnabled(1, true);
        this->show();
    }
}

void SignVerifyMessageDialog::on_addressBookButton_SM_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this, screen);
        dlg.setIsDialogMode(true);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_SM(dlg.getReturnValue());
        }
    }
}

void SignVerifyMessageDialog::on_addressBookButton_VM_clicked()
{

    if (model && model->getAddressTableModel())
    {
        AddressBookPage *dlg = new AddressBookPage(AddressBookPage::ForSending, AddressBookPage::SendingTab, this, screen);
        dlg->setIsDialogMode(true);
        dlg->setModel(model->getAddressTableModel());
        if (dlg->exec())
        {
            setAddress_VM(dlg->getReturnValue());
        }
    }
}
void SignVerifyMessageDialog::on_pasteButton_SM_clicked()
{
    setAddress_SM(QApplication::clipboard()->text());
}
void SignVerifyMessageDialog::on_pasteButton_VM_clicked()
{
    setAddress_VM(QApplication::clipboard()->text());
}

void SignVerifyMessageDialog::on_pasteButtonVM2_clicked()
{
    ui->signatureIn_VM->setText(QApplication::clipboard()->text());
}
void SignVerifyMessageDialog::on_btnClearVM_clicked()
{
    clearAll();
}
void SignVerifyMessageDialog::on_btnClearSM_clicked()
{
    clearAll();
}
void SignVerifyMessageDialog::on_btnCloseVM_clicked()
{
    clearAll(); close();
}
void SignVerifyMessageDialog::on_btnCloseSM_clicked()
{
    clearAll(); close();
}

void SignVerifyMessageDialog::on_signMessageButton_SM_clicked()
{
    /* Clear old signature to ensure users don't get confused on error with an old signature displayed */
    ui->signatureOut_SM->clear();

    if(ui->messageIn_SM->toPlainText().trimmed().isEmpty()){
        QMessageBox::critical(this, tr("Error Signing Message"),
            tr("Entered signing message is empty.\nPlease type some word/phrase to sign address with."),
            QMessageBox::Ok);
        return;
    }

    CBitcoinAddress addr(ui->addressIn_SM->text().toStdString());
    if (!addr.IsValid())
    {
        ui->addressIn_SM->setValid(false);
        QMessageBox::critical(this, tr("Error Sign Message"),
            tr("The entered address is invalid.") + QString(" ")
            + tr("Please check the address and try again."),
            QMessageBox::Ok);
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_SM->setValid(false);
        QMessageBox::critical(this, tr("Error Sign Message"),
            tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."),
            QMessageBox::Ok);
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid())
    {
        QMessageBox::critical(this, tr("Error Sign Message"),
            tr("Wallet unlock was cancelled."),
            QMessageBox::Ok);
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        QMessageBox::critical(this, tr("Error Sign Message"),
            tr("Private key for the entered address is not available."),
            QMessageBox::Ok);
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_SM->document()->toPlainText().toStdString();

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
    {
        QMessageBox::critical(this, tr("Error Sign Message"),
            tr("Message signing failed."),
            QMessageBox::Ok);
        return;
    }

    QMessageBox::information(this, tr("Success Sign Message"),
        tr("Message signed."),
        QMessageBox::Ok);

    ui->signatureOut_SM->setText(QString::fromStdString(EncodeBase64(&vchSig[0], vchSig.size())));
}

void SignVerifyMessageDialog::on_copySignatureButton_SM_clicked()
{
    QApplication::clipboard()->setText(ui->signatureOut_SM->text());
}



void SignVerifyMessageDialog::on_verifyMessageButton_VM_clicked()
{
    if(ui->messageIn_VM->toPlainText().trimmed().isEmpty()){
        QMessageBox::critical(this, tr("Error Verify Message"),
            tr("The entered message is empty.\nPlease enter required message to verify address."),
            QMessageBox::Ok);
        return;
    }

    CBitcoinAddress addr(ui->addressIn_VM->text().toStdString());
    if (!addr.IsValid())
    {
        ui->addressIn_VM->setValid(false);
        QMessageBox::critical(this, tr("Error Verify Message"),
            tr("The entered address is invalid.") + QString(" ")
            + tr("Please check the address and try again."),
            QMessageBox::Ok);
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_VM->setValid(false);
        QMessageBox::critical(this, tr("Error Verify Message"),
            tr("The entered address does not refer to a key.") + QString(" ")
            + tr("Please check the address and try again."),
            QMessageBox::Ok);
        return;
    }

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(ui->signatureIn_VM->text().toStdString().c_str(), &fInvalid);

    if (fInvalid)
    {
        ui->signatureIn_VM->setValid(false);
        QMessageBox::critical(this, tr("Error Verify Message"),
            tr("The signature could not be decoded.") + QString(" ")
            + tr("Please check the signature and try again."),
            QMessageBox::Ok);
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_VM->document()->toPlainText().toStdString();

    CKey key;
    if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
    {
        ui->signatureIn_VM->setValid(false);
        QMessageBox::critical(this, tr("Error Verify Message"),
            tr("The signature did not match the message digest.") + QString(" ")
            + tr("Please check the signature and try again."),
            QMessageBox::Ok);
        return;
    }

    if (!(CBitcoinAddress(key.GetPubKey().GetID()) == addr))
    {
        QMessageBox::critical(this, tr("Error Verify Message"),
            tr("Message verification failed."),
            QMessageBox::Ok);
        return;
    }

    QMessageBox::information(this, tr("Success Verify Message"),
        tr("Message verified."),
        QMessageBox::Ok);
}

void SignVerifyMessageDialog::clearAll()
{

    if (ui->tabWidget->currentIndex() == 0)
    {
        ui->addressIn_SM->setFocus();
    }
    else{
        ui->addressIn_VM->setFocus();
    }
    ui->addressIn_SM->clear();
    ui->messageIn_SM->clear();
    ui->signatureOut_SM->clear();
    ui->addressIn_VM->clear();
    ui->messageIn_VM->clear();
    ui->signatureIn_VM->clear();

}

bool SignVerifyMessageDialog::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn)
    {
        if (ui->tabWidget->currentIndex() == 0)
        {
            /* Select generated signature */
            if (object == ui->signatureOut_SM)
            {
                ui->signatureOut_SM->selectAll();
                return true;
            }
        }
        else if (ui->tabWidget->currentIndex() == 1)
        {
        }
    }
    return QDialog::eventFilter(object, event);
}

void SignVerifyMessageDialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

}

void SignVerifyMessageDialog::keyPressEvent(QKeyEvent *event)
{
#ifdef USE_FULLSCREEN
    if(event->key() == Qt::Key_Back)
    {
        close();
    }
#else
    if(event->key() == Qt::Key_Escape)
    {
        close();
    }
#endif
}

