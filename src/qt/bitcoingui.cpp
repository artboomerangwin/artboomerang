/*
 * Qt5 bitcoin GUI.
 *
 * W.J. van der Laan 2011-2012
 * The Bitcoin Developers 2011-2012
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

 */

#include "bitcoingui.h"
#include "transactiontablemodel.h"
#include "addressbookpage.h"

#include "sendcoinsdialog.h"
#include "donatedialog.h"
#include "signverifymessagedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "transactiondescdialog.h"
#include "addresstablemodel.h"
#include "transactionview.h"
#include "overviewpage.h"
#include "bitcoinunits.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "notificator.h"
#include "guiutil.h"
#include "rpcconsole.h"
#include "wallet.h"

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif


#include <QtCore/QDebug>

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QStackedWidget>
#include <QDateTime>
#include <QMovie>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>
#include <QDragEnterEvent>
#if QT_VERSION < 0x050000
#include <QUrl>
#else
#include <QUrlQuery>
#endif
#include <QStyle>
#include <QStyleFactory>
#include <QTextStream>
#include <QTextDocument>
#include <QSettings>
#include <iostream>
#include <QProgressDialog>

extern CWallet* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
double GetPoSKernelPS();

/*
 * define VERTICAL_TOOBAR_STYLESHEET "QToolBar {\
border:none;\
height:100%;\
padding-top:20px;\
text-align: left;\
}\
QToolButton {\
min-width:180px;\
background-color: transparent;\
border: 1px solid #3A3939;\
border-radius: 3px;\
margin: 3px;\
padding-left: 5px;\
padding-top:5px;\
width:100%;\
text-align: left;\
padding-bottom:5px;\
}\
QToolButton:pressed {\
background-color: #FFFFFF;\
border: 1px solid silver;\
}\
QToolButton:checked {\
background-color: #FFFFF;\
border: 1px solid silver;\
}\
QToolButton:hover {\
background-color: #FFFFFF;\
border: 1px solid gray;\
}"
*/
/*define HORIZONTAL_TOOLBAR_STYLESHEET "QToolBar {\
    border: 1px solid #393838;\
    background: 1px solid #302F2F;\
    font-weight: bold;\
}"
*/

#define VERTICAL_TOOBAR_STYLESHEET "QToolBar {\
}"
#define HORIZONTAL_TOOLBAR_STYLESHEET "QToolBar {\
}"

ActiveLabel::ActiveLabel(const QString & text, QWidget * parent):
    QLabel(parent){}

void ActiveLabel::mouseReleaseEvent(QMouseEvent * event)
{
    emit clicked();
}

BitcoinGUI::BitcoinGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    changePassphraseAction(0),
    unlockWalletAction(0),
    lockWalletAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0),
    nWeight(0),
    spinnerFrame(0)
{

    setWindowTitle(tr("ArtBoomerang") + " - " + tr("Wallet"));
#ifndef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":icons/bitcoin"));
    setWindowIcon(QIcon(":icons/bitcoin"));
#else
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
    setUnifiedTitleAndToolBarOnMac(true);

    screen = new BitScreen(this, this);
    //screen->maybeMoveOrResize(this);

    overviewPage = new OverviewPage(this, screen);
    transactionView = new TransactionView(this, screen);
    transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(transactionView);
    transactionsPage->setLayout(vbox);

    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab, this, screen);
    addressBookPage->setIsDialogMode(false);

    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab, this, screen);
    receiveCoinsPage->setIsDialogMode(false);

    sendCoinsPage = new SendCoinsDialog(this, screen);

#ifdef USE_FULLSCREEN
        overviewPage->fixLayoutOrientation();
#else
        sendCoinsPage->fixLayoutsSize();//width()-12,height()-12
#endif
    GUIUtil::restoreWindowGeometry("nWindow", QSize(640, 400), this, screen);


#ifndef USE_FULLSCREEN
    QApplication::setStyle(QStyleFactory::create("Fusion"));
