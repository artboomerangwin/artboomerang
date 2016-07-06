// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QScreen>

#include <QDebug>

#define NUM_ITEMS 3

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {
        setDecorationSize(64);
    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(this->decorationSize, this->decorationSize));
        int xspace = this->decorationSize + 2;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address =  index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
#if QT_VERSION < 0x050000
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }
#else
        if(value.canConvert<QBrush>())
        {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }
#endif

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(this->decorationSize, this->decorationSize);
    }

    int unit;
    int decorationSize;
    void setDecorationSize(int ds){ this->decorationSize = ds; }
    int getDecorationSize(){ return decorationSize; }


};

#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent, BitScreen *screen) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    screen(screen),
    currentBalance(-1),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    int em=fontMetrics().height();
#ifdef USE_FULLSCREEN
    txdelegate->setDecorationSize(2.5*em);
#endif

    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged(Qt::ScreenOrientation)));
    ui->setupUi(this);
    ui->label_under_bg->hide();
    //this->fixLayoutsSize(0,0);
    //this->sizeHint();
    //ui->label_4->setText("");

    ui->frame_2->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    ui->frame_2->setFixedHeight((NUM_ITEMS * (txdelegate->getDecorationSize() + 6))+em);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    //ui->listTransactions->sizeHint();
    ui->listTransactions->setIconSize(QSize(txdelegate->getDecorationSize(), txdelegate->getDecorationSize()));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (txdelegate->getDecorationSize() + 2));
    //ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 6));
    //ui->listTransactions->setIconSize(QSize(48, 48));
    //ui->listTransactions->setMinimumHeight(NUM_ITEMS * (48 + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

void OverviewPage::screenOrientationChanged(Qt::ScreenOrientation o)
{
#ifdef USE_FULLSCREEN
     fixLayoutOrientation();
#endif

}
void OverviewPage::fixLayoutOrientation(){
#ifdef USE_FULLSCREEN
    int em=fontMetrics().height();
    if(screen->screenType()!="desc"){
        int w = screen->size().width()-this->layout()->contentsMargins().left()-this->layout()->contentsMargins().right();
        int bottomSpace = screen->size().height()-ui->frame->height()-ui->frame_2->height()-40;
        if(screen->isPortrait()){
            if(ui->verticalLayoutVTransaction->isEmpty()){
                ui->verticalLayout_3->removeItem(ui->verticalSpacer_2);
                ui->frame->setFixedWidth(w);
                ui->frame_2->setFixedWidth(w);
                ui->verticalLayoutVTransaction->addWidget(ui->frame_2);
            }
        }else{
            ui->frame->setFixedWidth((w/2)-6);
            ui->frame_2->setFixedWidth((w/2)-6);
            if(false == ui->verticalLayoutVTransaction->isEmpty()){
                ui->verticalLayout_3->addWidget(ui->frame_2);
                ui->verticalLayout_3->addSpacerItem(ui->verticalSpacer_2);
            }
            if(bottomSpace>40){
                //ui->label_wallet_bg->resize(bottomSpace,bottomSpace);
               // ui->label_wallet_bg->setMaximumHeight(bottomSpace);
            }else{
             //   ui->label_wallet_bg->hide();
            }
        }
    }
#endif
}
void OverviewPage::fixLayoutSize(const int &w, const int &h){
#ifndef USE_FULLSCREEN
#ifdef USE_MINISCREEN
    ui->label_under_bg->hide();
    if(w<640)
    {
        ui->label_wallet_bg->hide();
         if(ui->verticalLayoutVTransaction->isEmpty()){
             ui->verticalLayoutVTransaction->addWidget(ui->frame_2);
             ui->verticalLayout_3->removeItem(ui->verticalSpacer_2);
             ui->verticalLayoutVTransaction->addSpacerItem(ui->verticalSpacer_2);
             //ui->verticalLayout_3->removeWidget(ui->frame_2);
         }
    }
    else{
        ui->label_wallet_bg->show();
         if(false == ui->verticalLayoutVTransaction->isEmpty()){
             //ui->verticalLayout_3->removeItem(ui->verticalSpacer_2);
             //ui->verticalLayoutVTransaction->removeWidget(ui->frame_2);
             ui->verticalLayoutVTransaction->removeItem(ui->verticalSpacer_2);
             ui->verticalLayout_3->addWidget(ui->frame_2);
             ui->verticalLayout_3->addSpacerItem(ui->verticalSpacer_2);
         }
    }
#endif
#endif
}


void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balance + stake + unconfirmedBalance + immatureBalance));
    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);
        //ui->listTransactions->setFixedWidth(200);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
}
