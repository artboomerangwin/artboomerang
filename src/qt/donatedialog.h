// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef DONATEDIALOG_H
#define DONATEDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QShowEvent>
#include <QString>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "walletmodel.h"
#include "guiutil.h"

namespace Ui {
class DonateDialog;
}

class DonateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DonateDialog(QWidget *parent = 0, BitScreen *screen=0);
    ~DonateDialog();
    void accept();
    void setModel(WalletModel *model);

signals:
    windowReady();
    addressReady(  );

private slots:
    void on_btnSend_clicked();
    void on_btnCancel_clicked();
    void screenOrientationChanged();
    void readReply(QNetworkReply*reply);
    void preloadDonationAddress();
    void updateDonationAddress();

private:
    Ui::DonateDialog *ui;
    WalletModel *model;
    BitScreen *screen;
    QString d_address;
    QNetworkAccessManager m_networkAccessManager;

protected:
    void keyPressEvent(QKeyEvent *event);
    void showEvent( QShowEvent* event );
    void focusEvent( QFocusEvent* event );
};

#endif // DONATEDIALOG_H
