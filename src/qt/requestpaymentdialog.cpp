#include "requestpaymentdialog.h"
#include "ui_requestpaymentdialog.h"

#include "bitcoinunits.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "optionsdialog.h"

#include <QPixmap>
#include <QUrl>
#include <QScroller>
#include <QScrollBar>
#include <QDesktopServices>
#include <QMovie>
#include <QClipboard>

#ifdef USE_QRCODE
#include <qrencode.h>
#endif
#include "plugins/smtp/smtp.h"

RequestPaymentDialog::RequestPaymentDialog(const QString &addr, const QString &label, QWidget *parent, BitScreen *screen) :
    QDialog(parent),
    ui(new Ui::RequestPaymentDialog),
    model(0),
    address(addr),
    screen(screen)
{
    ui->setupUi(this);
    ui->labelAddress->setText(address.left(35));

    setWindowTitle(QString(tr("Request Payment to: %1...")).arg(address.left(10)));
    this->appTitle = tr( "ArtBoomerang Payment Request" );
    ui->requestIncludeMessage->clear();
    ui->requestIncludeMessage->hide();
    ui->requestBy->addItem(tr("Email"));
    ui->requestBy->addItem(tr("Facebook"));
    ui->requestBy->setCurrentIndex(0);
    ui->btnOpenSettings->hide();
    ui->btnPasteMessage->hide();
    ui->frameSendForm->show();
    ui->frameProcessForm->hide();


    ui->scrollArea->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(this->ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    ui->scrollArea->horizontalScrollBar()->hide();
    ui->scrollArea->horizontalScrollBar()->resize(0, 0);
    //int em=fontMetrics().height();
    //ui->outUri->setMinimumHeight(2.5*em);
    //ui->outUri->setMaximumHeight(10*em);
    //ui->outUri->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


    ui->lnLabel->setText(label);
    ui->lnReqAmount->setFocus();

    ui->btnCopyAddress->setEnabled(false);
    ui->btnCopyURI->setEnabled(false);
    ui->btnSaveAs->setEnabled(false);

    //connect(ui->lnReqAmount, SIGNAL(returnPressed()),ui->lnLabel,SLOT(setFocus()));
    //connect(ui->lnLabel, SIGNAL(returnPressed()),ui->lnMessage,SLOT(setFocus()));
    //connect(ui->lnMessage, SIGNAL(returnPressed()),ui->requestBy,SLOT(setFocus()));
    //connect(ui->requestSendTo, SIGNAL(returnPressed()),ui->requestSubject,SLOT(setFocus()));
    //connect(ui->requestSubject, SIGNAL(returnPressed()),ui->chkIncludeMessage,SLOT(setFocus()));


#ifdef USE_FULLSCREEN
    ui->btnLayout->removeItem( ui->btnSpacer);
    ui->btnCancel->hide(); ui->btnCopyAddress->hide(); ui->btnCopyURI->hide(); ui->btnSaveAs->hide(); ui->btnClear->hide();
   // screen->adjustButtonSize(ui->btnSendRequest, ui->btnSendRequest->text());
  //  screen->adjustButtonSize(ui->btnPreviewRequest, ui->btnPreviewRequest->text());
    ui->btnSendRequest->setIconSize(screen->iconSize());
    ui->btnPreviewRequest->setIconSize(screen->iconSize());
    ui->btnRequestSendTo->setIconSize(screen->iconSize());
    ui->btnRequestCopyTo->setIconSize(screen->iconSize());
    ui->btnPasteMessage->setIconSize(screen->iconSize());

    actionBar =new ActionBar(this);
    ui->actionBarLayout->addWidget(actionBar);
    actionBar->setTitle(windowTitle(), true);
    ui->lblQRCode->setFixedSize(100,100);
    ui->lblQRCode->installEventFilter(this);
    QAction *copyAction = new QAction(QIcon(":/android_icons/action_copy"), tr("&Copy Address"), this);
    copyAction->setToolTip(tr("Copy Address to clipboard"));
    QAction *copyURIAction = new QAction(QIcon(":/android_icons/action_copy_uri"), tr("&Copy URI"), this);
    copyURIAction->setToolTip(tr("Copy URI to clipboard"));
    QAction *clearAction = new QAction(QIcon(":/android_icons/action_undo"), tr("Undo"), this);
    clearAction->setToolTip(tr("Undo Changes"));
    QAction *saveAction = new QAction(QIcon(":/android_icons/action_save"), tr("&Save As..."), this);
    saveAction->setToolTip(tr("Save QRCode image to a file"));
    QAction *fullScreenAction = new QAction(QIcon(":/android_icons/action_full_screen"), tr("&Full Screen"), this);
    fullScreenAction->setToolTip(tr("Show QRCode fullscreen"));
    actionBar->addButton(copyAction);
    actionBar->addButton(copyURIAction);
    actionBar->addButton(clearAction);
    actionBar->addButton(saveAction);
    actionBar->addButton(fullScreenAction);
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(on_btnCopyAddress_clicked()));
    connect(copyURIAction, SIGNAL(triggered()), this, SLOT(on_btnCopyURI_clicked()));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearAll()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(on_btnSaveAs_clicked()));
    connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(showFullScreen()));

    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
    screenOrientationChanged();
