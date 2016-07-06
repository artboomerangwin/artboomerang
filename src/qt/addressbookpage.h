// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef ADDRESSBOOKPAGE_H
#define ADDRESSBOOKPAGE_H

#include <QDialog>
#include <guiutil.h>
#ifdef USE_FULLSCREEN
#include <actionbar/actionbar.h>
#endif

namespace Ui {
    class AddressBookPage;
}
class AddressTableModel;
class OptionsModel;

QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
class QMenu;
class QModelIndex;
QT_END_NAMESPACE

/** Widget that shows a list of sending or receiving addresses.
  */
class AddressBookPage : public QDialog
{
    Q_OBJECT

public:
    enum Tabs {
        SendingTab = 0,
        ReceivingTab = 1
    };

    enum Mode {
        ForSending, /**< Open address book to pick address for sending */
        ForEditing  /**< Open address book for editing */
    };

    explicit AddressBookPage(Mode mode, Tabs tab, QWidget *parent = 0, BitScreen *screen=0);
    ~AddressBookPage();

    void setModel(AddressTableModel *model);
    void setOptionsModel(OptionsModel *optionsModel);
    const QString &getReturnValue() const { return returnValue; }
    void setIsDialogMode(bool isDialog);

public slots:
    void done(int retval);
    void exportClicked();
    void fixLayoutsSize(const int &w, const int &h);
#ifdef USE_FULLSCREEN
    void setActionBarMenu(ActionBar *bar);
#endif

private:
    bool isDialogMode;
    void resizeEvent(QResizeEvent *e);
    Ui::AddressBookPage *ui;
    AddressTableModel *model;
    OptionsModel *optionsModel;
    Mode mode;
    Tabs tab;
    BitScreen *screen;
    QWidget *parent;
#ifdef USE_FULLSCREEN
    ActionBar *actionBar;
#endif

    QString returnValue;
    QSortFilterProxyModel *proxyModel;
    QString newAddressToSelect;
    QMenu   *contextMenu;
    QAction *deleteAction;
    QAction *copyLabelAction;
    QAction *copyAddressAction;
    QAction *editAction;
    QAction *showQRCodeAction;
    QAction *signMessageAction;
    QAction *verifyMessageAction;
    QAction *exportAction;
    QAction *selectAddressAction;
    QAction *closeAction;

private slots:
    void copyLabelActionRelease();
    void enableActions(const QModelIndex &index);
    void screenOrientationChanged();
    void on_deleteButton_clicked();
    void on_newAddressButton_clicked();
    /** Copy address of currently selected address entry to clipboard */
    void on_copyToClipboard_clicked();
    void on_signMessage_clicked();
    void on_verifyMessage_clicked();
    void selectionChanged();
    void on_showQRCode_clicked();
    void on_btnRequestPayment_clicked();
    /** Spawn contextual menu (right mouse menu) for address book entry */
    void contextualMenu(const QPoint &point);

    /** Copy label of currently selected address entry to clipboard */
    void onCopyLabelAction();
    /** Edit currently selected address entry */
    void onEditAction();

    /** New entry/entries were added to address table */
    void selectNewAddress(const QModelIndex &parent, int begin, int end);
protected:
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void signMessage(QString addr);
    void verifyMessage(QString addr);
};

#endif // ADDRESSBOOKDIALOG_H
