// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "addressbookpage.h"
#include "ui_addressbookpage.h"

#include "addresstablemodel.h"
#include "optionsmodel.h"
#include "bitcoingui.h"
#include "editaddressdialog.h"
#include "csvmodelwriter.h"
#include "guiutil.h"

#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QScroller>
#include <QTouchEvent>
#include <QMouseEvent>

#ifdef USE_QRCODE
#include "qrcodedialog.h"
#endif

#include "requestpaymentdialog.h"

AddressBookPage::AddressBookPage(Mode mode, Tabs tab, QWidget *parent, BitScreen *screen) :
    QDialog(parent),
    ui(new Ui::AddressBookPage),
    model(0),
    optionsModel(0),
    mode(mode),
    tab(tab),screen(screen),
     parent(parent)
{

    ui->setupUi(this);

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    ui->newAddressButton->setIcon(QIcon());
    ui->copyToClipboard->setIcon(QIcon());
    ui->deleteButton->setIcon(QIcon());
#endif

#ifndef USE_QRCODE
    ui->showQRCode->setVisible(false);
#endif
    ui->tableView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(ui->tableView->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(this->ui->tableView->viewport(), QScroller::LeftMouseButtonGesture);
    ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    switch(mode)
    {
    case ForSending:
        connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->setFocus();
        break;
    case ForEditing:
        ui->btnChooseAddress->setVisible(false);
        break;
    }


    switch(tab)
    {
    case SendingTab:
        setWindowTitle("Address Book");
        ui->labelExplanation->setText(tr("These are your ArtBoomerang addresses for sending payments. Always check the amount and the receiving address before sending coins."));
        ui->deleteButton->setEnabled(false);
        ui->deleteButton->setVisible(true);
        ui->verifyMessage->setEnabled(true);
        ui->verifyMessage->setVisible(true);
        ui->signMessage->setVisible(false);
        ui->signMessage->setEnabled(false);
        ui->btnRequestPayment->hide();
        break;
    case ReceivingTab:
        setWindowTitle("Receiving Addresses");
        ui->labelExplanation->setText(tr("These are your ArtBoomerang addresses for receiving payments. It is recommended to use a new receiving address for each transaction."));
        ui->deleteButton->setVisible(false);
        ui->signMessage->setVisible(true);
        ui->signMessage->setEnabled(false);
        ui->verifyMessage->setEnabled(false);
        ui->verifyMessage->setVisible(false);
        ui->btnRequestPayment->show();
        break;
    }
    showQRCodeAction = new QAction(QIcon(":/icons/qrcode"),tr("&Show QR Code"), this);
    exportAction = new QAction(QIcon(":/icons/export"),tr("E&xport"), this);
#ifdef USE_FULLSCREEN
    //QFont font = ui->labelExplanation->font(); font.setPointSize(11);
    //ui->labelExplanation->setFont(font);
    screen->adjustButtonSize(ui->signMessage, "");
    screen->adjustButtonSize(ui->verifyMessage, "");
    screen->adjustButtonSize(ui->deleteButton, "");
    screen->adjustButtonSize(ui->newAddressButton, "");
    screen->adjustButtonSize(ui->copyToClipboard, "");
    screen->adjustButtonSize(ui->showQRCode, "");
    //ui->btnChooseAddress->setIconSize(screen->iconSize());
    ui->btnChooseAddress->hide();
    ui->btnRequestPayment->setIconSize(screen->iconSize());

    // Context menu actions
    selectAddressAction = new QAction(QIcon(":/android_icons/action_ok"), tr("C&hoose"), this);
    copyLabelAction = new QAction(QIcon(":/android_icons/action_copylabel"),tr("Copy &Label"), this);
    copyAddressAction = new QAction(QIcon(":/android_icons/action_copy"),tr("Copy &Address"), this);
    editAction = new QAction(QIcon(":/android_icons/action_edit"),tr("&Edit"), this);
    signMessageAction = new QAction(QIcon(":/android_icons/action_sign"),tr("S&ign Message"), this);
    verifyMessageAction = new QAction(QIcon(":/android_icons/action_verify"),tr("&Verify Message"), this);
    deleteAction = new QAction(QIcon(":/android_icons/action_delete"),tr("&Delete"), this);
    closeAction = new QAction(QIcon(":/android_icons/action_close"), tr("&Cancel"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(selectAddressAction, SIGNAL(triggered()), this, SLOT(accept()));
#else
    selectAddressAction = new QAction(QIcon(":/icons/choose"), tr("C&hoose"), this);
  #ifdef USE_MINISCREEN
    ui->signMessage->setMinimumWidth(screen->iconSize().width());
    ui->verifyMessage->setMinimumWidth(screen->iconSize().width());
    ui->deleteButton->setMinimumWidth(screen->iconSize().width());
    ui->newAddressButton->setMinimumWidth(screen->iconSize().width());
    ui->copyToClipboard->setMinimumWidth(screen->iconSize().width());
    ui->showQRCode->setMinimumWidth(screen->iconSize().width());
    ui->btnRequestPayment->setMinimumWidth(screen->iconSize().width());

  #endif
    // Context menu actions
    copyLabelAction = new QAction(QIcon(":/android_icons/action_copylabel"),tr("Copy &Label"), this);
    copyAddressAction = new QAction(QIcon(":/icons/editcopy"),tr("&Copy Address"), this);
    editAction = new QAction(QIcon(":/icons/edit"),tr("&Edit"), this);
    signMessageAction = new QAction(QIcon(":/android_icons/action_sign"),tr("S&ign Message"), this);
    verifyMessageAction = new QAction(QIcon(":/android_icons/action_verify"),tr("&Verify Message"), this);
    deleteAction = new QAction(QIcon(":/icons/remove"),tr("&Delete"), this);
#endif
    ui->horizontalLayout->addStretch();


    // set actions disabled by default - require selection change event
    //enableActions();
    deleteAction->setEnabled(false);
    verifyMessageAction->setEnabled(false);
    signMessageAction->setEnabled(false);

    copyLabelAction->setEnabled(false);
    copyAddressAction->setEnabled(false);
    editAction->setEnabled(false);
    showQRCodeAction->setEnabled(false);



    // Build context menu
    contextMenu = new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(editAction);
    if(tab == SendingTab){
        contextMenu->addAction(deleteAction);
    }
    contextMenu->addSeparator();
    contextMenu->addAction(showQRCodeAction);
    if(tab == ReceivingTab){

        contextMenu->addAction(signMessageAction);
    }
    else if(tab == SendingTab){
        contextMenu->addAction(verifyMessageAction);
    }
    contextMenu->addSeparator();
    contextMenu->addAction(exportAction);

    // Connect signals for context menu actions
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(on_copyToClipboard_clicked()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(onCopyLabelAction()));
    connect(editAction, SIGNAL(triggered()), this, SLOT(onEditAction()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(on_deleteButton_clicked()));
    connect(showQRCodeAction, SIGNAL(triggered()), this, SLOT(on_showQRCode_clicked()));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportClicked()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(on_signMessage_clicked()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(on_verifyMessage_clicked()));

    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));

    // Pass through accept action from button box
    connect(ui->btnChooseAddress, SIGNAL(clicked()), this, SLOT(accept()));
}
AddressBookPage::~AddressBookPage()
{
    delete ui;
}

#ifdef USE_FULLSCREEN
void AddressBookPage::setActionBarMenu(ActionBar *bar)
{
    bar->clearMenu();
    bar->addMenu(copyAddressAction);
    bar->addMenu(copyLabelAction);
    bar->addMenu(editAction);
    if(tab == SendingTab){
        bar->addMenu(deleteAction);
    }
    bar->addMenuSeparator();
    bar->addMenu(showQRCodeAction);
    if(tab == ReceivingTab){
        if(parent && parent->objectName()!="SignVerifyMessage"){
            bar->addMenu(signMessageAction);
        }
        bar->removeMenu(verifyMessageAction);
        bar->removeMenu(deleteAction);
    }
    else if(tab == SendingTab){
        bar->removeMenu(signMessageAction);
        if(parent && parent->objectName()!="SignVerifyMessage"){
            bar->addMenu(verifyMessageAction);
        }
    }
    if(isDialogMode==true){
        bar->addMenuSeparator();
        bar->addMenu(exportAction);
        bar->addMenuSeparator();
        bar->addMenu(closeAction);
    }
    //connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectionChanged()));

}
#endif

void AddressBookPage::screenOrientationChanged()
{
    if(this->isDialogMode == true){
#ifdef USE_FULLSCREEN
         setFixedSize(screen->virtualSize());
#endif
    }
}

bool AddressBookPage::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(object == ui->tableView)
        {
         //   QModelIndex index = ui->tableView->indexAt(QCursor::pos());
         //   enableActions(index);
          //  lastIndexPoint = QCursor::pos();
            //emit proxyIpValid(ui->proxyIp, LookupNumeric(ui->proxyIp->text().toStdString().c_str(), addr));
        }
    }

    return QDialog::eventFilter(object, event);
    //return AddressBookPage::eventFilter(object, event);
}
void AddressBookPage::enableActions(const QModelIndex &index)//bool isValid
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;
    if(index.isValid())
    {
        if(tab == SendingTab){
            deleteAction->setEnabled(true);
            verifyMessageAction->setEnabled(true);
            signMessageAction->setEnabled(false);
        }else{
            deleteAction->setEnabled(false);
            verifyMessageAction->setEnabled(false);
            signMessageAction->setEnabled(true);
        }
        copyLabelAction->setEnabled(true);
        copyAddressAction->setEnabled(true);
        editAction->setEnabled(true);
        showQRCodeAction->setEnabled(true);
        selectAddressAction->setEnabled(true);
        //contextMenu->exec(QCursor::pos());
    }
    else{
        selectAddressAction->setEnabled(true);
        deleteAction->setEnabled(false);
        verifyMessageAction->setEnabled(false);
        signMessageAction->setEnabled(false);
        copyLabelAction->setEnabled(false);
        copyAddressAction->setEnabled(false);
        editAction->setEnabled(false);
        showQRCodeAction->setEnabled(false);
    }
}