#else
    QFile f(":androidLight/style.qss");
    //if (f.exists())
    //{
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        QString style = ts.readAll();
        //qApp->setStyleSheet();
    //}
    setStyleSheet(style.arg(screen->iconSize().width()*3).arg(screen->iconSize().height()));


#endif


    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();
    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

    // Create tabs

    signVerifyMessageDialog = new SignVerifyMessageDialog(this, screen);

    centralWidget = new QStackedWidget(this);
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(sendCoinsPage);
    centralWidget->addWidget(receiveCoinsPage);
//#ifndef USE_FULLSCREEN
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
//#endif
    setCentralWidget(centralWidget);


    // Create status bar
    statusBar();

    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);

    labelEncryptionIcon = new ActiveLabel();

    labelStakingIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelEncryptionIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelStakingIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingIcon()));
        timerStakingIcon->start(30 * 1000);
        updateStakingIcon();
    }

    connect(labelEncryptionIcon, SIGNAL(clicked()), unlockWalletAction, SLOT(trigger()));

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);

    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
#ifdef USE_FULLSCREEN
    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));

    progressBar->setStyleSheet("QProgressBar { background-color: #e8e8e8; border: 1px solid grey; border-radius: 5px; padding: 0px; text-align: center; font-size: 8px; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #05B8CC, stop: 1 #05B8CC); }");
    //text-rendering: optimizeLegibility;  Unknown property text-rendering
#else
    QString curStyle = qApp->style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        progressBar->setStyleSheet("QProgressBar { background-color: #e8e8e8; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #05B8CC, stop: 1 #05B8CC); border-radius: 7px; margin: 0px; }");
    }
#endif

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);

    spinnerTimer = new QTimer(this);
    spinnerTimer->setInterval(MODEL_UPDATE_DELAY/3);
    connect(spinnerTimer, SIGNAL(timeout()), this, SLOT(updateSpinner()));

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    rpcConsole = new RPCConsole(this, screen);
    connect( openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()) );

    // Clicking on "Verify Message" in the address book sends you to the verify message tab
    connect(addressBookPage, SIGNAL(verifyMessage(QString)), this, SLOT(gotoVerifyMessageTab(QString)));
    // Clicking on "Sign Message" in the receive coins page sends you to the sign message tab
    connect(receiveCoinsPage, SIGNAL(signMessage(QString)), this, SLOT(gotoSignMessageTab(QString)));

    QSettings settings;
    QByteArray stateArray = settings.value("mainWindowState", "").toByteArray();
    restoreState(stateArray);

    //QCoreApplication::instance()->processEvents();
    gotoOverviewPage();

}

/**
 * @brief BitcoinGUI::fixLayoutsSize
 * dynamicallychanging size depending on window frame.
 * coould not be used for one-time frame size changes depending on screen size
 */

void BitcoinGUI::screenOrientationChanged()
{
#ifdef USE_FULLSCREEN
    screen->maybeMoveOrResize(this);
#endif

}

void BitcoinGUI::fixLayoutsSize(){
       overviewPage->fixLayoutSize(this->width(),this->height());
}

void BitcoinGUI::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
#ifndef USE_FULLSCREEN
    fixLayoutsSize();
#endif
}

BitcoinGUI::~BitcoinGUI()
{
    QSettings settings;
    QByteArray stateArray = saveState();
    settings.setValue("mainWindowState", stateArray);

    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
#endif
}

void BitcoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);
    QString txtOverview = tr("&Wallet");
    QString txtSend = tr("&Send coins");
    QString txtReceive = tr("&Receive coins");
    QString txtHistory = tr("&History");
    QString txtAddressBook = tr("&Address Book");
#ifdef USE_FULLSCREEN
    txtOverview = tr("&Wallet");
    txtSend = tr("&Send");
    txtReceive = tr("&Receive");
