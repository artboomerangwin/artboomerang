/**
  @file
  @author Stefan Frings
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

    //ActionBar* bar=new ActionBar(this);
    //connect(bar,&ActionBar::up,this, BitcoinGUI::gotoOverviewPage);
    //connect(bar, SIGNAL(up()), this, SLOT(gotoOverviewPage()));
*/

#include "actionbar.h"
#include <QIcon>
#include <QFontMetrics>
#include <QFont>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QMenuBar>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QScreen>

ActionBar::ActionBar(QWidget *parent) : QWidget(parent) {
   // "* {background:lightGray}"  #f6f7fa  #dadbde
    setStyleSheet(
        "ActionBar {background:transparent;}"
        "ActionBar {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);}"
        "QToolButton {height:2em;}"
        "QToolButton QMenu::item {padding: 0.3em 1.5em 0.3em 1.5em; border: 1px solid transparent;}"
        "QToolButton QMenu::item::selected {border-color: black;}"
     );

    // "QToolButton {height:1.6em}"
    //  "QToolButton#viewControl {font:bold}"
    //QWidget::setMaximumSize: (viewControl/QToolButton) Negative sizes (-98,16777215) are not possible

    setObjectName("actionBar");
    em=fontMetrics().height();
    topTabsBarExists = false;
    isHomePage = true;
    // setMinimumHeight(2.5*em);

    layout=new QHBoxLayout(this); layout->setSpacing(0); layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0); layout->setSizeConstraint(QLayout::SetNoConstraint);
    //layout->setSizeConstraint(QLayout::SetDefaultConstraint);

    // icon size added in adjustContent()
    // App Icon and Up Button
    appIcon=new QToolButton();
#ifdef USE_FULLSCREEN
    appIcon->setIcon(QIcon(":/android_icons/app"));
    appIcon->setIconSize(QSize(2*em,2*em));
#else
    appIcon->setIcon(QIcon(":/icons/app_full"));
#endif
    appIcon->setIconSize(QSize(2*em,2*em));
    appIcon->setAutoRaise(true);
    appIcon->setCheckable(false);
    appIcon->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(appIcon);

    // View Control Button
    viewControl=new QToolButton();
    viewControl->setObjectName("viewControl");
    //viewControl->setText(QApplication::applicationDisplayName());
    viewControl->setText(tr("ArtBoomerang"));
    viewControl->setAutoRaise(true);
    viewControl->setFocusPolicy(Qt::NoFocus);
    viewControl->setPopupMode(QToolButton::InstantPopup);
    viewControl->setCheckable(false);
    viewMenu=new QMenu(viewControl);
    viewMenu->setStyle(&menuStyle); // needed because the icon size cannot be set by a StyleSheet
    //viewControl->setMenu(viewMenu);
    //viewMenu->setDisabled(true);
    layout->addWidget(viewControl);

    // Spacer
    layout->addStretch();

    // Action Overflow Button
    overflowButton=new QToolButton();
    overflowButton->setIcon(QIcon(":/android_icons/overflow"));
    overflowButton->setIconSize(QSize(2*em,2*em));
    overflowButton->setToolTip(tr("more"));
    overflowButton->setAutoRaise(true);
    overflowButton->setFocusPolicy(Qt::NoFocus);
    overflowButton->setPopupMode(QToolButton::InstantPopup);
    overflowMenu=new QMenu(overflowButton);
    overflowMenu->setStyle(&menuStyle); // needed because the icon size cannot be set by a StyleSheet
    overflowButton->setMenu(overflowMenu);
    overflowButton->sizeHint();
    layout->addWidget(overflowButton);

    menuButton=new QToolButton();
    menuButton->setIcon(QIcon(":/android_icons/overflow"));
    menuButton->setIconSize(QSize(2*em,2*em));
    menuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    menuButton->setToolTip(tr("more"));
    menuButton->setAutoRaise(true);
    menuButton->setFocusPolicy(Qt::NoFocus);
    menuButton->setCheckable(false);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    actionBarButtonMenu=new QMenu(menuButton);
    actionBarButtonMenu->setStyle(&menuStyle);
    menuButton->setMenu(actionBarButtonMenu);
    menuButton->sizeHint();
    menuButton->hide(); // menuButton: show only if menu already added
    layout->addWidget(menuButton);
    //menuActions = QList();

    //actionGroup = new QActionGroup(this);
#ifdef USE_FULLSCREEN
  //  QScreen *screen = QApplication::screens().at(0);
  //  connect(screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
#endif
}

