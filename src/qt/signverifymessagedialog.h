// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIGNVERIFYMESSAGEDIALOG_H
#define SIGNVERIFYMESSAGEDIALOG_H

#include <QDialog>
#include <QWindow>
#include "guiutil.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif

namespace Ui {
    class SignVerifyMessageDialog;
}
class WalletModel;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class SignVerifyMessageDialog : public QDialog


{
    Q_OBJECT

public:
    explicit SignVerifyMessageDialog(QWidget *parent = 0, BitScreen *screen=0);
    ~SignVerifyMessageDialog();

    void setModel(WalletModel *model);
    void setAddress_SM(QString address);
    void setAddress_VM(QString address);

    void showTab_SM(bool fShow);
    void showTab_VM(bool fShow);

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);

private:
    Ui::SignVerifyMessageDialog *ui;
    WalletModel *model;
    BitScreen *screen;
#ifdef USE_FULLSCREEN
    ActionBar *actionBar;
#endif

private slots:
    void screenOrientationChanged();
    void clearAll();
    /* sign message */
    void on_addressBookButton_SM_clicked();
    void on_pasteButton_SM_clicked();
    void on_pasteButton_VM_clicked();
    void on_signMessageButton_SM_clicked();
    void on_copySignatureButton_SM_clicked();
    /* verify message */
    void on_addressBookButton_VM_clicked();
    void on_verifyMessageButton_VM_clicked();
    void on_pasteButtonVM2_clicked();
    void on_btnClearVM_clicked();
    void on_btnClearSM_clicked();
    void on_btnCloseVM_clicked();
    void on_btnCloseSM_clicked();
};

#endif // SIGNVERIFYMESSAGEDIALOG_H