#endif

    QCoreApplication::instance()->processEvents();

    connect(ui->requestBy, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRequestFields()));
    connect(ui->btnOpenSettings, SIGNAL(clicked()), this, SLOT(optionsClicked()));

#ifndef USE_FULLSCREEN
    genCode();
#endif
}


RequestPaymentDialog::~RequestPaymentDialog()
{
    delete ui;
}

void RequestPaymentDialog::screenOrientationChanged()
{
#ifdef USE_FULLSCREEN
    setFixedSize(screen->virtualSize());
    ui->outUri->sizeHint();
    genCode();
#endif

}


void RequestPaymentDialog::setModel(OptionsModel *model)
{
    this->model = model;

    if (model)
        connect(model, SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
    updateRequestFields();
    //ui->requestSendTo->setText("tsviho@gmail.com");
    //ui->requestSubject->setText("test");

}
void RequestPaymentDialog::clearAll(){
    ui->outUri->clear(); ui->lnMessage->clear(); ui->lnLabel->clear();  ui->lnReqAmount->clear();
    ui->requestSendTo->clear(); ui->requestCopyTo->clear();
    ui->requestSubject->clear(); ui->requestIncludeMessage->clear();
    ui->sendingLineAddress->clear(); ui->sendingLineAmount->clear();
    ui->sendingLineTo->clear(); ui->sendingLineSubject->clear();

    genCode();
}

void RequestPaymentDialog::updateRequestFields()
{

    // detect available send-requests methods and rebuild form
    ui->chkIncludeMessage->setChecked(false);
    ui->requestSendTo->clear(); ui->requestSubject->clear(); ui->requestIncludeMessage->clear();
    ui->sendingLineAddress->clear(); ui->sendingLineAmount->clear();
    ui->sendingLineTo->clear(); ui->sendingLineSubject->clear();
    int index =ui->requestBy->currentIndex();
    // if(this->model->getEaccountEmail()&&this->model->getEaccountV()){ui->requestBy->setCurrentIndex(0)}
    switch(index){
    case(0):
        if(this->model->getEaccountEmail()){
            ui->btnOpenSettings->hide();
            ui->requestSendTo->setVisible(true); ui->requestCopyTo->show(); ui->requestSubject->setVisible(true);
            ui->requestIncludeMessage->setVisible(false);
            ui->chkIncludeMessage->setVisible(true); ui->labelSendTo->setVisible(true); ui->labelSubject->setVisible(true);
            ui->requestSendTo->setInputMethodHints(Qt::ImhEmailCharactersOnly);
            ui->btnSendRequest->setVisible(true); ui->btnPreviewRequest->setVisible(true);
            ui->btnRequestCopyTo->show(); ui->btnRequestSendTo->show(); ui->labelCopyTo->show();
        }else{
            ui->btnOpenSettings->setVisible(true);
            ui->requestSendTo->setVisible(false); ui->requestCopyTo->hide();
            ui->requestSubject->setVisible(false); ui->requestIncludeMessage->setVisible(false);
            ui->chkIncludeMessage->setVisible(false); ui->labelSendTo->setVisible(false); ui->labelSubject->setVisible(false);
            ui->btnSendRequest->setVisible(false); ui->btnPreviewRequest->setVisible(false);
            ui->btnPasteMessage->hide(); ui->btnRequestCopyTo->hide(); ui->btnRequestSendTo->hide(); ui->labelCopyTo->hide();
        }
    break;
    case(1):
        ui->btnOpenSettings->setVisible(true);
        ui->requestSendTo->setVisible(false); ui->requestCopyTo->setVisible(false); ui->requestSubject->setVisible(false);
        ui->requestIncludeMessage->setVisible(false);
        ui->chkIncludeMessage->setVisible(false); ui->labelSendTo->setVisible(false); ui->labelSubject->setVisible(false);
        ui->btnSendRequest->setVisible(false); ui->btnPreviewRequest->setVisible(false);
        ui->btnPasteMessage->hide(); ui->btnRequestCopyTo->hide(); ui->btnRequestSendTo->hide(); ui->labelCopyTo->hide();
    break;
    default:
        ui->btnOpenSettings->hide();
        ui->requestSendTo->show(); ui->requestSubject->show(); ui->requestIncludeMessage->hide();
        ui->chkIncludeMessage->show(); ui->labelSendTo->show();ui->requestCopyTo->show(); ui->labelSubject->show();
        ui->requestSendTo->setInputMethodHints(Qt::ImhEmailCharactersOnly);
        break;
    }
}

bool RequestPaymentDialog::checkRequestForm()
{
    if(ui->requestSendTo->text().isEmpty()){
        QMessageBox::warning( 0, appTitle , tr( "\"Send to\" field is empty\n\n" )  );
        ui->requestSendTo->setFocus();
            return false;
    }
    if(false == GUIUtil::validateEmailAddress(ui->requestSendTo->text())){
        QMessageBox::warning( 0, appTitle , tr( "Incorrect Email address\n\n" )  );
        ui->requestSendTo->setFocus();
            return false;
    }
    if(!ui->requestCopyTo->text().isEmpty() && false == GUIUtil::validateEmailAddress(ui->requestCopyTo->text())){
        QMessageBox::warning( 0, appTitle , tr( "\"Copy To\": Incorrect Email address\n\n" )  );
        ui->requestCopyTo->setFocus();
            return false;
    }
    if(ui->requestSubject->text().isEmpty()){
        QMessageBox::warning( 0, appTitle , tr( "\"Subject\" field is empty\n\n" )  );
        ui->requestSubject->setFocus();
            return false;
    }
    if(ui->chkIncludeMessage->isChecked() && ui->requestIncludeMessage->toPlainText().isEmpty()){
        QMessageBox::warning( 0, appTitle , tr( "You forgot to write message.\n\n" )  );
        ui->requestIncludeMessage->setFocus();
        return false;
    }
    return true;
}

void RequestPaymentDialog::mailSent(const QString &text)
{
    ui->processMessages->setHtml(QString("<p align=\"center\">%1</p>").arg(text));
    movie->stop();
    QPixmap pix(":/icons/request_finished");
    ui->labelLoader->setPixmap(pix.scaled(100, 100));
    ui->btnCancelRequest->hide();
    ui->btnSendMore->show();
    ui->titleSendingRequest->setText(tr("Payment Request sent."));

}
void RequestPaymentDialog::updateMessageBox(const QString &text){
    ui->processMessages->setHtml(QString("<p align=\"center\">%1</p>").arg(text));
}

void RequestPaymentDialog::on_btnSendMore_clicked(){
    clearAll();
    ui->btnSendRequest->show(); ui->btnPreviewRequest->show();
    ui->frameSendForm->show();  ui->frameProcessForm->hide();
    ui->btnClear->show();
}
void RequestPaymentDialog::on_btnClear_clicked(){
    clearAll();
}

/*
Payment information
URI: bitcoin:164za25Ea1fxHmpKiGztePyTTc2nsAqiHB?label=test%20new&message=some%20message
Address: 164za25Ea1fxHmpKiGztePyTTc2nsAqiHB
Label: test new
Message: some message
 */

void RequestPaymentDialog::on_btnSendRequest_clicked()
{
    if(!checkRequestForm())
        return;
    ui->labelLoader->show();
    movie = new QMovie(this);
    movie->setCacheMode(QMovie::CacheAll);
    movie->setFileName(":/movies/loader_paymentrequest");
    //movie->setFileName("qrc:/movies/loader_paymentrequest");
    ui->labelLoader->setMovie(movie);
    movie->start();
    ui->btnCancelRequest->show();
    ui->btnSendMore->hide();
    ui->sendingLineAmount->setText(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, ui->lnReqAmount->value()));
    ui->sendingLineAddress->setText(address);
    ui->sendingLineTo->setText(ui->requestSendTo->text());
    ui->sendingLineSubject->setText(ui->requestSubject->text());
    ui->sendingLineAddress->setCursorPosition(0);
    ui->sendingLineSubject->setCursorPosition(0);
    ui->sendingLineTo->setCursorPosition(0);

    ui->btnClear->hide();
    ui->frameSendForm->hide();  ui->frameProcessForm->show();
    ui->btnSendRequest->hide(); ui->btnPreviewRequest->hide();
    //ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    ui->processMessages->clear();
    smtp = new Smtp(this->model->getEaccountEmailUser(), this->model->getEaccountEmailPwd(),
                    this->model->getEaccountEmailSmtp(), this->model->getEaccountEmailPort());
    smtp->setModel(model);
    smtp->setAppTitle(appTitle);
    ui->titleSendingRequest->setText(tr("Sending Payment Request..."));

#ifdef USE_QRCODE

   // create tmp file

    QString myDir;
#if QT_VERSION < 0x050000
        myDir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#else
        myDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#endif
    QFile file(myDir + "/qrcode.png");
    if(file.exists()){  file.remove(); }
    file.open(QIODevice::WriteOnly);
    QPixmap pixmap = QPixmap::fromImage(myImage).scaled(300, 300);
    pixmap.save(&file, "PNG");
    file.close();
#endif


    connect(smtp, SIGNAL(status(QString)), this, SLOT(updateMessageBox(QString)));
    connect(smtp, SIGNAL(finished(QString)), this, SLOT(mailSent(QString)));
    connect(smtp, SIGNAL(error(QString)), this, SLOT(mailSent(QString)));
    connect(smtp, SIGNAL(finished(QString)), movie, SLOT(stop()));
    connect(smtp, SIGNAL(error(QString)), movie, SLOT(stop()));

    if(!ui->requestCopyTo->text().isEmpty())
        smtp->setCC(ui->requestSendTo->text());

#ifdef USE_QRCODE

    smtp->sendMail(this->model->getEaccountEmailFrom(), ui->requestSendTo->text() ,
                   ui->requestSubject->text(), createRequestPaymentMessage(),
                   file  );

#else
    smtp->sendMail(this->model->getEaccountEmailFrom(), ui->requestSendTo->text() , ui->requestSubject->text(),createRequestPaymentMessage());
#endif

}

