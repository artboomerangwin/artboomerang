#include "transactionview.h"

#include "transactionfilterproxy.h"
#include "transactionrecord.h"
#include "walletmodel.h"
#include "addresstablemodel.h"
#include "transactiontablemodel.h"
#include "bitcoinunits.h"
#include "csvmodelwriter.h"
#include "transactiondescdialog.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "guiutil.h"

#include <QComboBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QPoint>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QDateTimeEdit>
#include <QScrollBar>
#include <QScroller>
#include <QTouchEvent>
#include <QDebug>

TransactionView::TransactionView(QWidget *parent, BitScreen *screen) :
    QWidget(parent), model(0), transactionProxyModel(0),
    transactionView(0), screen(screen),parent(parent)
{

    // Build filter row
    setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0,0,0,0);
#ifdef Q_OS_MAC
    hlayout->setSpacing(5);
    hlayout->addSpacing(26);
#else
    hlayout->setSpacing(0);
    hlayout->addSpacing(23);
#endif

    dateWidget = new QComboBox(this);
#ifndef USE_FULLSCREEN
    #ifdef Q_OS_MAC
        dateWidget->setFixedWidth(76);
    #else
        dateWidget->setFixedWidth(75);
    #endif
    #ifdef USE_MINISCREEN
        dateWidget->setFixedWidth(120);
    #endif
#endif

    dateWidget->addItem(tr("Date: All"), All);
    dateWidget->addItem(tr("Today"), Today);
    dateWidget->addItem(tr("This week"), ThisWeek);
    dateWidget->addItem(tr("This month"), ThisMonth);
    dateWidget->addItem(tr("Last month"), LastMonth);
    dateWidget->addItem(tr("This year"), ThisYear);
    dateWidget->addItem(tr("Range..."), Range);
    hlayout->addWidget(dateWidget);

    typeWidget = new QComboBox(this);
#ifndef USE_FULLSCREEN
    #ifdef Q_OS_MAC
        typeWidget->setFixedWidth(76);
    #else
        typeWidget->setFixedWidth(75);
    #endif
    #ifdef USE_MINISCREEN
        typeWidget->setFixedWidth(120);
    #endif
#endif

    typeWidget->addItem(tr("Type: All"), TransactionFilterProxy::ALL_TYPES);
    typeWidget->addItem(tr("Received with"), TransactionFilterProxy::TYPE(TransactionRecord::RecvWithAddress) |
                                        TransactionFilterProxy::TYPE(TransactionRecord::RecvFromOther));
    typeWidget->addItem(tr("Sent to"), TransactionFilterProxy::TYPE(TransactionRecord::SendToAddress) |
                                  TransactionFilterProxy::TYPE(TransactionRecord::SendToOther));
    typeWidget->addItem(tr("To yourself"), TransactionFilterProxy::TYPE(TransactionRecord::SendToSelf));
    typeWidget->addItem(tr("Mined"), TransactionFilterProxy::TYPE(TransactionRecord::Generated));
    typeWidget->addItem(tr("Other"), TransactionFilterProxy::TYPE(TransactionRecord::Other));

    hlayout->addWidget(typeWidget);


    addressWidget = new QLineEdit(this);
#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    addressWidget->setPlaceholderText(tr("Enter address or label to search"));
#endif

    amountWidget = new QLineEdit(this);
#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    amountWidget->setPlaceholderText(tr("Min amount"));
#endif

#ifndef USE_FULLSCREEN
    #ifdef Q_OS_MAC
        amountWidget->setFixedWidth(97);
    #else
        amountWidget->setFixedWidth(100);
    #endif
#endif
    amountWidget->setValidator(new QDoubleValidator(0, 1e20, 8, this));


    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setSpacing(0);
    //QLabel *l = new QLabel(); l->setTextFormat(Qt::RichText);
    //l->setText("<h3>"+tr("Transaction History")+"</h3>"); vlayout->addWidget(l);

    transactionView = new QTableView(this);
#ifdef USE_FULLSCREEN
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    hlayout2->setContentsMargins(0,3,0,3);
    hlayout2->addWidget(addressWidget);
    hlayout2->addWidget(amountWidget);
    vlayout->addLayout(hlayout);
    vlayout->addLayout(hlayout2);