void ActionBar::screenOrientationChanged()
{
#ifdef USE_FULLSCREEN
    if(!menuActions.isEmpty()){
        if(actionBarButtonMenu->isVisible()){
            actionBarButtonMenu->move(this->geometry().right()-actionBarButtonMenu->width(), this->geometry().bottom());
        }
    }
    adjustTabsSize();
#endif
}

ActionBar::~ActionBar() {}

/**
 * @brief ActionBar::addMenu
 * @param b
 * if menu action triggered - do nothing, action should be already set.
 * if action has menu - open messagebox with submenu actions list
 * submenu actions should be already set too in parent class
 */

void ActionBar::removeMenu(QAction *action)
{
    menuActions.removeOne(action);
    actionBarButtonMenu->removeAction(action);
}
void ActionBar::clearMenu()
{
    menuActions.clear();
    actionBarButtonMenu->clear();
}

void ActionBar::removeMenu(const QString &menutitle){
    for (int i=0; i<menuActions.size(); i++) {
        if (menuActions.at(i)->text() == menutitle) {
            QAction *action;
            action=menuActions.at(i);
            removeMenu(action);
            break;
        }
    }
}

void ActionBar::addMenuSeparator(){
    actionBarButtonMenu->addSeparator();
}

void ActionBar::addMenu(QMenu *menu){
    // Todo: adding existing menu
    QAction *a = new QAction(menu->icon(),menu->title(),this);
   /*     menu->setVisible(false);
    connect(menu, SIGNAL(aboutToShow()), menu, SLOT(hide()));
    connect(menu, SIGNAL(hovered()), menu, SLOT(hide()));
    connect(menu, SIGNAL(triggered()), menu, SLOT(hide()));
    connect(a, SIGNAL(hovered()), menu, SLOT(hide()));
    connect(a, SIGNAL(toggled()), menu, SLOT(hide()));
    connect(a, SIGNAL(triggered()), menu, SLOT(hide()));
    */
    a->setMenu(menu);
    // menuButton->setPopupMode(QToolButton::DelayedPopup);

    //menu->hide();
    addMenu(a);
    connect(a, SIGNAL(hovered()), this, SLOT(showPopUpDialogMenu()));
   // connect(a, SIGNAL(toggled()), this, SLOT(showPopUpDialogMenu()));
   // connect(a, SIGNAL(triggered()), this, SLOT(showPopUpDialogMenu()));
}
QMenu *ActionBar::addMenu(const QString &menutitle){

    QMenu *m = new QMenu(menutitle, this); m->setFixedSize(QSize(0,0));  m->resize(0,0);
    addMenu(m);
    return m;
}

void ActionBar::addMenu(QAction *action)
{
    menuActions.append(action);
    actionBarButtonMenu->addAction(action);

    /* remove: menuActions.removeOne(action);
*/
   // QMenuBar *menuBar = parent->menuBar();
    //menuBar = actionMenuBar;
    //menuBar = actionMenuBar->findChild<QToolButton*>("qt_menubar_ext_button");
    //QToolButton *b = menuBar->findChild<QToolButton*>("qt_menubar_ext_button");
    //b=new QToolButton();
    /*
QMessageBox *box = new QMessageBox("title", "text", QMessageBox::NoIcon, QMessageBox::Save, QMessageBox::Close, QMessageBox::Open);
box->button(QMessageBox::Save)->setText("Save part");
box->show();
QMessageBox msgBox;
msgBox.setText(trUtf8("Eliminar CD"));
msgBox.setInformativeText(record.value(cd_titulo).toString());
QAbstractButton *myYesButton = msgBox.addButton(trUtf8("Sim"), QMessageBox::YesRole);
QAbstractButton *myNoButton = msgBox.addButton(trUtf8("Não"), QMessageBox::NoRole);
msgBox.setIcon(QMessageBox::Question);
msgBox.exec();
qmessagebox.cpp uses buttonBox = new QDialogButtonBox; and the addButton() method

d->buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
d->customButtonList.append(button);
QPushButton *btnCopy = new QPushButton(tr("C&opy Details"));
    ui->buttonBox->addButton(btnCopy, QDialogButtonBox::NoRole);

 QMessageBox msgBox;
 QPushButton *connectButton = msgBox.addButton(tr("Connect"), QMessageBox::ActionRole);
 QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);

 msgBox.exec();

 if (msgBox.clickedButton() == connectButton) {
     // connect
 } else if (msgBox.clickedButton() == abortButton) {
     // abort
 }
*/

}