QString RequestPaymentDialog::createRequestPaymentMessage()
{
    return QString(ui->requestIncludeMessage->toHtml() +
                   "\n<hr />\n<h3>"+tr("Payment information")+"</h3>\n"
                   "<b>"+tr("Amount")+":</b> %1<br />\n<b>"+tr("Address")+":</b> %2<br />\n"
                   "<b>URI:</b> <a href=\"%3\">%4</a><br />\n"
                  ).arg(BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, ui->lnReqAmount->value()))
                   .arg(address).arg(QString(addressURI).replace("artboomerang:", "artboomerang://"))
            .arg(addressURI);
}

void RequestPaymentDialog::on_btnCancelRequest_clicked()
{
    smtp->stop();
    ui->btnSendRequest->show(); ui->btnPreviewRequest->show();
    ui->frameProcessForm->hide(); ui->frameSendForm->show();

}

void RequestPaymentDialog::on_btnPreviewRequest_clicked()
{
    if(!checkRequestForm())
        return;
    if(!requestPreview){
    }
     requestPreview = new QDialog(this, Qt::Popup);
     requestPreview->setObjectName("requestPreview");
     requestPreview->setGeometry(frameGeometry());
     requestPreview->setContextMenuPolicy(Qt::NoContextMenu);
     requestPreview->setWindowTitle("Payment Request Preview");
#ifdef USE_FULLSCREEN
     requestPreview->setWindowState(requestPreview->windowState() ^ Qt::WindowMaximized);
#endif
     QVBoxLayout *vlayout = new QVBoxLayout(requestPreview);
     vlayout->setContentsMargins(9,9,9,9);
     vlayout->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
     QHBoxLayout *top = new QHBoxLayout();
     QLabel *im = new QLabel(); im->setObjectName("requestPreviewImage");
     im->setPixmap(QPixmap::fromImage(myImage).scaled(100, 100));
     QFormLayout *topR = new QFormLayout();
     top->addWidget(im);
     top->addLayout(topR);
     vlayout->addLayout(top);

     QLineEdit *to = new QLineEdit(ui->requestSendTo->text()); to->setStyleSheet(" *{font-weight: bold; background:transparent;}"); to->setReadOnly(true);
     topR->addRow(tr("To:"), to);
     if(!ui->requestCopyTo->text().isEmpty()){
         QLineEdit *cc = new QLineEdit(ui->requestCopyTo->text()); cc->setStyleSheet(" *{font-weight: bold; background:transparent;}"); cc->setReadOnly(true);
         topR->addRow(tr("Cc:"), cc);
     }
     QLineEdit *s = new QLineEdit(ui->requestSubject->text()); s->setStyleSheet(" *{font-weight: bold;background:transparent;}"); s->setReadOnly(true);
     topR->addRow(tr("Subject"), s);
     QLineEdit *n = new QLineEdit(tr("Image will be attached")); n->setStyleSheet(" *{font-weight: bold;background:transparent;}"); n->setReadOnly(true);
     topR->addRow("QRCode:", n);
     to->setCursorPosition(0); s->setCursorPosition(0);


    QTextEdit *t = new QTextEdit(); t->setHtml(createRequestPaymentMessage()); t->setStyleSheet("*{background:transparent;}"); t->setReadOnly(true);
    vlayout->addWidget(t);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addStretch();
    QPushButton *b = new QPushButton(); b->setText(tr("<< Back"));
    b->setMinimumWidth(this->width()/1.01);
    hlayout->addWidget(b);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);
    connect(b, SIGNAL(clicked()), requestPreview, SLOT(close()));

    requestPreview->show();
}

