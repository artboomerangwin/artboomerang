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

*/

#ifndef ACTIONBAR_H
#define ACTIONBAR_H

#include "menustyle.h"
#include <QWidget>
#include <QList>
#include <QAction>
#include <QMenu>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QToolButton>
#include <QProxyStyle>
#include <QStyleOption>
#include <QToolBar>
#include <QMenuBar>

/**
 * Toolbar similar to Android's Action Bar, can also be used on Desktop OS.
 * The action bar shows an icon, a title and any number of action buttons.
 * Also the title can have a menu of navigation actions.
 * <p>
 * If the buttons do not fit into the window, then they are displayed
 * as an "overflow" menu on the right side.
 * <p>
 * See http://developer.android.com/design/patterns/actionbar.html
 *
 * To be used within a vertical box layout this way:
 * <pre><code>
 * MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
 *     QVBoxLayout* layout=new QVBoxLayout(this);
 *     layout->setMargin(0);
 *     layout->setSizeConstraint(QLayout::SetNoConstraint);
 *
 *     // Action bar
 *     ActionBar* actionBar=new ActionBar(this);
 *     actionBar->setTitle("My App",false);
 *     actionBar->addNavigation(new QAction("News",this));
 *     actionBar->addNavigation(new QAction("Weather",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/search"),"Search",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/call"),"Call",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/settings"),"Settings",this));
 *     layout->addWidget(actionBar);
 *
 *     // Content of main window below the action bar
 *     layout->addWidget(new QLabel("Hello",this));
 *     layout->addStretch();
 * }
 * </code></pre>
 * The layout of the main window must use the option QLayout::SetNoConstraint,
 * to support screen rotation on mpbile devices.
 * <p>
 * The action bar needs two icons in the resource file:
 * <ul>
 *     <li>QIcon(":icons/app") is used for the initial display.
 *     <li>QIcon(":icons/app_up") is used when you enable "up" navigation by calling setTitle().
 * </ul>
 */
class ActionBar : public QWidget {
    Q_OBJECT

public:
    /**
     * Constructor, creates an Activity bar with icon and title but no action buttons.
     * The icon is loaded from the resource file via QIcon(":icons/app").
     * The title is taken from QApplication::applicationDisplayName().
     *
     * @param parent Points to the parent window.
     */
    explicit ActionBar(QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~ActionBar();

    /**
     * Set title of the action bar.
     * @param title Either the name of the application or title of the current view within the application.
     * @param showUpButton Enables "up" navigation. Then the action bar emits signal up() on click on the icon.
     */
    void setTitle(const QString& title, bool showUpButton);

    /** Get the current title of the action bar. */
    QString getTitle();
    void setupToolBar(QToolBar* tbar);

    /**
     * Adds a view navigation link to the title of the action bar.
     * @param action The action, containing at least a text and optinally an icon. The action emits signal triggered() when clicked.
     */
    void addNavigation(QAction* action);

    /**
     * Adds many view navigation links to the title of the action bar.
     * @param actions List of actions.
     * @see addAction()
     */
    void addNavigations(QList<QAction*> actions);

    /**
     * Removes a view navigation link from the title of the action bar.
     * @param action The action that had been added before.
     */
    void removeNavigation(QAction* action);

    /**
     * Adds an action button (or overflow menu item) to the action bar.
     * @param action The action, containing at least a text and optinally an icon. The action emits signal triggered() when clicked.
     * @param position Insert before this position. 0=before first button, 1=second button. Default is -1=append to the end.
     */
    void addButton(QAction* action, int position=-1);

    /**
     * Removes an action button (or overflow menu item) from the action bar.
     * @param action The action that had been added before.
     */
    void removeButton(QAction* action);

    /**
     * Adjust the number of buttons in the action bar. Buttons that don't fit go into the overflow menu.
     * You need to call this method after adding, removing or changing the visibility of an action.
     */
    void adjustContent();

    /**
     * Menu-only right button (not a buttons bar+overflow)
     */
    void clearMenu();
    void removeMenu(const QString &menutitle);
    void removeMenu(QAction *action);
    void addMenu(QAction *action);
    void addMenu(QMenu *menu);
    QMenu *addMenu(const QString &menutitle);
    void addMenuSeparator();

    void addTabsBar(QToolBar* tbar);
    void addTab(QAction* action);

signals:
    /** Emitted when the user clicks on the app icon (for "up" navigation) */
    void up();

public slots:
    void showPopUpDialogMenu();
    void buttonSetState();
    /** Can be used to open the overflow menu */
    void openOverflowMenu();

protected:
    /**
     * Overrides the paintEvent method to draw background color properly.
     */
    void paintEvent(QPaintEvent* event);

    /**
     * Overrides the resizeEvent to adjust the content depending on the new size.
     */
    void resizeEvent(QResizeEvent* event);

private:
    int em;
    QSize buttonIconOnlySize;
    bool isHomePage;

    /** Horizontal layout of the navigation bar */
    QHBoxLayout* layout;

    QToolButton *menuButton;
    QMenu *actionBarButtonMenu;
    QList<QAction*> menuActions;
    QList<QMenu*> actionBarMenuList;
    bool topTabsBarExists;
    QToolBar* topTabsBar;
    QAction* topTabsBarLeftCorner;
    QAction* topTabsBarRightCorner;

    /** The Button that contains the applications icon, used for "up" navigation. */
    QToolButton* appIcon;

    /** The button that contains the title, used for view navigation. */
    QToolButton* viewControl;

    /** The menu that appears when the user clicks on the title. */
    QMenu* viewMenu;

    /** List of actions for the action buttons and overflow menu. */
    QList<QAction*> buttonActions;

    /** List of action buttons, same order as buttonActions. */
    QList<QToolButton*> actionButtons;

    /** Overflow button, is only visible when there are more buttons than available space. */
    QToolButton* overflowButton;

    /** The menu that appears when the user clicks on the overflow button. */
    QMenu* overflowMenu;

    /** Used to control the size of icons in menu items. */
    MenuStyle menuStyle;

private slots:
    void screenOrientationChanged();
    void adjustTabsSize();
    /** Internally used to forward events from the appIcon button. */
    void appIconClicked();

};

#endif // ACTIONBAR_H