void ActionBar::showPopUpDialogMenu()
{
    if(actionBarButtonMenu->activeAction()){
        int size = actionBarButtonMenu->activeAction()->menu()->actions().count();
        if(size>0){
            actionBarButtonMenu->activeAction()->menu()->setVisible(false);
            QMessageBox *msgBox =  new QMessageBox(QMessageBox::Warning, QString("title"), QString("text"), 0, this);

            msgBox->setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox->setButtonText( QMessageBox::Save, tr("btn text") );
            msgBox->show();

        }
    }

}

void ActionBar::setupToolBar(QToolBar* tbar){
    tbar->setMovable(false);
    tbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tbar->setObjectName("actionBarToolBar");
    tbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // ToolButtonIconOnly
    tbar->setContentsMargins(QMargins(0,0,0,0));
    tbar->addWidget(this);
}

void ActionBar::addTabsBar(QToolBar* tbar)
{
    topTabsBar = tbar;
    topTabsBar->setObjectName("topToolbar");
    topTabsBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // ToolButtonIconOnly
    topTabsBar->setIconSize(QSize(1.5*em,1.5*em));
    topTabsBar->setFloatable(false);
    topTabsBar->setMovable(false);
    topTabsBar->setContentsMargins(QMargins(0,0,0,0));

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    QWidget* spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    topTabsBarLeftCorner = topTabsBar->addWidget(spacer);
    topTabsBarRightCorner = topTabsBar->addWidget(spacer2);
    topTabsBarExists=true;
}
void ActionBar::addTab(QAction* action)
{
    if(!topTabsBarExists){ return; }
      topTabsBar->removeAction(topTabsBarRightCorner);
      topTabsBar->addAction(action);
      topTabsBar->addAction(topTabsBarRightCorner);
}
void ActionBar::adjustTabsSize(){
    if(!topTabsBarExists){ return; }
    if(topTabsBar->actions().isEmpty()){  return; }
    int i=0; int last = topTabsBar->actions().size()-1;
    if(last==1){ return; }
    int w = (this->width()/(last-1))-4; // 4 - possible button border/spacing, should be detected correct way
    foreach (QAction* action, topTabsBar->actions()){ i++;
        if(i==1||i>last) { continue;}
        topTabsBar->widgetForAction(action)->setMinimumWidth(w);
    }
}

void ActionBar::resizeEvent(QResizeEvent* event) {
    int oldWidth=event->oldSize().width();
    int newWidth=event->size().width();
    if (oldWidth!=newWidth) {
        adjustContent();
#ifdef USE_FULLSCREEN
        screenOrientationChanged();
#endif
    }
}