void RequestPaymentDialog::closeFullScreen()
{
#ifdef USE_FULLSCREEN
    if(fullScreenQRCode){
        fullScreenQRCode->removeEventFilter(this);
        fullScreenQRCode->close();
        delete fullScreenQRCode;
    }
#endif
}
void RequestPaymentDialog::showFullScreen()
{

#ifdef USE_FULLSCREEN
    fullScreenQRCode = new QWidget(this, Qt::Popup);
    fullScreenQRCode->setObjectName("fullScreenQRCode");
    fullScreenQRCode->setWindowState(fullScreenQRCode->windowState() ^ Qt::WindowMaximized);
    fullScreenQRCode->setStyleSheet("#fullScreenQRCode {background:#000000; opacity: 0.3;}");
    //fullScreenQRCode->setWindowOpacity(0.3); // not supported on several platforms
    QGridLayout *vlayout = new QGridLayout(fullScreenQRCode);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    QLabel *l = new QLabel(fullScreenQRCode);
    l->setObjectName("fullScreenQRCodeImage");
    int em=fontMetrics().height();
    int w = screen->virtualSize().width()-em;
    int h = screen->virtualSize().height()-em;
    int iw = w<h?w:h;
    l->setPixmap(QPixmap::fromImage(myImage).scaled(iw, iw));
    vlayout->addWidget(l);
    fullScreenQRCode->installEventFilter(this);
    fullScreenQRCode->show();
#endif

}