void AddressBookPage::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    ui->signMessage->sizeHint();
    ui->verifyMessage->sizeHint();
    ui->deleteButton->sizeHint();
    ui->newAddressButton->sizeHint();
    ui->copyToClipboard->sizeHint();
    ui->showQRCode->sizeHint();
    ui->btnRequestPayment->sizeHint();
}

void AddressBookPage::fixLayoutsSize(const int &w, const int &h){
    //this->resize(w,h);
}

void AddressBookPage::setIsDialogMode(bool isDialog){
    this->isDialogMode = isDialog;
    if(parent && parent->objectName()=="SignVerifyMessage"){
        ui->signMessage->setVisible(false); ui->signMessage->setEnabled(false);
        ui->verifyMessage->setEnabled(false); ui->verifyMessage->setVisible(false);
    }
    if(isDialog == true){
        ui->signMessage->setVisible(false); ui->signMessage->setEnabled(false);
        ui->verifyMessage->setEnabled(false); ui->verifyMessage->setVisible(false);
#ifdef USE_FULLSCREEN
        //setWindowState(this->windowState() ^ Qt::WindowMaximized);
        screenOrientationChanged();
        connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));

        selectAddressAction->setEnabled(true);
        actionBar =new ActionBar(this);
        setActionBarMenu(actionBar);
        ui->actionBarLayout->addWidget(actionBar);
        actionBar->setTitle(windowTitle(), true);
        connect(actionBar, SIGNAL(up()), this, SLOT(close()));
        actionBar->addButton(selectAddressAction);
