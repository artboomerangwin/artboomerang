// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef QRCODEDIALOG_H
#define QRCODEDIALOG_H

#include <QDialog>
#include <QImage>
#include "guiutil.h"
#include "optionsmodel.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif

namespace Ui {
    class QRCodeDialog;
}
class OptionsModel;

class QRCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QRCodeDialog(const QString &addr, const QString &label, bool enableReq, QWidget *parent = 0, BitScreen *screen=0);
    ~QRCodeDialog();

    void setModel(OptionsModel *model);

private slots:
    void clearAll();
    void showFullScreen();
    void closeFullScreen();
    void screenOrientationChanged();
    void on_lnReqAmount_textChanged();
    void on_lnLabel_textChanged();
    void on_lnMessage_textChanged();
    void on_btnSaveAs_clicked();
    void on_btnCopyURI_clicked();
    void on_btnCopyAddress_clicked();
    void on_btnClose_clicked();
    void on_chkReqPayment_toggled(bool fChecked);

    void updateDisplayUnit();

private:
#ifdef USE_FULLSCREEN
    ActionBar *actionBar;
#endif

    QWidget *fullScreenQRCode;
    Ui::QRCodeDialog *ui;
    OptionsModel *model;
    QString address;
    QString addressURI;
    QImage myImage;
    BitScreen *screen;

    void genCode();
    QString getURI();
    bool eventFilter(QObject *object, QEvent *event);
protected:
    int qrImageWidth;
    int qrImageMinWidth;
};

#endif // QRCODEDIALOG_H
