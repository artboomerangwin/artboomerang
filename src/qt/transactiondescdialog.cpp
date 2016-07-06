#include "transactiondescdialog.h"
#include "ui_transactiondescdialog.h"

#include "transactiontablemodel.h"

#include <QModelIndex>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScroller>
#include <QTouchEvent>
#include <QClipboard>

TransactionDescDialog::TransactionDescDialog(const QModelIndex &idx, QWidget *parent, BitScreen *screen) :
    QDialog(parent), screen(screen),
    ui(new Ui::TransactionDescDialog)
{
    ui->setupUi(this);
    address = idx.data(TransactionTableModel::AddressRole).toString();
    label = idx.data(TransactionTableModel::LabelRole).toString();
    amount = idx.data(TransactionTableModel::FormattedAmountRole).toString();
    txid = idx.data(TransactionTableModel::TxIDRole).toString();

    QString desc = idx.data(TransactionTableModel::LongDescriptionRole).toString();
    ui->detailText->setHtml(desc);


    setWindowTitle(tr("Transaction details"));
    ui->detailText->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(ui->detailText->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(ui->detailText->viewport(), QScroller::LeftMouseButtonGesture);

    ui->detailText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->detailText->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QPushButton *btnCopy = new QPushButton(tr("C&opy Details"));
    ui->buttonBox->addButton(btnCopy, QDialogButtonBox::NoRole);
    connect(btnCopy, SIGNAL(clicked()), this, SLOT(copyDetails()));



#ifdef USE_FULLSCREEN
    screenOrientationChanged();
    connect(screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
    ActionBar *actionBar =new ActionBar(this);
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));
    QAction *closeAction = new QAction(QIcon(":/android_icons/action_close"), tr("&Close"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
    ui->actionBarLayout->addWidget(actionBar);
    actionBar->setTitle(windowTitle(), true);
    QAction *copyAddressAction = new QAction(QIcon(":/android_icons/action_copy"),tr("Copy &Address"), this);
    QAction *copyTxIDAction = new QAction(QIcon(":/android_icons/action_copy_id"),tr("Copy &Transaction ID"), this);
    actionBar->addButton(copyAddressAction);
    if(!label.isEmpty()){
        QAction *copyLabelAction = new QAction(QIcon(":/android_icons/action_copy_label"),tr("Copy &Label"), this);
        actionBar->addButton(copyLabelAction);
        connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(copyLabel()));
    }
    if(!amount.isEmpty()){
        QAction *copyAmountAction = new QAction(QIcon(":/android_icons/action_copy_amount"),tr("Copy A&mount"), this);
        actionBar->addButton(copyAmountAction);
        connect(copyAmountAction, SIGNAL(triggered()), this, SLOT(copyAmount()));
    }
    actionBar->addButton(copyTxIDAction);
    actionBar->addButton(closeAction);
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyTxIDAction, SIGNAL(triggered()), this, SLOT(copyTxID()));

#else

    QPushButton *btnCopyAddress = new QPushButton(tr("Copy &Address"));
    ui->buttonBox->addButton(btnCopyAddress, QDialogButtonBox::NoRole);
    if(!label.isEmpty()){
        QPushButton *btnCopyLabel = new QPushButton(tr("Copy &Label"));
        ui->buttonBox->addButton(btnCopyLabel, QDialogButtonBox::NoRole);
        connect(btnCopyLabel, SIGNAL(clicked()), this, SLOT(copyLabel()));
    }
    if(!amount.isEmpty() || amount != "0.00"){
        QPushButton *btnCopyAmount = new QPushButton(tr("Copy A&mount"));
        ui->buttonBox->addButton(btnCopyAmount, QDialogButtonBox::NoRole);
        connect(btnCopyAmount, SIGNAL(clicked()), this, SLOT(copyAmount()));
    }
    QPushButton *btnCopyTxId = new QPushButton(tr("Copy &Transaction ID"));
    ui->buttonBox->addButton(btnCopyTxId, QDialogButtonBox::NoRole);
    connect(btnCopyAddress, SIGNAL(clicked()), this, SLOT(copyAddress()));
    connect(btnCopyTxId, SIGNAL(clicked()), this, SLOT(copyTxID()));
    screen->adjustPopupDialogButtonsSize(this, ui->buttonBox);

#endif

}

TransactionDescDialog::~TransactionDescDialog()
{
    delete ui;
}

void TransactionDescDialog::screenOrientationChanged()
{

#ifdef USE_FULLSCREEN
    screen->adjustDialogFullScreen(this);
    screen->adjustPopupDialogButtonsSize(this, ui->buttonBox);
#endif

}

void TransactionDescDialog::copyDetails(){
    QString selection = ui->detailText->toPlainText();

    if(!selection.isEmpty())
    {
        QApplication::clipboard()->setText(selection);
    }
}
void TransactionDescDialog::copyTxData(const QString &v, const QString &title)
{
    QApplication::clipboard()->clear();
    if(v.isEmpty())
        return;
    QApplication::clipboard()->setText(v);
    QMessageBox::information(this, QString("Copy %1").arg(title),
            tr("%1: \"%2\"\ncopied to clipboard.").arg(title)
               .arg(v.length()>35?v.left(35)+"...":v),
            QMessageBox::Ok);

}
void TransactionDescDialog::copyAddress()
{
    copyTxData(address, tr("Address"));
}

void TransactionDescDialog::copyLabel()
{
    copyTxData(label,tr("Address Label"));
}

void TransactionDescDialog::copyAmount()
{
    copyTxData(amount,tr("Amount"));
}

void TransactionDescDialog::copyTxID()
{
    copyTxData(txid,tr("Transaction ID"));
}

/*
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
void TransactionView::mousePressEvent(QMouseEvent *e)
{
    QDateEdit::mousePressEvent(e);
}
*/
void TransactionDescDialog::keyPressEvent(QKeyEvent *event)
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

void TransactionDescDialog::closeEvent(QCloseEvent *e)
{
    emit(stopExec());
    QWidget::closeEvent(e);
}