#else
        if(parent){
            if(parent->windowType()==Qt::Dialog){
              setGeometry(parent->frameGeometry());
            }
            else{
              resize(parent->width(),parent->height());
            }
        }
#endif
    }

}


void AddressBookPage::setOptionsModel(OptionsModel *optionsModel)
{
    this->optionsModel = optionsModel;
}

void AddressBookPage::setModel(AddressTableModel *model)
{
    this->model = model;
    if(!model)
        return;

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    switch(tab)
    {
    case ReceivingTab:
        // Receive filter
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Receive);
        break;
    case SendingTab:
        // Send filter
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Send);
        break;
    }
    ui->tableView->setModel(proxyModel);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);

    // Set column widths
#if QT_VERSION < 0x050000
    ui->tableView->horizontalHeader()->setResizeMode(AddressTableModel::Label, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setResizeMode(AddressTableModel::Address, QHeaderView::Stretch);
    //ui->tableView->horizontalHeader()->setResizeMode(AddressTableModel::Address, QHeaderView::ResizeToContents);
#else
    ui->tableView->horizontalHeader()->setSectionResizeMode(AddressTableModel::Label, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(AddressTableModel::Address, QHeaderView::Stretch);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(AddressTableModel::Address, QHeaderView::ResizeToContents);
#endif

    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged()));
    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(enableActions(QModelIndex)));

    // Select row for newly created address
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(selectNewAddress(QModelIndex,int,int)));

    selectionChanged();
}

