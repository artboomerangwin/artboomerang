// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "editaddressdialog.h"
#include "ui_editaddressdialog.h"
#include "addresstablemodel.h"
#include "guiutil.h"
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QWindow>
#include <QScreen>
#include <QDebug>


EditAddressDialog::EditAddressDialog(Mode mode, QWidget *parent, BitScreen *screen)
    : QDialog( parent,  Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::EditAddressDialog), mapper(0), mode(mode), model(0), screen(screen)
{
    ui->setupUi(this);
    ui->labelHelpStealth->hide(); ui->labelStealthInfo->hide();
    screen->setLabelIcon(ui->labelStealthInfo, ":/icons/info");



    /*
    if(screen->isPortrait()){
        if(ui->layoutForLabel->isEmpty())
            ui->layoutForLabel->addWidget(ui->labelLabel);
        if(ui->layoutForAddress->isEmpty())
            ui->layoutForAddress->addWidget(ui->labelAddress);
    }else{
        if(!ui->layoutForLabel->isEmpty())
            ui->defaultLayoutForLabel->addWidget(ui->labelLabel);
        if(!ui->layoutForAddress->isEmpty())
            ui->defaultLayoutForAddress->addWidget(ui->labelAddress);
    }
    */

    switch(mode)
    {
    case NewReceivingAddress:
        setWindowTitle(tr("New receiving address"));
        ui->addressEdit->setEnabled(false);
        ui->addressEdit->setText(tr("Generated automatically"));
        ui->stealthCB->setEnabled(true);
        ui->stealthCB->setVisible(true);
        ui->pasteButton->setEnabled(false);
        ui->pasteButton->setVisible(false);
        break;
    case NewSendingAddress:
        setWindowTitle(tr("New sending address"));
        ui->labelAddress->show();
        ui->stealthCB->setVisible(false);
        ui->pasteButton->setEnabled(true);
        ui->pasteButton->setVisible(true);
        break;
    case EditReceivingAddress:
        setWindowTitle(tr("Edit receiving address"));
        ui->labelAddress->show();
        ui->addressEdit->setEnabled(false);
        ui->addressEdit->setVisible(true);
        ui->stealthCB->setEnabled(false);
        ui->stealthCB->setVisible(true);
        ui->pasteButton->setEnabled(false);
        ui->pasteButton->setVisible(false);
        break;
    case EditSendingAddress:
        setWindowTitle(tr("Edit sending address"));
        ui->labelAddress->show();
        ui->stealthCB->setVisible(false);
        ui->pasteButton->setEnabled(true);
        ui->pasteButton->setVisible(true);
        break;
    }
    //ui->stealthCB->setChecked(false);
#ifdef USE_FULLSCREEN
    ui->pasteButton->setIconSize(screen->iconSize());
    ActionBar *actionBar =new ActionBar(this);
    actionBar->setTitle(windowTitle(), true);
    ui->actionBarLayout->addWidget(actionBar);
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));
    // ui->addressEdit->setInputMethodHints(Qt::ImhLatinOnly|Qt::ImhDigitsOnly);
    // qApp->inputMethod()->show(); qApp->inputMethod()->reset();
    ui->labelEdit->setFocus();
    connect(screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
    screenOrientationChanged();
#endif

    connect(ui->stealthCB, SIGNAL(toggled(bool)), ui->labelStealthInfo, SLOT(setVisible(bool)));
    connect(ui->stealthCB, SIGNAL(toggled(bool)), ui->labelHelpStealth, SLOT(setVisible(bool)));
    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
}
EditAddressDialog::~EditAddressDialog()
{
    delete ui;
}

void EditAddressDialog::screenOrientationChanged()
{

#ifdef USE_FULLSCREEN
    //setWindowState(windowState() ^ Qt::WindowMaximized);
    screen->adjustDialogFullScreen(this);
    //setFixedSize(screen->virtualSize());
    //screen->adjustPopupDialogSize(this, true); // small dialog
    screen->adjustPopupDialogButtonsSize(this, ui->buttonBox);
#endif

}

void EditAddressDialog::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->addressEdit->setText(QApplication::clipboard()->text());
}

void EditAddressDialog::setModel(AddressTableModel *model)
{
    this->model = model;
    if(!model)
        return;

    mapper->setModel(model);
    mapper->addMapping(ui->labelEdit, AddressTableModel::Label);
    mapper->addMapping(ui->addressEdit, AddressTableModel::Address);
    mapper->addMapping(ui->stealthCB, AddressTableModel::Type);
    //if(ui->addressEdit->text().length()>35){
    //    ui->stealthCB->setChecked(true);
    //}

}

void EditAddressDialog::loadRow(int row)
{
    mapper->setCurrentIndex(row);
}

bool EditAddressDialog::saveCurrentRow()
{
    if(!model)
        return false;

    switch(mode)
    {
    case NewReceivingAddress:
    case NewSendingAddress:
        {
        int typeInd  = ui->stealthCB->isChecked() ? AddressTableModel::AT_Stealth : AddressTableModel::AT_Normal;
        address = model->addRow(
                mode == NewSendingAddress ? AddressTableModel::Send : AddressTableModel::Receive,
                ui->labelEdit->text(),
                ui->addressEdit->text(),
                typeInd);
        }
        break;
    case EditReceivingAddress:
    case EditSendingAddress:
        if(mapper->submit())
        {
            address = ui->addressEdit->text();
        }
        break;
    }
    return !address.isEmpty();
}

void EditAddressDialog::accept()
{
    if(!model)
        return;

    if(!saveCurrentRow())
    {
        switch(model->getEditStatus())
        {
        case AddressTableModel::OK:
            // Failed with unknown reason. Just reject.
            break;
        case AddressTableModel::NO_CHANGES:
            // No changes were made during edit operation. Just reject.
            break;
        case AddressTableModel::INVALID_ADDRESS:
            QMessageBox::warning(this, windowTitle(),
                tr("The entered address \"%1\" is not a valid ArtBoomerang address.").arg(ui->addressEdit->text()),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case AddressTableModel::DUPLICATE_ADDRESS:
            QMessageBox::warning(this, windowTitle(),
                tr("The entered address \"%1\" is already in the address book.").arg(ui->addressEdit->text()),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case AddressTableModel::WALLET_UNLOCK_FAILURE:
            QMessageBox::critical(this, windowTitle(),
                tr("Could not unlock wallet."),
                QMessageBox::Ok, QMessageBox::Ok);
            break;
        case AddressTableModel::KEY_GENERATION_FAILURE:
            QMessageBox::critical(this, windowTitle(),
                tr("New key generation failed."),
                QMessageBox::Ok, QMessageBox::Ok);
            break;

        }
        return;
    }
    QDialog::accept();
}

QString EditAddressDialog::getAddress() const
{
    return address;
}

void EditAddressDialog::setAddress(const QString &address)
{
    this->address = address;
    ui->addressEdit->setText(address);
}