#endif

    overviewAction = new QAction(QIcon(":/icons/overview"), txtOverview, this);
    overviewAction->setToolTip(tr("Show general overview of wallet"));
    overviewAction->setCheckable(true);
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), txtSend, this);
    sendCoinsAction->setToolTip(tr("Send coins to a ArtBoomerang address"));
    sendCoinsAction->setCheckable(true);
    tabGroup->addAction(sendCoinsAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), txtReceive, this);
    receiveCoinsAction->setToolTip(tr("Show the list of addresses for receiving payments"));
    receiveCoinsAction->setCheckable(true);
    tabGroup->addAction(receiveCoinsAction);

    historyAction = new QAction(QIcon(":/icons/history"), txtHistory, this);
    historyAction->setCheckable(true);
    historyAction->setToolTip(tr("Browse transaction history"));
    tabGroup->addAction(historyAction);

    addressBookAction = new QAction(QIcon(":/icons/address-book"), txtAddressBook, this);
    addressBookAction->setCheckable(true);
    addressBookAction->setToolTip(tr("Edit the list of stored addresses and labels"));
    tabGroup->addAction(addressBookAction);

#ifndef USE_FULLSCREEN
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
#else
    //connect(historyAction, SIGNAL(hovered()), historyAction, SLOT(showT()));

#endif

    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
//#ifdef USE_FULLSCREEN
//    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(openAddressBookPage()));
//#else
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
//#endif

    donateAction = new QAction(QIcon(":/icons/donate"), tr("Donate ArtBoomerang"), this);
    donateAction->setToolTip(tr("Edit the list of stored addresses and labels"));

    quitAction = new QAction(QIcon(":/icons/exit"), tr("E&xit"), this);
    quitAction->setToolTip(tr("Quit application"));
#ifndef USE_FULLSCREEN
    donateAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
#endif
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/app_about"), tr("&About ArtBoomerang"), this);
    aboutAction->setToolTip(tr("Show information about ArtBoomerang"));
    aboutAction->setMenuRole(QAction::AboutRole);