#else
  #ifdef USE_MINISCREEN
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    hlayout2->setContentsMargins(0,3,0,3);
    hlayout2->addWidget(addressWidget);
    hlayout2->addWidget(amountWidget);
    vlayout->addLayout(hlayout);
    vlayout->addLayout(hlayout2);
  #else
    hlayout->addWidget(addressWidget);
    hlayout->addWidget(amountWidget);
    vlayout->addLayout(hlayout);
  #endif
#endif

    vlayout->addWidget(createDateRangeWidget());
    vlayout->addWidget(transactionView);
    vlayout->setSpacing(0);
    int width = transactionView->verticalScrollBar()->sizeHint().width();
    // Cover scroll bar width with spacing
#ifdef Q_OS_MAC
    hlayout->addSpacing(width+2);
#else
    hlayout->addSpacing(width);
#endif
    // Always show scroll bar
    transactionView->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(transactionView->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(transactionView->viewport(), QScroller::LeftMouseButtonGesture);

    transactionView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    transactionView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    transactionView->setTabKeyNavigation(false);
    transactionView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Actions
    QAction *copyAddressAction = new QAction(tr("Copy address"), this);
    QAction *copyLabelAction = new QAction(tr("Copy label"), this);
    QAction *copyAmountAction = new QAction(tr("Copy amount"), this);
    QAction *copyTxIDAction = new QAction(tr("Copy transaction ID"), this);
    QAction *editLabelAction = new QAction(tr("Edit label"), this);
    QAction *showDetailsAction = new QAction(tr("Show transaction details"), this);

    contextMenu = new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    contextMenu->addAction(copyAmountAction);
    contextMenu->addAction(copyTxIDAction);
    contextMenu->addAction(editLabelAction);
    contextMenu->addAction(showDetailsAction);

    // Connect actions
    connect(dateWidget, SIGNAL(activated(int)), this, SLOT(chooseDate(int)));
    connect(typeWidget, SIGNAL(activated(int)), this, SLOT(chooseType(int)));
    connect(addressWidget, SIGNAL(textChanged(QString)), this, SLOT(changedPrefix(QString)));
    connect(amountWidget, SIGNAL(textChanged(QString)), this, SLOT(changedAmount(QString)));

    connect(transactionView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));

    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(copyLabel()));
    connect(copyAmountAction, SIGNAL(triggered()), this, SLOT(copyAmount()));
    connect(copyTxIDAction, SIGNAL(triggered()), this, SLOT(copyTxID()));
    connect(editLabelAction, SIGNAL(triggered()), this, SLOT(editLabel()));
    connect(showDetailsAction, SIGNAL(triggered()), this, SLOT(showDetails()));
#ifdef USE_FULLSCREEN
    connect(transactionView, SIGNAL(clicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));

    screenOrientationChanged();
    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
#else
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), this, SIGNAL(doubleClicked(QModelIndex)));

#endif
}
void TransactionView::screenOrientationChanged(){
#ifdef USE_FULLSCREEN
    int em=fontMetrics().height();
    typeWidget->setMinimumWidth(8*em);
    dateWidget->setMinimumWidth(8*em);
    amountWidget->setMinimumWidth(6*em);
    typeWidget->setMaximumWidth(10*em);
    dateWidget->setMaximumWidth(10*em);
    amountWidget->setMaximumWidth(8*em);
    dateFrom->setMinimumHeight(2.5*em);
    dateTo->setMinimumHeight(2.5*em);
    dateFrom->setMinimumWidth(7*em);
    dateTo->setMinimumWidth(7*em);

#endif

}
bool TransactionView::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(object == dateTo)
        {
        }
    }

    return TransactionView::eventFilter(object, event);
}