void AddressBookPage::on_copyToClipboard_clicked()
{
    //selectionChanged();
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;

    GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Address);
    QString addr = QApplication::clipboard()->text();
    if(addr.isEmpty()){

        QMessageBox::warning(this, tr("Copy Address"),
            tr("Please select address to copy."),
            QMessageBox::Ok);
    }else{
        QMessageBox::information(this, tr("Copy Address"),
            tr("Address: \"%1\"\ncopied to clipboard.")
               .arg(addr.length()>35?addr.left(35)+"...":addr),
            QMessageBox::Ok);
    }
}


void AddressBookPage::copyLabelActionRelease()
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;

    // Figure out which address was selected, and return it
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString v;
    foreach (QModelIndex index, indexes)
    {
        QVariant address = table->model()->data(index);
        v = address.toString();
    }

}

void AddressBookPage::onCopyLabelAction()
{
    GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Label);
    QString addr = QApplication::clipboard()->text();
    if(addr.isEmpty()){
        QMessageBox::warning(this, tr("Copy Address Label"),
            tr("Please select label to copy."),
            QMessageBox::Ok);
    }else{
        QMessageBox::information(this, tr("Copy Address Label"),
            tr("Address Label: \"%1\"\ncopied to clipboard.")
               .arg(addr.length()>35?addr.left(35)+"...":addr),
            QMessageBox::Ok);
    }
}

void AddressBookPage::onEditAction()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
    if(indexes.isEmpty())
        return;

    EditAddressDialog
        dlg(tab == SendingTab ? EditAddressDialog::EditSendingAddress : EditAddressDialog::EditReceivingAddress,
                this, this->screen);
    dlg.setModel(model);

    QModelIndex origIndex = proxyModel->mapToSource(indexes.at(0));
    dlg.loadRow(origIndex.row());
    dlg.exec();

}

void AddressBookPage::on_signMessage_clicked()
{
    QTableView *table = ui->tableView;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString addr;

    foreach (QModelIndex index, indexes)
    {
        QVariant address = index.data();
        addr = address.toString();
    }

    emit signMessage(addr);
}

void AddressBookPage::on_verifyMessage_clicked()
{
    QTableView *table = ui->tableView;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString addr;

    foreach (QModelIndex index, indexes)
    {
        QVariant address = index.data();
        addr = address.toString();
    }

    emit verifyMessage(addr);
}

void AddressBookPage::on_newAddressButton_clicked()
{
    if(!model)
        return;

    EditAddressDialog dlg(tab == SendingTab ?
            EditAddressDialog::NewSendingAddress : EditAddressDialog::NewReceivingAddress,
                this, this->screen);
    dlg.setModel(model);
    if(dlg.exec())
    {
        newAddressToSelect = dlg.getAddress();
    }
}

void AddressBookPage::on_deleteButton_clicked()
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;

    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    if(!indexes.isEmpty())
    {
        QString addr, label;
        foreach (QModelIndex index, indexes)
        {
            addr = index.data().toString(), label = index.sibling(index.row(), 0).data().toString();
        }
        if(!addr.isEmpty()){
            QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Delete Address"),
                tr("Are you sure you want to delete address?\nLabel: \"%1\"\nAddress: \"%2\"")
                   .arg(label.length()>35?label.left(35)+"...":label)
                   .arg(addr.length()>35?addr.left(35)+"...":addr),
                QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                table->model()->removeRow(indexes.at(0).row());
            }
        }
    }


}