#if QT_VERSION < 0x050000
        aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#else
        aboutQtAction = new QAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#endif
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setToolTip(tr("Modify configuration options for ArtBoomerang"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(QIcon(":/icons/bitcoin"), tr("&Show / Hide"), this);
    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet..."), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    loadWalletAction = new QAction(QIcon(":/icons/file_load"), tr("&Load Wallet..."), this);
    loadWalletAction->setToolTip(tr("Load existing wallet"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet"));
    lockWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Wallet"), this);
    lockWalletAction->setToolTip(tr("Lock wallet"));
    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);

    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    exportAction->setToolTip(tr("Export the data in the current tab to a file"));
    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));


    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction, SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(backupWalletAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(loadWalletAction, SIGNAL(triggered()), this, SLOT(loadWallet()));
    connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
    connect(unlockWalletAction, SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(lockWalletAction, SIGNAL(triggered()), this, SLOT(lockWallet()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
    connect(donateAction, SIGNAL(triggered()), this, SLOT(donateClicked()));


    // set disabled/hidden by default
    exportAction->setVisible(false);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();

#endif

    QMenu *file = appMenuBar->addMenu(tr("&ArtBoomerang"));
    file->addAction(backupWalletAction);
    file->addSeparator();
    file->addAction(loadWalletAction);
    file->addSeparator();
    file->addAction(exportAction);
    file->addAction(signMessageAction);
    file->addAction(verifyMessageAction);
    file->addSeparator();
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    settings->addAction(encryptWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addAction(unlockWalletAction);
    settings->addAction(lockWalletAction);
    settings->addSeparator();
    settings->addAction(optionsAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(donateAction);
    help->addSeparator();
    help->addAction(aboutAction);
#ifndef USE_FULLSCREEN
        help->addAction(aboutQtAction);
#endif


}

float BitcoinGUI::getPointSize(const QFont& font) {
    // Workaround against QTBUG-4958
    // QFontInfo can properly convert from pixels to points but returns
    // wrong value if the font has already a size in points.
    if (font.pixelSize() != -1) {
        // Size is set in pixels, convert to points
        float pointSize=QFontInfo(font).pointSizeF();
        return pointSize;
    }
    else {
        // Size is set in points, no conversion needed
        return font.pointSizeF();
    }
}
void BitcoinGUI::setActionBarMenu(ActionBar *bar)
{
#ifdef USE_FULLSCREEN
    bar->clearMenu();
    bar->addMenu(donateAction);
    bar->addMenuSeparator();
    bar->addMenu(backupWalletAction);
    bar->addMenu(loadWalletAction);
    bar->addMenuSeparator();
    bar->addMenu(encryptWalletAction);
    bar->addMenu(changePassphraseAction);
    bar->addMenu(unlockWalletAction);
    bar->addMenu(lockWalletAction);
    bar->addMenuSeparator();
    //if(exportAction->isEnabled()){
    //    actionBar->addMenu(exportAction);
    //}
    bar->addMenuSeparator();
    bar->addMenu(optionsAction);
    bar->addMenu(openRPCConsoleAction);
    bar->addMenuSeparator();
    bar->addMenu(aboutAction);
    bar->addMenuSeparator();
    bar->addMenu(quitAction);
#endif

}

void BitcoinGUI::createToolBars()
{

    actionBar =new ActionBar(this);
    actionBar->addNavigation(overviewAction);

#ifdef USE_FULLSCREEN
    actionBar->addButton(historyAction);
    actionBar->addButton(addressBookAction);
    setMenuWidget(actionBar);
    // toolBar (addToolBar) is a pointer to an existing toolbar
    mainToolbar = addToolBar(tr("Tabs toolbar"));
    actionBar->addTabsBar(mainToolbar);

    actionBar->addTab(overviewAction);
    actionBar->addTab(sendCoinsAction);
    actionBar->addTab(receiveCoinsAction);

#else
    mainToolbar = addToolBar(tr("Tabs toolbar"));
    actionBar->addButton(overviewAction);
    actionBar->addButton(sendCoinsAction);
    actionBar->addButton(receiveCoinsAction);
    actionBar->addButton(historyAction);
    actionBar->addButton(addressBookAction);
    actionBar->setupToolBar(mainToolbar);

#endif
  mainToolbar->setContextMenuPolicy(Qt::NoContextMenu);


  // for movable toolbar only
    /*
    connect(mainToolbar, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(mainToolbarOrientation(Qt::Orientation)));
    connect(secondaryToolbar, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(secondaryToolbarOrientation(Qt::Orientation)));
    mainToolbarOrientation(mainToolbar->orientation());
    secondaryToolbarOrientation(secondaryToolbar->orientation());
  */
}

void BitcoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Replace some strings and icons, when using the testnet
        if(clientModel->isTestNet())
        {
            setWindowTitle(windowTitle() + QString(" ") + tr("[testnet]"));
#ifndef Q_OS_MAC
            qApp->setWindowIcon(QIcon(":icons/bitcoin_testnet"));
            setWindowIcon(QIcon(":icons/bitcoin_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/bitcoin_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(tr("ArtBoomerang client") + QString(" ") + tr("[testnet]"));
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
                toggleHideAction->setIcon(QIcon(":/icons/toolbar_testnet"));
            }

            aboutAction->setIcon(QIcon(":/icons/toolbar_testnet"));
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks(), clientModel->getNumBlocksOfPeers());
        connect(clientModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        rpcConsole->setClientModel(clientModel);
        addressBookPage->setOptionsModel(clientModel->getOptionsModel());
        receiveCoinsPage->setOptionsModel(clientModel->getOptionsModel());

    }
}
void BitcoinGUI::updateSpinner()
{
    spinnerTimer->start();
    labelBlocksIcon->setPixmap(QIcon(QString(":/movies/spinner-%1").arg(spinnerFrame, 3, 10, QChar('0')))
            .pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
    spinnerFrame = (spinnerFrame + 1) % SPINNER_FRAMES;
}

void BitcoinGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);

        overviewPage->setModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        signVerifyMessageDialog->setModel(walletModel);

        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon pop-up for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));
    }
}



void BitcoinGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("ArtBoomerang client"));
    trayIcon->setIcon(QIcon(":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif

    notificator = new Notificator(qApp->applicationName(), trayIcon, this);
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHideAction->trigger();
    }
}
#endif

void BitcoinGUI::donateClicked()
{
    if(!walletModel)
        return;
    DonateDialog dlg(this,screen);
    dlg.setModel(walletModel);
    dlg.exec();
}
void BitcoinGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;
    OptionsDialog dlg(this,screen);
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void BitcoinGUI::aboutClicked()
{
    AboutDialog dlg(this,screen);
    dlg.setModel(clientModel);
    dlg.exec();
}


void BitcoinGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to ArtBoomerang network", "", count));
}

