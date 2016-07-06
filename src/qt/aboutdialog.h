// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QWidget>
#include <QDialog>
#include "guiutil.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif
namespace Ui {
    class AboutDialog;
}
class ClientModel;

/** "About" dialog box */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0, BitScreen *screen=0);
    ~AboutDialog();

    void setModel(ClientModel *model);
private:
    Ui::AboutDialog *ui;
    void keyPressEvent(QKeyEvent *);

private slots:
    void on_pushButton_clicked();
};

#endif // ABOUTDIALOG_H
