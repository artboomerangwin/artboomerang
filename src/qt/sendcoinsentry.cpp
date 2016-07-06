// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendcoinsentry.h"
#include "ui_sendcoinsentry.h"
#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "stealth.h"

#include <QApplication>
#include <QClipboard>

SendCoinsEntry::SendCoinsEntry(QWidget *parent, BitScreen *screen) :
    QFrame(parent),
    ui(new Ui::SendCoinsEntry),
    model(0),screen(screen), parent(parent)
{
    ui->setupUi(this);
    ui->addressBookButton->setIconSize(screen->iconSize());
    ui->pasteButton->setIconSize(screen->iconSize());
    ui->deleteButton->setIconSize(screen->iconSize());


#ifdef Q_OS_MAC
    ui->payToLayout->setSpacing(4);
#endif
#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->addAsLabel->setPlaceholderText(tr("Enter a label for this address to add it to your address book"));
    ui->payTo->setPlaceholderText(tr("Enter a ArtBoomerang address (e.g. FfCDhekqN2ngKCfKLyxQodtAUfCQVPqFFt)"));
    ui->narration->setPlaceholderText(tr("Enter a short note to send with payment (max 24 characters)"));
#endif
#ifdef USE_FULLSCREEN
    ui->addAsLabel->setPlaceholderText(tr("Label for this address to add it to your address book"));
    ui->payTo->setPlaceholderText(tr("ABM address (e.g. FfCDhekqN2ngKCfKLyxQodtAUfCQVPqFFt)"));
    ui->narration->setPlaceholderText(tr("Short note to send with payment (max 24 characters)"));
#endif
    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(ui->payTo);

    GUIUtil::setupAddressWidget(ui->payTo, this);
    ui->narration->setMaxLength(24);
}

SendCoinsEntry::~SendCoinsEntry()
{
    delete ui;
}

void SendCoinsEntry::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SendCoinsEntry::on_addressBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage *dlg = new AddressBookPage(AddressBookPage::ForSending, AddressBookPage::SendingTab, parent, this->screen);
    dlg->setIsDialogMode(true);
    dlg->setModel(model->getAddressTableModel());
    if(dlg->exec())
    {
        ui->payTo->setText(dlg->getReturnValue());
        ui->payAmount->setFocus();
    }
}

void SendCoinsEntry::on_payTo_textChanged(const QString &address)
{
    if(!model)
        return;
    
    
    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if(!associatedLabel.isEmpty())
        ui->addAsLabel->setText(associatedLabel);

}

void SendCoinsEntry::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    connect(ui->payAmount, SIGNAL(textChanged()), this, SIGNAL(payAmountChanged()));

    clear();
}


void SendCoinsEntry::clear()
{
    ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->narration->clear();
    ui->payTo->setFocus();
    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void SendCoinsEntry::on_deleteButton_clicked()
{
    emit removeEntry(this);
}

bool SendCoinsEntry::validate()
{
    // Check input validity
    bool retval = true;

    if(!ui->payAmount->validate())
    {
        retval = false;
    }
    else
    {
        if(ui->payAmount->value() <= 0)
        {
            // Cannot send 0 coins or less
            ui->payAmount->setValid(false);
            retval = false;
        }
    }

    if(!ui->payTo->hasAcceptableInput() ||
       (model && !model->validateAddress(ui->payTo->text())))
    {
        ui->payTo->setValid(false);
        retval = false;
    }

    return retval;
}

SendCoinsRecipient SendCoinsEntry::getValue()
{
    SendCoinsRecipient rv;

    rv.address = ui->payTo->text();
    rv.label = ui->addAsLabel->text();
    rv.narration = ui->narration->text();
    
    if (rv.address.length() > 75 
        && IsStealthAddress(rv.address.toStdString()))
        rv.typeInd = AddressTableModel::AT_Stealth;
    else
        rv.typeInd = AddressTableModel::AT_Normal;
    
    rv.amount = ui->payAmount->value();
    
    return rv;
}

QWidget *SendCoinsEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->payTo);
    QWidget::setTabOrder(ui->payTo, ui->addressBookButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    QWidget::setTabOrder(ui->deleteButton, ui->addAsLabel);
    QWidget::setTabOrder(ui->addAsLabel, ui->narration);
    return ui->payAmount->setupTabChain(ui->narration);
}

void SendCoinsEntry::setValue(const SendCoinsRecipient &value)
{
    ui->payTo->setText(value.address);
    ui->narration->setText(value.narration);
    ui->addAsLabel->setText(value.label);
    ui->payAmount->setValue(value.amount);
}

bool SendCoinsEntry::isClear()
{
    return ui->payTo->text().isEmpty();
}

void SendCoinsEntry::setFocus()
{
    ui->payTo->setFocus();
}

void SendCoinsEntry::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        // Update payAmount with the current unit
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}
