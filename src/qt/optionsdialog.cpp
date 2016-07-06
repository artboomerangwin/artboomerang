// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "guiconstants.h"
#include "bitcoinunits.h"
#include "monitoreddatamapper.h"
#include "netbase.h"
#include "optionsmodel.h"
#include <QDir>
#include <QIntValidator>
#include <QLocale>
#include <QMessageBox>
#include <QRegExp>
#include <QRegExpValidator>
#include <QKeyEvent>
#include <QScroller>
#include <QScrollBar>
#include <QFormLayout>

#include <QDebug>

OptionsDialog::OptionsDialog(QWidget *parent, BitScreen *screen) : QDialog(parent),
    ui(new Ui::OptionsDialog),
    model(0),
    mapper(0),
    fRestartWarningDisplayed_Proxy(false),
    fRestartWarningDisplayed_Lang(false),
    fProxyIpValid(true),
    screen(screen),
    fCapsLock(false)
{
    ui->setupUi(this);
    setWindowTitle(tr("ArtBoomerang: Settings"));
    ui->eAccUname->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->eAccPwd->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->eAccMailFrom->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->eAccSmtp->setMaxLength(35);
    ui->eAccSmtpPort->setMaxLength(5);
    ui->eAccUname->setPlaceholderText(tr("Your Email username"));
    ui->eAccPwd->setPlaceholderText(tr("Your Email password"));
    ui->eAccMailFrom->setPlaceholderText("your_email@email.com");
    ui->eAccSmtp->setPlaceholderText("smtp.gmail.com");
    ui->eAccSmtpPort->setPlaceholderText("465");
    ui->eAccSmtpPort->setValidator( new QIntValidator(1, 99999, this) );
    ui->eAccUname->installEventFilter(this);
    ui->eAccPwd->installEventFilter(this);
    ui->eAccSmtp->installEventFilter(this);
    ui->eAccMailFrom->installEventFilter(this);

    ui->eAccEmail->addItem("SMTP settings disabled", QVariant("false"));
    ui->eAccEmail->addItem("Gmail", QVariant("smtp.gmail.com"));
    ui->eAccEmail->addItem("Yahoo! Mail", QVariant("smtp.mail.yahoo.com"));
    ui->eAccEmail->addItem("Hotmail", QVariant("smtp.live.com"));
    ui->eAccEmail->addItem("Other...", QVariant("other"));


    /* Network elements init */
#ifndef USE_UPNP
    ui->mapPortUpnp->setEnabled(false);
#endif

    ui->proxyIp->setEnabled(false);
    ui->proxyPort->setEnabled(false);
    ui->proxyPort->setValidator(new QIntValidator(1, 65535, this));

    ui->socksVersion->setEnabled(false);
    ui->socksVersion->addItem("5", 5);
    ui->socksVersion->addItem("4", 4);
    ui->socksVersion->setCurrentIndex(0);

    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->proxyIp, SLOT(setEnabled(bool)));
    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->proxyPort, SLOT(setEnabled(bool)));
    connect(ui->connectSocks, SIGNAL(toggled(bool)), ui->socksVersion, SLOT(setEnabled(bool)));
    connect(ui->connectSocks, SIGNAL(clicked(bool)), this, SLOT(showRestartWarning_Proxy()));

    ui->proxyIp->installEventFilter(this);
    ui->tabWidget->installEventFilter(this);

    /* Window elements init */
    ui->comboBoxThemes->hide(); ui->labelThemes->hide();
#ifdef Q_OS_MAC
    //ui->tabWindow->setVisible(false);
    ui->tabWidget->removeTab(2);