void TransactionView::setModel(WalletModel *model)
{
    int em=fontMetrics().height();
    this->model = model;
    if(model)
    {
        transactionProxyModel = new TransactionFilterProxy(this);
        transactionProxyModel->setSourceModel(model->getTransactionTableModel());
        transactionProxyModel->setDynamicSortFilter(true);
        transactionProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        transactionProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        transactionProxyModel->setSortRole(Qt::EditRole);

        transactionView->setModel(transactionProxyModel);
        transactionView->setAlternatingRowColors(true);
        transactionView->setSelectionBehavior(QAbstractItemView::SelectRows);
        transactionView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        transactionView->setSortingEnabled(true);
        transactionView->sortByColumn(TransactionTableModel::Date, Qt::DescendingOrder);
        transactionView->verticalHeader()->hide();


        transactionView->horizontalHeader()->resizeSection(TransactionTableModel::Date, 7*em);
        transactionView->horizontalHeader()->resizeSection(TransactionTableModel::Type, 7*em);
#ifdef USE_FULLSCREEN
        transactionView->horizontalHeader()->resizeSection(TransactionTableModel::Status, em);
#else
        transactionView->horizontalHeader()->resizeSection(TransactionTableModel::Status, 1.7*em);
#endif
#if QT_VERSION < 0x050000
        transactionView->horizontalHeader()->setResizeMode(TransactionTableModel::ToAddress, QHeaderView::Stretch);
        transactionView->horizontalHeader()->setResizeMode(TransactionTableModel::Description, QHeaderView::Stretch);
#else
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::ToAddress, QHeaderView::Stretch);
        transactionView->horizontalHeader()->setSectionResizeMode(TransactionTableModel::Description, QHeaderView::Stretch);
#endif
        transactionView->horizontalHeader()->resizeSection(TransactionTableModel::Amount, 7*em);

        //default view filter
        dateWidget->setCurrentIndex(2);
        typeWidget->setCurrentIndex(1);
        chooseDate(2); chooseType(1);




    }
}

/**
 * resize page for small screen
 */
void TransactionView::fixLayoutsSize(){
    if(parent->width()<640){
        transactionView->hideColumn(4);
#ifdef USE_FULLSCREEN
        transactionView->hideColumn(2);
#else
  #ifdef USE_MINISCREEN
        transactionView->hideColumn(2);
  #endif
#endif
    }else{
        transactionView->showColumn(2);
        transactionView->showColumn(4);
    }
}

void TransactionView::chooseDate(int idx)
{
    if(!transactionProxyModel)
        return;
    QDate current = QDate::currentDate();
    dateRangeWidget->setVisible(false);
    switch(dateWidget->itemData(idx).toInt())
    {
    case All:
        transactionProxyModel->setDateRange(
                TransactionFilterProxy::MIN_DATE,
                TransactionFilterProxy::MAX_DATE);
        break;
    case Today:
        transactionProxyModel->setDateRange(
                QDateTime(current),
                TransactionFilterProxy::MAX_DATE);
        break;
    case ThisWeek: {
        // Find last Monday
        QDate startOfWeek = current.addDays(-(current.dayOfWeek()-1));
        transactionProxyModel->setDateRange(
                QDateTime(startOfWeek),
                TransactionFilterProxy::MAX_DATE);

        } break;
    case ThisMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month(), 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case LastMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month()-1, 1)),
                QDateTime(QDate(current.year(), current.month(), 1)));
        break;
    case ThisYear:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), 1, 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case Range:
        dateRangeWidget->setVisible(true);
        dateRangeChanged();
        break;
    }
}

void TransactionView::chooseType(int idx)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setTypeFilter(
        typeWidget->itemData(idx).toInt());
}

void TransactionView::changedPrefix(const QString &prefix)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setAddressPrefix(prefix);
}

void TransactionView::changedAmount(const QString &amount)
{
    if(!transactionProxyModel)
        return;
    qint64 amount_parsed = 0;
    if(BitcoinUnits::parse(model->getOptionsModel()->getDisplayUnit(), amount, &amount_parsed))
    {
        transactionProxyModel->setMinAmount(amount_parsed);
    }
    else
    {
        transactionProxyModel->setMinAmount(0);
    }
}

void TransactionView::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this,screen,
            tr("Export Transaction Data"), QString(),
            tr("Comma separated file (*.csv)")
                );

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(transactionProxyModel);
    writer.addColumn(tr("Confirmed"), 0, TransactionTableModel::ConfirmedRole);
    writer.addColumn(tr("Date"), 0, TransactionTableModel::DateRole);
    writer.addColumn(tr("Type"), TransactionTableModel::Type, Qt::EditRole);
    writer.addColumn(tr("Label"), 0, TransactionTableModel::LabelRole);
    writer.addColumn(tr("Address"), 0, TransactionTableModel::AddressRole);
    writer.addColumn(tr("Amount"), 0, TransactionTableModel::FormattedAmountRole);
    writer.addColumn(tr("ID"), 0, TransactionTableModel::TxIDRole);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void TransactionView::contextualMenu(const QPoint &point)
{
    QModelIndex index = transactionView->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void TransactionView::copyAddress()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::AddressRole);
}