void ActionBar::paintEvent(QPaintEvent*) {
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void ActionBar::setTitle(const QString& title, bool showUpButton) {
    viewControl->setText(title);
    int em=fontMetrics().height();
    if (showUpButton) {
        isHomePage = true;
        appIcon->setIcon(QIcon(":/android_icons/app_up"));
        appIcon->setToolTip(tr("Up"));
        appIcon->setIconSize(QSize(2*em,2*em));
        connect(appIcon, &QToolButton::clicked, this, &ActionBar::appIconClicked);
    }
    else {
        isHomePage = false;
        appIcon->setIcon(QIcon(":/android_icons/app"));
        appIcon->setToolTip("");
        appIcon->setIconSize(QSize(3*em,2*em));
        disconnect(appIcon, &QToolButton::clicked, this, &ActionBar::appIconClicked);
    }
    adjustContent();
}

QString ActionBar::getTitle() {
    return viewControl->text();
}

void ActionBar::appIconClicked() {
    emit up();
}

void ActionBar::adjustContent() {
#ifdef USE_FULLSCREEN
    if(menuActions.size()>0){
        menuButton->show();
    }else{
        menuButton->hide();
    }
#endif
    int appLogoButtonSize = appIcon->sizeHint().width() + viewControl->sizeHint().width();
    if(viewMenu->actions().count()==0){
        viewControl->hide();
        appIcon->setText(viewControl->text());
        appIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        appLogoButtonSize = appIcon->sizeHint().width();
    }else{
        viewControl->show();
        appIcon->setToolButtonStyle(Qt::ToolButtonIconOnly);
        viewMenu->setMinimumWidth(viewControl->sizeHint().width());
    }
    viewMenu->repaint();
    // Get size of one em (text height in pixels)
    // update size of app icon and overflow menu button
    //appIcon->setIconSize(QSize(3*em,3*em));
    //overflowButton->setIconSize(QSize(2*em,2*em));
    buttonIconOnlySize = overflowButton->size();
    viewControl->setMaximumWidth(this->width()-6*em);

    // Check if all action buttons fit into the available space with text beside icon.
    bool needOverflow=false;
    int space=width() - appLogoButtonSize;

    int lastTextButton = 0;
    for (int i=0; i<buttonActions.size(); i++) {
        QAction* action=buttonActions.at(i);
        QToolButton* button=actionButtons.at(i);
        if (action->isVisible()) {
            button->setIconSize(QSize(2*em,2*em));
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            space-=button->sizeHint().width();
            if(space<0){
                break;
            }
            lastTextButton++;
        }
    }
    if (space<0) {
        // Not enough space.
        // Check if all action buttons fit into the available space without text.
        int space=width() - appLogoButtonSize -overflowButton->sizeHint().width();
        for (int i=0; i<buttonActions.size(); i++) {
            QAction* action=buttonActions.at(i);
            QToolButton* button=actionButtons.at(i);
            if (action->isVisible()) {
                button->setToolButtonStyle(Qt::ToolButtonIconOnly);
                space-=button->sizeHint().width();
            }
        }
        if (space<0) {
            // The buttons still don't fit, we need an overflow menu.
            needOverflow=true;
        }
    }
    // Calculate space available to display action buttons
    overflowMenu->clear();
    space=width() - appLogoButtonSize;
    if (needOverflow) {
        space-=overflowButton->sizeHint().width();
    }

    // Show/Hide action buttons and overflow menu entries
    for (int i=0; i<buttonActions.size(); i++) {
        QAction* action=buttonActions.at(i);
        QToolButton* button=actionButtons.at(i);
        if (action->isVisible()) {
            space-=button->sizeHint().width();
            if (space>=0) {
                // show as button
                button->setDisabled(!buttonActions.at(i)->isEnabled());
                button->show();
            }
            else {
                // show as overflow menu entry
                button->hide();
                overflowMenu->addAction(action);
            }
        }
    }

    // Show/Hide the overflow menu button
    if (needOverflow) {
        if(overflowMenu->actions().count()>0)
            overflowButton->show();
    }
    else {
        overflowButton->hide();
    }
}

void ActionBar::addNavigation(QAction* action) {
    QWidget::addAction(action);
    viewMenu->addAction(action);
    if (!viewMenu->isEmpty()) {
        viewControl->setMenu(viewMenu);
    }
}

void ActionBar::addNavigations(QList<QAction*> actions) {
    QWidget::addActions(actions);
    viewMenu->addActions(actions);
    for (int i=0; i<actions.size(); i++) {
        addAction(actions.at(i));
    }
}

void ActionBar::removeNavigation(QAction* action) {
    QWidget::removeAction(action);
    viewMenu->removeAction(action);
    if (viewMenu->isEmpty()) {
        viewControl->setMenu(NULL);
    }
}

void ActionBar::addButton(QAction* action, int position) {
    if(!buttonActions.isEmpty() && buttonActions.contains(action)){
        return;
    }
    if (position==-1) {
        position=buttonActions.size();
    }
    buttonActions.insert(position,action);
    QToolButton* button=new QToolButton();
    button->setIconSize(QSize(2*em,2*em));
    button->setText(action->text());
    button->setToolTip(action->text());
    button->setIcon(action->icon());
    button->setDisabled(!action->isEnabled());
    button->setFocusPolicy(Qt::NoFocus);
    button->setAutoRaise(true);
    button->setCheckable(action->isCheckable());
    connect(button,&QToolButton::clicked,action,&QAction::trigger);
    connect(button, SIGNAL(clicked()), this, SLOT(buttonSetState()));
    connect(action, SIGNAL(toggled(bool)), button, SLOT(setChecked(bool)));
    actionButtons.insert(position,button);
    layout->insertWidget(position+3,button);
}
void ActionBar::buttonSetState(){
    for (int i=0; i<buttonActions.size(); i++) {
        QAction* action=buttonActions.at(i);
        QToolButton* button=actionButtons.at(i);
        button->setDisabled(!action->isEnabled());
    }
}

void ActionBar::removeButton(QAction* action) {
    QToolButton* button=NULL;
    for (int i=0; i<buttonActions.size(); i++) {
        if (buttonActions.at(i)==action) {
            button=actionButtons.at(i);
            break;
        }
    }
    if (button) {
        layout->removeWidget(button);
        actionButtons.removeOne(button);
        delete button;
        buttonActions.removeOne(action);
    }
}

void ActionBar::openOverflowMenu() {
    if (overflowButton->isVisible()) {
        overflowButton->click();
    }
}