#endif

    QAbstractItemView *qv = ui->lang->view();
    qv->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(qv->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(qv->viewport(), QScroller::LeftMouseButtonGesture);
    qv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->eAccScrollArea->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(ui->eAccScrollArea->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(ui->eAccScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    ui->eAccScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->eAccScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);



    /* Display elements init */
    QDir translations(":translations");
    ui->lang->addItem(QString("(") + tr("default") + QString(")"), QVariant(""));
    foreach(const QString &langStr, translations.entryList())
    {
        QLocale locale(langStr);

        /** check if the locale name consists of 2 parts (language_country) */
        if(langStr.contains("_"))
        {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language - native country (locale name)", e.g. "Deutsch - Deutschland (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" - ") + locale.nativeCountryName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language - country (locale name)", e.g. "German - Germany (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" - ") + QLocale::countryToString(locale.country()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
        else
        {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language (locale name)", e.g. "Deutsch (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language (locale name)", e.g. "German (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
    }

    ui->unit->setModel(new BitcoinUnits(this));

    /* Widget-to-option mapper */
    mapper = new MonitoredDataMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);

    /* enable apply button when data modified */
    connect(mapper, SIGNAL(viewModified()), this, SLOT(enableApplyButton()));
    /* disable apply button when new data loaded */
    connect(mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(disableApplyButton()));
    /* setup/change UI elements when proxy IP is invalid/valid */
    connect(this, SIGNAL(proxyIpValid(QValidatedLineEdit *, bool)), this, SLOT(handleProxyIpValid(QValidatedLineEdit *, bool)));

#ifdef USE_FULLSCREEN
    ui->coinControlFeatures->hide(); // ToDo: require to find space on Android screen to display this feature
    ui->coinControlFeatures->setChecked(false);
    setAttribute(Qt::WA_AcceptTouchEvents);
    tabSlider = new TabSlider(this, ui->tabWidget);
    ui->tabWidget->removeTab(2);
    actionBar =new ActionBar(this);
    ui->actionBarLayout->addWidget(actionBar);
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));
    actionBar->setTitle(windowTitle(), true);

    screenOrientationChanged();
    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
#endif


}

void OptionsDialog::screenOrientationChanged(){
#ifdef USE_FULLSCREEN
    setFixedSize(screen->virtualSize());
    //screen->adjustPopupDialogButtonsSize(this, ui->buttonBox)
    int w = (screen->size().width()-(ui->verticalLayout->contentsMargins().left()*2)-9)/3;
    ui->horizontalLayout_Buttons->removeItem(ui->horizontalSpacer_1);
    ui->okButton->setFixedWidth(w);
    ui->cancelButton->setFixedWidth(w);
    ui->applyButton->setFixedWidth(w);

    ui->eAccLabelHelp->sizeHint();
    if(screen->size().width()<400){
        ui->eAccForm->setRowWrapPolicy(QFormLayout::WrapAllRows);
        ui->eAccForm_2->setRowWrapPolicy(QFormLayout::WrapAllRows);
    }else{
        ui->eAccForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
        ui->eAccForm_2->setRowWrapPolicy(QFormLayout::DontWrapRows);
    }
    if(screen->isPortrait()){
        int w = screen->size().width()/2;
        ui->proxyIp->setMaximumWidth(w);
        ui->proxyPort->setMaximumWidth(w);
        ui->socksVersion->setMaximumWidth(w);
        ui->formLayoutProxy->addRow(ui->proxyIpLabel, ui->proxyIp);
        ui->formLayoutProxy->addRow(ui->proxyPortLabel, ui->proxyPort);
        ui->formLayoutProxy->addRow(ui->socksVersionLabel, ui->socksVersion);

        ui->mapPortUpnp->setMaximumWidth(screen->size().width()-36);
        ui->connectSocks->setMaximumWidth(screen->size().width()-36);

        ui->eAccForm->addRow(ui->eAccUnameLabel, ui->eAccUname);
        ui->eAccForm->addRow(ui->eAccPwdLabel, ui->eAccPwd);
        ui->eAccForm->addRow(ui->eAccMailFromLabel, ui->eAccMailFrom);
    }else{
        ui->mapPortUpnp->setMaximumWidth(screen->size().width()-36);
        ui->connectSocks->setMaximumWidth(screen->size().width()-36);
        ui->horizontalLayout_Network->removeItem(ui->horizontalSpacer_Network);
        ui->horizontalLayout_Network->addWidget(ui->proxyIpLabel);
        ui->horizontalLayout_Network->addWidget(ui->proxyIp);
        ui->horizontalLayout_Network->addWidget(ui->proxyPortLabel);
        ui->horizontalLayout_Network->addWidget(ui->proxyPort);
        ui->horizontalLayout_Network->addWidget(ui->socksVersionLabel);
        ui->horizontalLayout_Network->addWidget(ui->socksVersion);
        ui->horizontalLayout_Network->addItem(ui->horizontalSpacer_Network);

        ui->eAccForm_2->addRow(ui->eAccUnameLabel, ui->eAccUname);
        ui->eAccForm_2->addRow(ui->eAccPwdLabel, ui->eAccPwd);
        ui->eAccForm_2->addRow(ui->eAccMailFromLabel, ui->eAccMailFrom);
    }
    ui->eAccScrollAreaWidgetContents->resize(ui->eAccScrollAreaWidgetContents->sizeHint());
    ui->tabWidget->sizeHint();
#endif
}

OptionsDialog::~OptionsDialog()
{
    secureClearPassFields();
    delete ui;
}

void OptionsDialog::setCurrentTab(int index)
{
    if(index>= 0 || index<= ui->tabWidget->count()){
        ui->tabWidget->setCurrentIndex(index);
    }

}
void OptionsDialog::setModel(OptionsModel *model)
{
    this->model = model;

    if(model)
    {
        connect(model, SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

        mapper->setModel(model);
        setMapper();
        mapper->toFirst();
    }

    /* update the display unit, to not use the default ("BTC") */
    updateDisplayUnit();

    /* warn only when language selection changes by user action (placed here so init via mapper doesn't trigger this) */
    connect(ui->lang, SIGNAL(valueChanged()), this, SLOT(showRestartWarning_Lang()));

    if(model->getEaccountEmail()){
        QString s = model->getEaccountEmailSmtp();
        if(s=="smtp.gmail.com" || s=="smtp.mail.yahoo.com" || s=="smtp.live.com"){
            ui->eAccEmail->setValue(model->getEaccountEmailSmtp());
            ui->eAccEmail->setItemText(4, "Other...");
        }else{
            ui->eAccEmail->setValue(QVariant("other"));
            ui->eAccEmail->setItemText(4, "SMTP server enabled");
        }
        ui->eAccEmail->setItemText(0, "Disable SMTP");
    }else{
        ui->eAccEmail->setCurrentIndex(0);
        cleanExternalAccounts();
    }
    connect(ui->eAccEmail, SIGNAL(currentIndexChanged(int)), this, SLOT(smtpChanged(int)));

    /* disable apply button after settings are loaded as there is nothing to save */
    disableApplyButton();

}

void OptionsDialog::setMapper()
{
    /* Main */
    mapper->addMapping(ui->transactionFee, OptionsModel::Fee);
    mapper->addMapping(ui->reserveBalance, OptionsModel::ReserveBalance);
    mapper->addMapping(ui->bitcoinAtStartup, OptionsModel::StartAtStartup);
    mapper->addMapping(ui->detachDatabases, OptionsModel::DetachDatabases);

    /* Network */
    mapper->addMapping(ui->mapPortUpnp, OptionsModel::MapPortUPnP);

    mapper->addMapping(ui->connectSocks, OptionsModel::ProxyUse);
    mapper->addMapping(ui->proxyIp, OptionsModel::ProxyIP);
    mapper->addMapping(ui->proxyPort, OptionsModel::ProxyPort);
    mapper->addMapping(ui->socksVersion, OptionsModel::ProxySocksVersion);

    /* Window */
#ifndef Q_OS_MAC
    mapper->addMapping(ui->minimizeToTray, OptionsModel::MinimizeToTray);
    mapper->addMapping(ui->minimizeOnClose, OptionsModel::MinimizeOnClose);
#endif

    /* Display */
    mapper->addMapping(ui->lang, OptionsModel::Language);
    mapper->addMapping(ui->unit, OptionsModel::DisplayUnit);
    mapper->addMapping(ui->displayAddresses, OptionsModel::DisplayAddresses);
    mapper->addMapping(ui->coinControlFeatures, OptionsModel::CoinControlFeatures);

    /* External Accounts */
    mapper->addMapping(ui->eAccEmail, OptionsModel::EaccountEmail);
    mapper->addMapping(ui->eAccUname, OptionsModel::EaccountEmailUser);
    mapper->addMapping(ui->eAccPwd, OptionsModel::EaccountEmailPwd);
    mapper->addMapping(ui->eAccMailFrom, OptionsModel::EaccountEmailFrom);
    mapper->addMapping(ui->eAccSmtp, OptionsModel::EaccountEmailSmtp);
    mapper->addMapping(ui->eAccSmtpPort, OptionsModel::EaccountEmailPort);
}

void OptionsDialog::enableApplyButton()
{
    ui->applyButton->setEnabled(true);
}

void OptionsDialog::disableApplyButton()
{
    ui->applyButton->setEnabled(false);
}

void OptionsDialog::enableSaveButtons()
{
    /* prevent enabling of the save buttons when data modified, if there is an invalid proxy address present */
    if(fProxyIpValid)
        setSaveButtonState(true);
}

void OptionsDialog::disableSaveButtons()
{
    setSaveButtonState(false);
}

void OptionsDialog::setSaveButtonState(bool fState)
{
    ui->applyButton->setEnabled(fState);
    ui->okButton->setEnabled(fState);
}

void OptionsDialog::on_okButton_clicked()
{
    mapperSubmit();
    accept();
}

void OptionsDialog::on_cancelButton_clicked()
{
    reject();
}

void OptionsDialog::on_applyButton_clicked()
{
    mapperSubmit();
}



void OptionsDialog::smtpChanged(int index)
{
    cleanExternalAccounts();
    ui->eAccUname->clear(); ui->eAccPwd->clear(); ui->eAccMailFrom->clear();
    ui->eAccLabelHelp->setText("");
    ui->eAccEmail->setItemText(4, "Other...");
    ui->eAccEmail->setItemText(0, "Disable SMTP settings");
    switch(index){
        case 0:
          ui->eAccEmail->setItemText(0, "SMTP settings disabled");
            break;
        case 1: // gmail
        ui->eAccSmtpPort->setText("465"); ui->eAccSmtp->setText(ui->eAccEmail->value().toString());
      break;
        case 2: // yahoo!
        ui->eAccSmtpPort->setText("465"); ui->eAccSmtp->setText(ui->eAccEmail->value().toString());
      break;
        case 3: // hotmail
          ui->eAccSmtpPort->setText("465"); ui->eAccSmtp->setText(ui->eAccEmail->value().toString());
        break;
        case 4: // other/custom...
        ui->eAccSmtpPort->clear(); ui->eAccSmtp->clear();
            break;
    }

}
void OptionsDialog::cleanExternalAccounts()
{
    bool c = ui->eAccEmail->value().toString() == "false" ? false : true;
    if(c == false){
        ui->eAccUname->clear(); ui->eAccPwd->clear(); ui->eAccMailFrom->clear();
        ui->eAccSmtp->clear(); ui->eAccSmtpPort->clear();
    }
    ui->eAccUname->setEnabled(c); ui->eAccPwd->setEnabled(c); ui->eAccMailFrom->setEnabled(c);
    ui->eAccSmtp->setEnabled(c); ui->eAccSmtpPort->setEnabled(c);

}
void OptionsDialog::showRestartWarning_Proxy()
{
    if(!fRestartWarningDisplayed_Proxy)
    {
        QMessageBox::warning(this, tr("Warning"), tr("This setting will take effect after restarting ArtBoomerang."), QMessageBox::Ok);
        fRestartWarningDisplayed_Proxy = true;
    }
}

void OptionsDialog::showRestartWarning_Lang()
{
    if(!fRestartWarningDisplayed_Lang)
    {
        QMessageBox::warning(this, tr("Warning"), tr("This setting will take effect after restarting ArtBoomerang."), QMessageBox::Ok);
        fRestartWarningDisplayed_Lang = true;
    }
}

void OptionsDialog::updateDisplayUnit()
{
    if(model)
    {
        /* Update transactionFee with the current unit */
        ui->transactionFee->setDisplayUnit(model->getDisplayUnit());
    }
}


void OptionsDialog::handleProxyIpValid(QValidatedLineEdit *object, bool fState)
{
    // this is used in a check before re-enabling the save buttons
    fProxyIpValid = fState;

    if(fProxyIpValid)
    {
        enableSaveButtons();
        ui->statusProxy->clear();
    }
    else
    {
        disableSaveButtons();
        object->setValid(fProxyIpValid);
        ui->statusProxy->setStyleSheet("QLabel { color: red; }");
        ui->statusProxy->setText(tr("The supplied proxy address is invalid."));
    }
}
/*
void OptionsDialog::updateTheme()
{
    //connect(comboBoxThemes, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTheme()));


    QFile f(":qdarkstyle/style.qss");
    if (f.exists())
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    QFile f(":qlightstyle/style.qss");
    if (f.exists())
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

}
*/
bool OptionsDialog::event(QEvent *event)
{
    // Detect Caps Lock key press.
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_CapsLock) {
            fCapsLock = !fCapsLock;
        }
        if (fCapsLock) {
            ui->eAccLabelHelp->setText(tr("Warning: The Caps Lock key is on!"));
        } else {
            ui->eAccLabelHelp->clear();
        }
    }
    return QWidget::event(event);
}


bool OptionsDialog::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::FocusOut)
    {
        if(object == ui->proxyIp)
        {
            CService addr;
            /* Check proxyIp for a valid IPv4/IPv6 address and emit the proxyIpValid signal */
            emit proxyIpValid(ui->proxyIp, LookupNumeric(ui->proxyIp->text().toStdString().c_str(), addr));
        }
        if(object == ui->eAccUname || object == ui->eAccSmtp){
            if(ui->eAccSmtp->text() == "smtp.gmail.com"){
                bool isEmail = GUIUtil::validateEmailAddress(ui->eAccUname->text());
                if(isEmail == false) {
                    ui->eAccLabelHelp->setText(tr("Your full Gmail or Google Apps email address required for authentication"));
                }else{
                    ui->eAccLabelHelp->clear();
                }
            }else{
                 ui->eAccLabelHelp->clear();
            }
        }
        else if(object == ui->eAccMailFrom){
            bool isEmail = GUIUtil::validateEmailAddress(ui->eAccMailFrom->text());
            if(isEmail == false) {
                    ui->eAccLabelHelp->setText(tr("Email format is incorrect"));
            }else{
                    ui->eAccLabelHelp->clear();
            }
        }
    }
    /* Detect Caps Lock.
     * There is no good OS-independent way to check a key state in Qt, but we
     * can detect Caps Lock by checking for the following condition:
     * Shift key is down and the result is a lower case character, or
     * Shift key is not down and the result is an upper case character.
     */
    if (event->type() == QEvent::KeyPress) {
        if(object == ui->eAccPwd) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            QString str = ke->text();
            if (str.length() != 0) {
                const QChar *psz = str.unicode();
                bool fShift = (ke->modifiers() & Qt::ShiftModifier) != 0;
                if ((fShift && psz->isLower()) || (!fShift && psz->isUpper())) {
                    fCapsLock = true;
                    ui->eAccLabelHelp->setText(tr("Warning: The Caps Lock key is on!"));
                } else if (psz->isLetter()) {
                    fCapsLock = false;
                    ui->eAccLabelHelp->clear();
                }
            }
        }
        else if(object == ui->eAccUname || object == ui->eAccSmtp){
            if(ui->eAccSmtp->text() == "smtp.gmail.com"){
                bool isEmail = GUIUtil::validateEmailAddress(ui->eAccUname->text());
                if(isEmail == false) {
                    ui->eAccLabelHelp->setText(tr("Your full Gmail or Google Apps email address required for authentication"));
                }else{
                    ui->eAccLabelHelp->clear();
                }
            }else{
                ui->eAccLabelHelp->clear();
            }
        }
        if(object == ui->eAccMailFrom){
            // ui->eAccMailFrom->setText(ui->eAccMailFrom->text().remove(" "));
            // should to remove but adding additional incorrect char at theend of string
        }
    }

