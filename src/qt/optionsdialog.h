// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "guiutil.h"
#ifdef USE_FULLSCREEN
    #include "tabslider/tabSlider.h"
    #include "actionbar/actionbar.h"
#endif

namespace Ui {
class OptionsDialog;
}
class OptionsModel;
class MonitoredDataMapper;
class QValidatedLineEdit;

/** Preferences dialog. */
class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0, BitScreen *screen=0);
    ~OptionsDialog();
    void accept();

    void setModel(OptionsModel *model);
    void setMapper();
    void setCurrentTab(int index);

protected:
#ifdef USE_FULLSCREEN
    ActionBar *actionBar;
#endif
    bool event(QEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mapperSubmit();

private slots:
    void screenOrientationChanged();
    void cleanExternalAccounts();
    /* enable only apply button */
    void enableApplyButton();
    /* disable only apply button */
    void disableApplyButton();
    /* enable apply button and OK button */
    void enableSaveButtons();
    /* disable apply button and OK button */
    void disableSaveButtons();
    /* set apply button and OK button state (enabled / disabled) */
    void setSaveButtonState(bool fState);
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_applyButton_clicked();

    void showRestartWarning_Proxy();
    void showRestartWarning_Lang();
    void updateDisplayUnit();
    void handleProxyIpValid(QValidatedLineEdit *object, bool fState);
    void secureClearPassFields();
    void smtpChanged(int index);

signals:
    void proxyIpValid(QValidatedLineEdit *object, bool fValid);

private:
    Ui::OptionsDialog *ui;
    OptionsModel *model;
    MonitoredDataMapper *mapper;
    bool fRestartWarningDisplayed_Proxy;
    bool fRestartWarningDisplayed_Lang;
    bool fProxyIpValid;
    BitScreen *screen;
    bool fCapsLock;
#ifdef USE_FULLSCREEN
    TabSlider *tabSlider;
#endif

};

#endif // OPTIONSDIALOG_H
