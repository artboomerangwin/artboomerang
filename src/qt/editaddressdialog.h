// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef EDITADDRESSDIALOG_H
#define EDITADDRESSDIALOG_H

#include <QDialog>
#include "guiutil.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif
QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui {
    class EditAddressDialog;
}
class AddressTableModel;

/** Dialog for editing an address and associated information.
 */
class EditAddressDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        NewReceivingAddress,
        NewSendingAddress,
        EditReceivingAddress,
        EditSendingAddress
    };

    explicit EditAddressDialog(Mode mode, QWidget *parent = 0, BitScreen *screen=0);
    //EditAddressDialog(Mode mode, QWidget *parent = 0, BitScreen *screen=0,
    //            Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    ~EditAddressDialog();

    void setModel(AddressTableModel *model);
    void loadRow(int row);

    QString getAddress() const;
    void setAddress(const QString &address);

public slots:
    void accept();
private slots:
    void on_pasteButton_clicked();
    void screenOrientationChanged();

private:
    bool saveCurrentRow();

    QWidget *parent;
    Ui::EditAddressDialog *ui;
    QDataWidgetMapper *mapper;
    Mode mode;
    AddressTableModel *model;
    BitScreen *screen;

    QString address;
};

#endif // EDITADDRESSDIALOG_H