#ifdef USE_FULLSCREEN
    tabSlider->listenEvent(object, event);
#endif
    return QDialog::eventFilter(object, event);
}


void OptionsDialog::keyPressEvent(QKeyEvent *event)
{
#ifdef USE_FULLSCREEN
    if(windowType() != Qt::Widget && event->key() == Qt::Key_Back)
    {
        close();
    }
#else
    if(windowType() != Qt::Widget && event->key() == Qt::Key_Escape)
    {
        close();
    }
#endif
}

void OptionsDialog::secureClearPassFields()
{
    // Attempt to overwrite text so that they do not linger around in memory
    ui->eAccPwd->setText(QString(" ").repeated(ui->eAccPwd->text().size()));
    ui->eAccPwd->clear();
}

void OptionsDialog::mapperSubmit()
{
    if(ui->eAccEmail->value().toString() != "false")
    {
        if(ui->eAccSmtp->text().isEmpty()){
            QMessageBox::information(this, tr("External Account Option"),
                                 tr("Email account SMTP Server is empty."));
            ui->eAccSmtp->setFocus();

        }else if(ui->eAccUname->text().isEmpty()){
            QMessageBox::information(this, tr("External Account Option"),
                                 tr("Email account username is empty."));
            ui->eAccUname->setFocus();

        }else if(ui->eAccSmtp->text() == "smtp.gmail.com"
                 && false == GUIUtil::validateEmailAddress(ui->eAccUname->text())){
                QMessageBox::information(this, tr("External Account Option"),
                   tr("Your full Gmail or Google Apps email address required for authentication."));
                ui->eAccUname->setFocus();

        }
        else if(ui->eAccPwd->text().isEmpty()){
            QMessageBox::information(this, tr("External Account Option"),
                                 tr("Email account password is empty."));
            ui->eAccPwd->setFocus();

        }else if(ui->eAccMailFrom->text().isEmpty()){
                QMessageBox::information(this, tr("External Account Option"),
                                     tr("\"Mail From\" address is empty."));
                ui->eAccMailFrom->setFocus();

        }else if(false == GUIUtil::validateEmailAddress(ui->eAccMailFrom->text())){
                QMessageBox::information(this, tr("External Account Option"),
                                     tr("Incorrect Email-From address."));
                ui->eAccMailFrom->setFocus();

        }else if(ui->eAccSmtpPort->text().isEmpty()){
                    QMessageBox::information(this, tr("External Account Option"),
                                         tr("Please set SMTP Port."));
               ui->eAccSmtpPort->setFocus();

        }else if(ui->eAccSmtp->text().contains("smtp.gmail.com")&&!ui->eAccMailFrom->text().contains("@gmail.")){
            QMessageBox::information(this, tr("External Account Option"),
                                 tr("\"Mail From\" is not a Gmail address."));
            ui->eAccMailFrom->setFocus();
        }
        else if(ui->eAccSmtp->text().contains(".yahoo.")&&!ui->eAccMailFrom->text().contains("@yahoo.")){
            QMessageBox::information(this, tr("External Account Option"),
                                 tr("\"Mail From\" is not a Yahoo! Mail address."));
            ui->eAccMailFrom->setFocus();

        }else if((ui->eAccEmail->currentIndex()==2 || ui->eAccEmail->currentIndex()==3)
                && false == GUIUtil::validateEmailAddress(ui->eAccUname->text())
                && !ui->eAccMailFrom->text().contains(ui->eAccUname->text()+"@"))
        {
                QMessageBox::information(this, tr("External Account Option"),
                                     tr("\"Mail From\" is not correct address for username \"%1\".").arg(ui->eAccUname->text()));
                ui->eAccMailFrom->setFocus();

        }else if((ui->eAccEmail->currentIndex()==2 || ui->eAccEmail->currentIndex()==3)
                && true == GUIUtil::validateEmailAddress(ui->eAccUname->text())
                && ui->eAccMailFrom->text() != ui->eAccUname->text())
        {
                QMessageBox::information(this, tr("External Account Option"),
                                     tr("\"Mail From\" is not correct address for username \"%1\".").arg(ui->eAccUname->text()));
                ui->eAccMailFrom->setFocus();

        }else{
            mapper->submit();
            disableApplyButton();
        }
    }else{
        mapper->submit();
        disableApplyButton();
    }
}
void OptionsDialog::accept()
{

    secureClearPassFields();
    QDialog::accept();

}