void AddressBookPage::selectionChanged()
{
    // Set button states based on selected tab and selection
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;

    if(table->selectionModel()->hasSelection())
    {
       // enableActions();
        switch(tab)
        {
        case SendingTab:
            // In sending tab, allow deletion of selection
            ui->deleteButton->setEnabled(true);
            ui->deleteButton->setVisible(true);
            deleteAction->setEnabled(true);
            if(isDialogMode == false){
                ui->signMessage->setEnabled(false);
                ui->signMessage->setVisible(false);
                ui->verifyMessage->setEnabled(true);
                ui->verifyMessage->setVisible(true);
            }
            break;
        case ReceivingTab:
            // Deleting receiving addresses, however, is not allowed
            ui->deleteButton->setEnabled(false);
            ui->deleteButton->setVisible(false);
            deleteAction->setEnabled(false);
            if(isDialogMode == false){
                ui->signMessage->setEnabled(true);
                ui->signMessage->setVisible(true);
                ui->verifyMessage->setEnabled(false);
                ui->verifyMessage->setVisible(false);
            }
            //ui->verifyMessage->hide();
            break;
        }
        ui->copyToClipboard->setEnabled(true);
        ui->showQRCode->setEnabled(true);
        ui->btnRequestPayment->setEnabled(true);
    }
    else
    {
        ui->deleteButton->setEnabled(false);
        ui->showQRCode->setEnabled(false);
        ui->btnRequestPayment->setEnabled(false);
        ui->copyToClipboard->setEnabled(false);
        ui->signMessage->setEnabled(false);
        ui->verifyMessage->setEnabled(false);
    }
}

void AddressBookPage::done(int retval)
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel() || !table->model())
        return;
    // When this is a tab/widget and not a model dialog, ignore "done"
    if(mode == ForEditing)
        return;

    // Figure out which address was selected, and return it
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);

    foreach (QModelIndex index, indexes)
    {
        QVariant address = table->model()->data(index);
        returnValue = address.toString();
    }

    if(returnValue.isEmpty())
    {
        // If no address entry selected, return rejected
        retval = Rejected;
    }

    QDialog::done(retval);
}

void AddressBookPage::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this, screen,
            tr("Export Address Book Data"), QString(), tr("Comma separated file (*.csv)")
            );

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(proxyModel);
    writer.addColumn("Label", AddressTableModel::Label, Qt::EditRole);
    writer.addColumn("Address", AddressTableModel::Address, Qt::EditRole);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void AddressBookPage::on_showQRCode_clicked()
{
#ifdef USE_QRCODE
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString address, label;

    foreach (QModelIndex index, indexes)
    {
        address = index.data().toString(), label = index.sibling(index.row(), 0).data(Qt::EditRole).toString();

    }
    if(!address.isEmpty()){
        QRCodeDialog *dlg = new QRCodeDialog(address, label, tab == ReceivingTab, this, screen);
        dlg->setWindowModality(Qt::WindowModal);
        if(optionsModel)
            dlg->setModel(optionsModel);

        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    }
#endif
}

void AddressBookPage::on_btnRequestPayment_clicked()
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString address, label;
    foreach (QModelIndex index, indexes)
    {
        address = index.data().toString(), label = index.sibling(index.row(), 0).data(Qt::EditRole).toString();

    }
    if(!address.isEmpty()){
        RequestPaymentDialog *dlg = new RequestPaymentDialog(address, label, this, screen);
        dlg->setWindowModality(Qt::WindowModal);
        if(optionsModel)
            dlg->setModel(optionsModel);

        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    }
}

void AddressBookPage::contextualMenu(const QPoint &point)
{
    QModelIndex index = ui->tableView->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void AddressBookPage::selectNewAddress(const QModelIndex &parent, int begin, int end)
{
    QModelIndex idx = proxyModel->mapFromSource(model->index(begin, AddressTableModel::Address, parent));
    if(idx.isValid() && (idx.data(Qt::EditRole).toString() == newAddressToSelect))
    {
        // Select row of newly created address, once
        ui->tableView->setFocus();
        ui->tableView->selectRow(idx.row());
        newAddressToSelect.clear();
    }
}