void BitcoinGUI::setNumBlocks(int count, int nTotalBlocks)
{
    // don't show / hide progress bar and its label if we have no connection to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    QString strStatusBarWarnings = clientModel->getStatusBarWarnings();
    QString tooltip;

    labelBlocksIcon->setFixedSize(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    labelBlocksIcon->show();

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    QDateTime currentDate = QDateTime::currentDateTime();
    int secs = lastBlockDate.secsTo(currentDate);

    tooltip = tr("Processed %1 blocks of transaction history.").arg(count);

    // Set icon state: spinning if catching up, tick otherwise
    //if(secs < 70)
    int64_t peerBlks = GetNumBlocksOfPeers();
    if((peerBlks>0 && nBestHeight > peerBlks-3) || secs < 240)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        spinnerTimer->stop();
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

        overviewPage->showOutOfSyncWarning(false);

        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);
    }
    else
    {
        updateSpinner();
        overviewPage->showOutOfSyncWarning(true);

        // Represent time from last generated block in human readable text
        QString timeBehindText;
        const int HOUR_IN_SECONDS = 60*60;
        const int DAY_IN_SECONDS = 24*60*60;
        const int WEEK_IN_SECONDS = 7*24*60*60;
        const int YEAR_IN_SECONDS = 31556952; // Average length of year in Gregorian calendar
        if(secs < 120)
        {
            timeBehindText = tr("%n minute(s)","",secs/(60));
        }
        if(secs < 2*DAY_IN_SECONDS)
        {
            timeBehindText = tr("%n hour(s)","",secs/HOUR_IN_SECONDS);
        }
        else if(secs < 2*WEEK_IN_SECONDS)
        {
            timeBehindText = tr("%n day(s)","",secs/DAY_IN_SECONDS);
        }
        else if(secs < YEAR_IN_SECONDS)
        {
            timeBehindText = tr("%n week(s)","",secs/WEEK_IN_SECONDS);
        }
        else
        {
            int years = secs / YEAR_IN_SECONDS;
            int remainder = secs % YEAR_IN_SECONDS;
            timeBehindText = tr("%1 and %2").arg(tr("%n year(s)", "", years)).arg(tr("%n week(s)","", remainder/WEEK_IN_SECONDS));
        }

        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        if(count < nTotalBlocks)
        {
            if (strStatusBarWarnings.isEmpty())
            {
                progressBarLabel->setText(tr("Synchronizing with network..."));
                progressBarLabel->setVisible(true);
                progressBar->setFormat(tr("%1 behind").arg(timeBehindText));
                progressBar->setMaximum(nTotalBlocks);
                progressBar->setValue(count);
                progressBar->setVisible(true);

#ifdef USE_FULLSCREEN
                QFont font = progressBarLabel->font(); font.setPointSize(10);
                progressBarLabel->setFont(font);
#endif
            }
        }
        else
        {
            if (strStatusBarWarnings.isEmpty())
                progressBarLabel->setVisible(false);

            progressBar->setVisible(false);
            tooltip = tr("Downloaded %1 blocks of transaction history.").arg(count);
        }



        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1 ago.").arg(timeBehindText);
        tooltip += QString("<br>");
        tooltip += tr("Transactions after this will not yet be visible.");
    }
    // Override progressBarLabel text and hide progress bar, when we have warnings to display
    if (!strStatusBarWarnings.isEmpty())
    {
        progressBarLabel->setText(strStatusBarWarnings);
        progressBarLabel->setVisible(true);
        progressBar->setVisible(false);
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);


}

void BitcoinGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}

void BitcoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}
void BitcoinGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_OS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            if(rpcConsole)
                rpcConsole->hide();

            qApp->quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void BitcoinGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
    QString strMessage =
        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, nFeeRequired));
    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Confirm transaction fee"), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    *payFee = (retval == QMessageBox::Yes);
}

void BitcoinGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
    if(!clientModel->inInitialBlockDownload())
    {
        // On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));

        notificator->notify(Notificator::Information,
                            (amount)<0 ? tr("Sent transaction") :
                                         tr("Incoming transaction"),
                              tr("Date: %1\n"
                                 "Amount: %2\n"
                                 "Type: %3\n"
                                 "Address: %4\n")
                              .arg(date)
                              .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                              .arg(type)
                              .arg(address), icon);
    }
}


void BitcoinGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);
#ifdef USE_FULLSCREEN
    setActionBarMenu(actionBar);
#endif
    actionBar->removeButton(exportAction);
    exportAction->setVisible(false);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoHistoryPage()
{

    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);

    exportAction->setVisible(true);
    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
    actionBar->addButton(exportAction);
#ifdef USE_FULLSCREEN
    setActionBarMenu(actionBar);
#endif

}

void BitcoinGUI::openAddressBookPage(){

    addressBookAction->setChecked(true);
    AddressBookPage a(AddressBookPage::ForEditing, AddressBookPage::SendingTab, this, screen);
    a.setIsDialogMode(true);
    //a->setOptionsModel(clientModel->getOptionsModel());
    a.setModel(walletModel->getAddressTableModel());
    //a.open();
    a.exec();
    a.close();

}

void BitcoinGUI::gotoAddressBookPage()
{
    //addressBookPage->setIsDialogMode(true);
    //addressBookPage->open();
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);

    exportAction->setVisible(true); exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
    actionBar->addButton(exportAction);
#ifdef USE_FULLSCREEN
    //addressBookPage->setActionBarMenu(actionBar);
    setActionBarMenu(actionBar);
#else

#endif

}

void BitcoinGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);

    exportAction->setEnabled(true);
    exportAction->setVisible(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
    actionBar->addButton(exportAction);
#ifdef USE_FULLSCREEN
    //addressBookPage->setActionBarMenu(actionBar);
    setActionBarMenu(actionBar);
#endif
}

void BitcoinGUI::gotoSendCoinsPage()
{
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);

    exportAction->setVisible(false);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    actionBar->removeButton(exportAction);
#ifdef USE_FULLSCREEN
    setActionBarMenu(actionBar);
#endif

}



void BitcoinGUI::gotoSignMessageTab(QString addr)
{
    // call show() in showTab_SM()
    signVerifyMessageDialog->showTab_SM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void BitcoinGUI::gotoVerifyMessageTab(QString addr)
{
    // call show() in showTab_VM()
    signVerifyMessageDialog->showTab_VM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);
}



void BitcoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        int nValidUrisFound = 0;
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            if (sendCoinsPage->handleURI(uri.toString()))
                nValidUrisFound++;
        }

        // if valid URIs were found
        if (nValidUrisFound)
            gotoSendCoinsPage();
        else
            notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid ArtBoomerang address or malformed URI parameters."));
    }

    event->acceptProposedAction();
}

