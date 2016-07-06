// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef ASKPASSPHRASEDIALOG_H
#define ASKPASSPHRASEDIALOG_H

#include <QDialog>
#include "guiutil.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif

namespace Ui {
    class AskPassphraseDialog;
}

class WalletModel;

/** Multifunctional dialog to ask for passphrases. Used for encryption, unlocking, and changing the passphrase.
 */
class AskPassphraseDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        Encrypt,       /**< Ask passphrase twice and encrypt */
        UnlockStaking, /**< Ask passphrase and unlock */
        Unlock,        /**< Ask passphrase and unlock */
        ChangePass,    /**< Ask old passphrase + new passphrase twice */
        Decrypt        /**< Ask passphrase and decrypt wallet */
    };

    explicit AskPassphraseDialog(Mode mode, QWidget *parent = 0, BitScreen *screen=0);
    ~AskPassphraseDialog();

    void accept();

    void setModel(WalletModel *model);

private:
    BitScreen *screen;
    Ui::AskPassphraseDialog *ui;
    Mode mode;
    WalletModel *model;
    bool fCapsLock;

private slots:
    void screenOrientationChanged();
    void textChanged();
    bool event(QEvent *event);
    bool eventFilter(QObject *, QEvent *event);
    void secureClearPassFields();
};

#endif // ASKPASSPHRASEDIALOG_H