void TransactionView::copyLabel()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::LabelRole);
}

void TransactionView::copyAmount()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::FormattedAmountRole);
}

void TransactionView::copyTxID()
{
    GUIUtil::copyEntryData(transactionView, 0, TransactionTableModel::TxIDRole);
}

void TransactionView::editLabel()
{
    if(!transactionView->selectionModel() ||!model)
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        AddressTableModel *addressBook = model->getAddressTableModel();
        if(!addressBook)
            return;
        QString address = selection.at(0).data(TransactionTableModel::AddressRole).toString();
        if(address.isEmpty())
        {
            // If this transaction has no associated address, exit
            return;
        }
        // Is address in address book? Address book can miss address when a transaction is
        // sent from outside the UI.
        int idx = addressBook->lookupAddress(address);
        if(idx != -1)
        {
            // Edit sending / receiving address
            QModelIndex modelIdx = addressBook->index(idx, 0, QModelIndex());
            // Determine type of address, launch appropriate editor dialog type
            QString type = modelIdx.data(AddressTableModel::TypeRole).toString();

            EditAddressDialog dlg(type==AddressTableModel::Receive
                                         ? EditAddressDialog::EditReceivingAddress
                                         : EditAddressDialog::EditSendingAddress,
                                  this, screen);
            dlg.setModel(addressBook);
            dlg.loadRow(idx);
            dlg.exec();
        }
        else
        {
            // Add sending address
            EditAddressDialog dlg(EditAddressDialog::NewSendingAddress,
                                  this, screen);
            dlg.setModel(addressBook);
            dlg.setAddress(address);
            dlg.exec();
        }
    }
}

void TransactionView::showDetails()
{
    if(!transactionView->selectionModel())
        return;
    QModelIndexList selection = transactionView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        TransactionDescDialog dlg(selection.at(0),this,screen);
        dlg.exec();
    }
}

QWidget *TransactionView::createDateRangeWidget()
{
    dateRangeWidget = new QFrame();
    dateRangeWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dateRangeWidget->setContentsMargins(1,1,1,1);
    QHBoxLayout *layout = new QHBoxLayout(dateRangeWidget);
    layout->setContentsMargins(0,0,0,0);
    layout->addSpacing(23);
    layout->addWidget(new QLabel(tr("Range:")));

    dateFrom = new QDateTimeEdit(this);
    dateFrom->setDisplayFormat("dd/MM/yy");
    dateFrom->setCalendarPopup(true);
    dateFrom->setDate(QDate::currentDate().addDays(-7));
    layout->addWidget(dateFrom);
    layout->addWidget(new QLabel(tr("to")));

    dateTo = new QDateTimeEdit(this);
    dateTo->setDisplayFormat("dd/MM/yy");
    dateTo->setCalendarPopup(true);
    dateTo->setDate(QDate::currentDate());
    layout->addWidget(dateTo);
    layout->addStretch();
    int em=fontMetrics().height();
    dateFrom->setMinimumHeight(2*em);
    dateTo->setMinimumHeight(2*em);
    dateFrom->setMinimumWidth(7*em);
    dateTo->setMinimumWidth(7*em);

    // Hide by default
    dateRangeWidget->hide();

    // Notify on change
    connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));
    connect(dateTo, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));

    return dateRangeWidget;
}

void TransactionView::dateRangeChanged()
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setDateRange(
            QDateTime(dateFrom->date()),
            QDateTime(dateTo->date()).addDays(1));
}

void TransactionView::focusTransaction(const QModelIndex &idx)
{
    if(!transactionProxyModel)
        return;
    QModelIndex targetIdx = transactionProxyModel->mapFromSource(idx);
    transactionView->scrollTo(targetIdx);
    transactionView->setCurrentIndex(targetIdx);
    transactionView->setFocus();
}