void BitcoinGUI::handleURI(QString strURI)
{
    // URI has to be valid
    if (sendCoinsPage->handleURI(strURI))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
    }
    else
        notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid ArtBoomerang address or malformed URI parameters."));
}

void BitcoinGUI::mainToolbarOrientation(Qt::Orientation orientation)
{
    if(orientation == Qt::Horizontal)
    {
        //mainIcon->setPixmap(QPixmap(":/icons/app_about"));
        //mainIcon->show();
        mainToolbar->setStyleSheet(HORIZONTAL_TOOLBAR_STYLESHEET);
    }
    else
    {
        //mainIcon->setPixmap(QPixmap(":images/icons/app_about"));
        //mainIcon->show();

        mainToolbar->setStyleSheet(VERTICAL_TOOBAR_STYLESHEET);
    }
}

void BitcoinGUI::secondaryToolbarOrientation(Qt::Orientation orientation)
{
   secondaryToolbar->setStyleSheet(orientation == Qt::Horizontal ? HORIZONTAL_TOOLBAR_STYLESHEET : VERTICAL_TOOBAR_STYLESHEET);
}

void BitcoinGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        changePassphraseAction->setEnabled(false);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setChecked(false);
        encryptWalletAction->setEnabled(true);
        encryptWalletAction->setText(tr("&Encrypt Wallet..."));
        break;
    case WalletModel::Unlocked:
        disconnect(labelEncryptionIcon, SIGNAL(clicked()), unlockWalletAction, SLOT(trigger()));
        disconnect(labelEncryptionIcon, SIGNAL(clicked()),   lockWalletAction, SLOT(trigger()));
        connect   (labelEncryptionIcon, SIGNAL(clicked()),   lockWalletAction, SLOT(trigger()));
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>")); // TODO: Click to lock + translations
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setText(tr("Wallet Encrypted"));
        encryptWalletAction->setChecked(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        disconnect(labelEncryptionIcon, SIGNAL(clicked()), unlockWalletAction, SLOT(trigger()));
        disconnect(labelEncryptionIcon, SIGNAL(clicked()),   lockWalletAction, SLOT(trigger()));
        connect   (labelEncryptionIcon, SIGNAL(clicked()), unlockWalletAction, SLOT(trigger()));
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>")); // TODO: Click to unlock + translations
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setText(tr("Wallet Encrypted"));
        encryptWalletAction->setChecked(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}

void BitcoinGUI::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt: AskPassphraseDialog::Decrypt, this, screen);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void BitcoinGUI::loadWallet()
{
    copyingProgressSize = 0;
    QMessageBox::StandardButton retval= QMessageBox::question(this, tr("Load Existing Wallet"),
           "<qt>" +
           tr("<b>IMPORTANT: ArtBoomerang will close after finish copying process.</b>"
           "<br/>Be sure to select correct and not corrupted wallet file from your backup folder."
           "<br/>If you have coins in your current wallet - backup your wallet before copy!") +
           "<br/><br/>Do you want to continue?<br/>"
           "</qt>",
               QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);
    if(retval != QMessageBox::Yes){
        return;
    }
    QString myDir;
#if QT_VERSION < 0x050000
        myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
        myDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
    QFileDialog *fd = new QFileDialog(this, tr("Load Wallet Database"), myDir,
                                      tr("Wallet Database files (*.dat)"));
#ifdef USE_FULLSCREEN
    fd->setFixedSize(screen->virtualSize());
#endif
    QString dataDir = QString("%1").arg(GetDataDir().string().c_str());
    QString newWallet = dataDir + "/wallet.dat.copy";
    copyingProgressSize = 0;
    QString loadWallet;
    if(fd->exec())
    {
        QStringList list = fd->selectedFiles();
        loadWallet = list.first();
    }
    if(!loadWallet.isEmpty())
    {
        QFile newWalletFile(newWallet);
        if(newWalletFile.exists()){
            newWalletFile.remove();
        }
        QFile loadWalletFile(loadWallet);
        int sz = QFileInfo(loadWalletFile).size() / 1024;
        copyingProgressSize = sz;
        copyingProgress = new QProgressDialog(this);
        copyingProgress->setWindowModality(Qt::WindowModal);
        copyingProgress->setLabelText(tr("Copying wallet file (%1 Mb)...").arg(sz/1024));
        copyingProgress->setCancelButtonText(tr("Abort Copy"));
        copyingProgress->setRange(0, sz);
        copyingProgressTimer = new QTimer(this);copyingProgressTimer->setInterval(200);
        connect(copyingProgress, SIGNAL(canceled()), copyingProgressTimer, SLOT(stop()));
        connect(copyingProgressTimer, SIGNAL(timeout()), this, SLOT(updateCopyingProgress()));
        copyingProgressTimer->start();
        copyingProgress->show();
        if(!loadWalletFile.open(QIODevice::ReadOnly)) return;
        if(loadWalletFile.copy(newWalletFile.fileName())){
            loadWalletFile.close();
            if(true == walletModel->loadNewWallet(newWallet)){ //  rename in db model;
                copyingProgressTimer->stop(); copyingProgress->close();
                   QMessageBox::information(this, tr("Load Existing Wallet"),
                       tr("Your wallet copied.<br/>ArtBoomerang will close now to reload the database."),
                       QMessageBox::Ok);
                QApplication::quit();
            }
        }
    }
}