bool RequestPaymentDialog::eventFilter(QObject *object, QEvent *event)
{
#ifdef USE_FULLSCREEN
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (object == ui->lblQRCode)
        {
                showFullScreen();
                return true;
        }
        if (object->objectName() == "fullScreenQRCodeImage" || object->objectName() == "fullScreenQRCode")
        {
                closeFullScreen();
                return true;
        }
    }
#endif

    return QDialog::eventFilter(object, event);
}

void RequestPaymentDialog::genCode()
{
    QString uri = getURI();
    if (uri != "")
    {
        ui->lblQRCode->setText("");

        QRcode *code = QRcode_encodeString(uri.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (!code)
        {
            ui->lblQRCode->setText(tr("Error encoding URI into QR Code."));
            return;
        }
        myImage = QImage(code->width + 8, code->width + 8, QImage::Format_RGB32);
        myImage.fill(0xffffff);
        unsigned char *p = code->data;
        for (int y = 0; y < code->width; y++)
        {
            for (int x = 0; x < code->width; x++)
            {
                myImage.setPixel(x + 4, y + 4, ((*p & 1) ? 0x0 : 0xffffff));
                p++;
            }
        }
        QRcode_free(code);

        ui->lblQRCode->resize(100, 100);
        ui->lblQRCode->setPixmap(QPixmap::fromImage(myImage).scaled(100, 100));

        ui->outUri->setPlainText(uri);

    }
}

QString RequestPaymentDialog::getURI()
{
    QString ret = QString("artboomerang:%1").arg(address);
    int paramCount = 0;

    ui->outUri->clear();
    addressURI.clear();

    if (ui->lnReqAmount->validate())
    {
            // even if we allow a non BTC unit input in lnReqAmount, we generate the URI with BTC as unit (as defined in BIP21)
            ret += QString("?amount=%1").arg(BitcoinUnits::format(BitcoinUnits::BTC, ui->lnReqAmount->value()));
            paramCount++;
    }


    if (!ui->lnLabel->text().isEmpty())
    {
        QString lbl(QUrl::toPercentEncoding(ui->lnLabel->text()));
        ret += QString("%1label=%2").arg(paramCount == 0 ? "?" : "&").arg(lbl);
        paramCount++;
    }

    if (!ui->lnMessage->text().isEmpty())
    {
        QString msg(QUrl::toPercentEncoding(ui->lnMessage->text()));
        ret += QString("%1message=%2").arg(paramCount == 0 ? "?" : "&").arg(msg);
        paramCount++;
    }

    // limit URI length to prevent a DoS against the QR-Code dialog
    if (ret.length() > MAX_URI_LENGTH)
    {
        ui->btnSaveAs->setEnabled(false);
        ui->lblQRCode->setText(tr("Resulting URI too long, try to reduce the text for label / message."));
        return QString("");
    }
    this->addressURI = ret;

    ui->btnCopyAddress->setEnabled(true);
    ui->btnCopyURI->setEnabled(true);
    ui->btnSaveAs->setEnabled(true);
    return ret;
}

void RequestPaymentDialog::on_lnReqAmount_textChanged()
{
    genCode();
}

void RequestPaymentDialog::on_lnLabel_textChanged()
{
    genCode();
}

void RequestPaymentDialog::on_lnMessage_textChanged()
{
    genCode();
}

void RequestPaymentDialog::on_btnSaveAs_clicked()
{
    QString fn = GUIUtil::getSaveFileName(this, screen, tr("Save QR Code"), QString(), tr("PNG Images (*.png)"));
    if (!fn.isEmpty())
        myImage.scaled(EXPORT_IMAGE_SIZE, EXPORT_IMAGE_SIZE).save(fn);
}


void RequestPaymentDialog::on_btnCopyURI_clicked()
{
    //error: Data set on unsupported clipboard mode. QMimeData object will be deleted.
    GUIUtil::setClipboard(addressURI);
    QMessageBox::information(this, tr("Copy URI"),
        tr("Generated URI copied to clipboard."),
        QMessageBox::Ok);
}

void RequestPaymentDialog::on_btnCopyAddress_clicked()
{
    GUIUtil::setClipboard(address);
    QMessageBox::information(this, tr("Copy Address"),
        tr("Address: \"%1\"\ncopied to clipboard.")
           .arg(address.length()>35?address.left(35)+"...":address),
        QMessageBox::Ok);

}

void RequestPaymentDialog::on_btnRequestSendTo_clicked(){
    QString t = QApplication::clipboard()->text();
    if(!t.isEmpty())
        ui->requestSendTo->setText(t.left(61));
}
void RequestPaymentDialog::on_btnRequestCopyTo_clicked(){
    QString t = QApplication::clipboard()->text();
    if(!t.isEmpty())
        ui->requestCopyTo->setText(t.left(61));
}
void RequestPaymentDialog::on_btnPasteMessage_clicked(){
    QString t = QApplication::clipboard()->text();
    if(!t.isEmpty())
        ui->requestIncludeMessage->setText(t);
}

void RequestPaymentDialog::on_btnCancel_clicked()
{
    //smtp->close();
    clearAll();
    RequestPaymentDialog::close();
}

void RequestPaymentDialog::on_chkIncludeMessage_toggled(bool fChecked)
{
    ui->requestIncludeMessage->clear();
    if (!fChecked){ // if chkIncludeMessage is not active, don't display additional message field
       ui->requestIncludeMessage->hide(); ui->btnPasteMessage->hide();
    }else{
        ui->requestIncludeMessage->show();  ui->btnPasteMessage->show();
        ui->requestIncludeMessage->setFocus();
#ifdef USE_FULLSCREEN
        qApp->inputMethod()->show();
#endif
    }
}

void RequestPaymentDialog::updateDisplayUnit()
{
    if (model)
    {
        // Update lnReqAmount with the current unit
        ui->lnReqAmount->setDisplayUnit(model->getDisplayUnit());
    }
}

void RequestPaymentDialog::optionsClicked()
{
    if(model){
        OptionsDialog dlg(this,screen);
        dlg.setModel(model);
#ifdef USE_FULLSCREEN
        dlg.setCurrentTab(3);
#else
        dlg.setCurrentTab(4);
#endif
        dlg.exec();
    }
}

void RequestPaymentDialog::keyPressEvent(QKeyEvent *event)
{
#ifdef USE_FULLSCREEN
    if(event->key() == Qt::Key_Back)
    {
        close();
    }
#else
    if(event->key() == Qt::Key_Escape)
    {
        close();
    }
#endif

}