void BitcoinGUI::updateCopyingProgress(){
    QString dataDir = QString("%1").arg(GetDataDir().string().c_str());
    QFileInfo info(dataDir + "/wallet.dat.copy");
    int sz = info.size()/1024;
    copyingProgress->setValue(sz);
    if(copyingProgressSize>0 && sz == copyingProgressSize){
        copyingProgressTimer->stop(); copyingProgress->close();
    }
}

void BitcoinGUI::updateCopyingProgress(qint64 w){
    copyingProgressSize += w;
    copyingProgress->setValue( copyingProgressSize / 1024 );
}

void BitcoinGUI::backupWallet()
{
    QString saveDir;
#if QT_VERSION < 0x050000
        saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
        saveDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!walletModel->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void BitcoinGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this, screen);
    dlg.setModel(walletModel);
    dlg.exec();
}

void BitcoinGUI::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog::Mode mode = sender() == unlockWalletAction ?
            AskPassphraseDialog::UnlockStaking : AskPassphraseDialog::Unlock;
        AskPassphraseDialog dlg(mode, this, screen);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void BitcoinGUI::lockWallet()
{
    if(!walletModel)
        return;

    walletModel->setWalletLocked(true);
}

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void BitcoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void BitcoinGUI::updateWeight()
{
    if (!pwalletMain)
        return;

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;

    TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
    if (!lockWallet)
        return;

    pwalletMain->GetStakeWeight(*pwalletMain, nWeight);
}

void BitcoinGUI::updateStakingIcon()
{
    updateWeight();

    if (nLastCoinStakeSearchInterval && nWeight)
    {
        uint64_t nNetworkWeight = GetPoSKernelPS();
        unsigned nEstimateTime = nStakeTargetSpacing * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
            text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
            text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
            text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
            text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }

        labelStakingIcon->setPixmap(QIcon(":/icons/staking_on").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelStakingIcon->setToolTip(tr("Staking.<br>Your weight is %1<br>Network weight is %2<br>Expected time to earn reward is %3").arg(nWeight).arg(nNetworkWeight).arg(text));
    }
    else
    {
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_off").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        if (pwalletMain && pwalletMain->IsLocked())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is locked"));
        else if (vNodes.empty())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is offline"));
        else if (IsInitialBlockDownload())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is syncing"));
        else if (!nWeight)
            labelStakingIcon->setToolTip(tr("Not staking because you don't have mature coins"));
        else
            labelStakingIcon->setToolTip(tr("Not staking"));
    }
}
