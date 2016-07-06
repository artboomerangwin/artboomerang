// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "qrcodedialog.h"
#include "ui_qrcodedialog.h"

#include "bitcoinunits.h"
#include "guiconstants.h"

#include <QPixmap>
#include <QUrl>
#include <QScroller>
#include <QScrollBar>


#include <qrencode.h>

QRCodeDialog::QRCodeDialog(const QString &addr, const QString &label, bool enableReq, QWidget *parent, BitScreen *screen) :
    QDialog(parent),
    ui(new Ui::QRCodeDialog),
    model(0),
    address(addr),
    screen(screen)
{
    ui->setupUi(this);
    qrImageMinWidth = 300;
    qrImageWidth = 300;//min (default) QR-code width in pixels.

    setWindowTitle(QString(tr("QR-code for address: %1...")).arg(address.left(10)));

    ui->scrollArea->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(this->ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    ui->scrollAreaWidgetContents->resize(ui->scrollAreaWidgetContents->sizeHint());
    ui->scrollArea->horizontalScrollBar()->hide();
    ui->scrollArea->horizontalScrollBar()->resize(0, 0);
    int em=fontMetrics().height();
    ui->outUri->setMinimumHeight(2.5*em);
    ui->outUri->setMaximumHeight(5*em);
    //ui->outUri->setFixedWidth(300);
    ui->outUri->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);//Fixed


    ui->chkReqPayment->setVisible(enableReq);
    ui->lblAmount->setVisible(enableReq);
    ui->lnReqAmount->setVisible(enableReq);

    ui->lnLabel->setText(label);

    ui->btnCopyAddress->setEnabled(false);
    ui->btnCopyURI->setEnabled(false);
    ui->btnSaveAs->setEnabled(false);

#ifdef USE_FULLSCREEN
    ui->btnClose->hide();
    ui->btnCopyAddress->hide();
    ui->btnSaveAs->hide();
    actionBar =new ActionBar(this);
    ui->actionBarLayout->addWidget(actionBar);
    QAction *copyAction = new QAction(QIcon(":/android_icons/action_copy"), tr("&Copy Address"), this);
    copyAction->setToolTip(tr("Copy Address to clipboard"));
    QAction *clearAction = new QAction(QIcon(":/android_icons/action_undo"), tr("Undo"), this);
    clearAction->setToolTip(tr("Undo Changes"));
    QAction *saveAction = new QAction(QIcon(":/android_icons/action_save"), tr("&Save As..."), this);
    saveAction->setToolTip(tr("Save QRCode image to a file"));
    QAction *fullScreenAction = new QAction(QIcon(":/android_icons/action_full_screen"), tr("&Full Screen"), this);
    fullScreenAction->setToolTip(tr("Show QRCode fullscreen"));
    //ui->lblQRCode->installEventFilter(this);
    ui->lblQRCode->installEventFilter(this);
    actionBar->addButton(copyAction);
    actionBar->addButton(clearAction);
    actionBar->addButton(saveAction);
    actionBar->addButton(fullScreenAction);
    connect(actionBar, SIGNAL(up()), this, SLOT(close()));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(on_btnCopyAddress_clicked()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(on_btnSaveAs_clicked()));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearAll()));
    connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(showFullScreen()));
    //connect(ui->lblQRCode, SIGNAL(mousePressEvent()), this, SLOT(showFullScreen()));

    connect(this->screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged()));
    screenOrientationChanged();
    //setWindowState(this->windowState() ^ Qt::WindowMaximized);
#endif

    QCoreApplication::instance()->processEvents();

    /*
Payment information
URI: bitcoin:164za25Ea1fxHmpKiGztePyTTc2nsAqiHB?label=test%20new&message=some%20message
Address: 164za25Ea1fxHmpKiGztePyTTc2nsAqiHB
Label: test new
Message: some message
     */
    genCode();
}

QRCodeDialog::~QRCodeDialog()
{
    delete ui;
}

void QRCodeDialog::clearAll(){
    ui->outUri->clear();
    ui->lnMessage->clear();
    ui->lnLabel->clear();
    ui->lnReqAmount->clear();
    genCode();
}

void QRCodeDialog::screenOrientationChanged()
{
#ifdef USE_FULLSCREEN
    setFixedSize(screen->virtualSize());
    ui->outUri->sizeHint();
    int em=fontMetrics().height();
    qrImageWidth = screen->virtualSize().width()-em;
    if(screen->virtualSize().height()<screen->virtualSize().width()){
        qrImageWidth = screen->virtualSize().height()-em;
    }
    if(screen->isPortrait()){
        actionBar->setTitle(tr("QR-code"), true);
        ui->lblQRCode->setFixedSize(qrImageWidth,qrImageWidth);
    }else{
        actionBar->setTitle(windowTitle(), true);
        ui->lblQRCode->setFixedSize(qrImageMinWidth,qrImageMinWidth);
    }
    genCode();
#endif

}


void QRCodeDialog::setModel(OptionsModel *model)
{
    this->model = model;

    if (model)
        connect(model, SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void QRCodeDialog::closeFullScreen()
{
#ifdef USE_FULLSCREEN
    if(fullScreenQRCode){
        fullScreenQRCode->removeEventFilter(this);
        fullScreenQRCode->close();
    }
#endif
}

void QRCodeDialog::showFullScreen()
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
    l->setPixmap(QPixmap::fromImage(myImage).scaled(qrImageWidth, qrImageWidth));
    vlayout->addWidget(l);
    fullScreenQRCode->installEventFilter(this);
    fullScreenQRCode->show();
#endif
}

bool QRCodeDialog::eventFilter(QObject *object, QEvent *event)
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
void QRCodeDialog::genCode()
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

#ifdef USE_FULLSCREEN
    if(screen->isPortrait()){
        ui->lblQRCode->resize(qrImageWidth, qrImageWidth);
        ui->lblQRCode->setPixmap(QPixmap::fromImage(myImage).scaled(qrImageWidth, qrImageWidth));
    }else{
        ui->lblQRCode->resize(qrImageMinWidth, qrImageMinWidth);
        ui->lblQRCode->setPixmap(QPixmap::fromImage(myImage).scaled(qrImageMinWidth, qrImageMinWidth));
    }
#else
    ui->lblQRCode->resize(qrImageMinWidth, qrImageMinWidth);
    ui->lblQRCode->setPixmap(QPixmap::fromImage(myImage).scaled(qrImageMinWidth, qrImageMinWidth));
#endif

        ui->outUri->setPlainText(uri);

    }
}

QString QRCodeDialog::getURI()
{
    QString ret = QString("artboomerang:%1").arg(address);
    int paramCount = 0;

    ui->outUri->clear();
    addressURI.clear();

    if (ui->chkReqPayment->isChecked())
    {
        if (ui->lnReqAmount->validate())
        {
            // even if we allow a non BTC unit input in lnReqAmount, we generate the URI with BTC as unit (as defined in BIP21)
            ret += QString("?amount=%1").arg(BitcoinUnits::format(BitcoinUnits::BTC, ui->lnReqAmount->value()));
            paramCount++;
        }
        //else
        //{
        //    ui->btnSaveAs->setEnabled(false);
        //    ui->lblQRCode->setText(tr("The entered amount is invalid, please check."));
        //    return QString("");
        //}
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

void QRCodeDialog::on_lnReqAmount_textChanged()
{
    genCode();
}

void QRCodeDialog::on_lnLabel_textChanged()
{
    genCode();
}

void QRCodeDialog::on_lnMessage_textChanged()
{
    genCode();
}

void QRCodeDialog::on_btnSaveAs_clicked()
{
    QString fn = GUIUtil::getSaveFileName(this, screen, tr("Save QR Code"), QString(), tr("PNG Images (*.png)"));
    if (!fn.isEmpty())
        myImage.scaled(EXPORT_IMAGE_SIZE, EXPORT_IMAGE_SIZE).save(fn);
}


void QRCodeDialog::on_btnCopyURI_clicked()
{
    //error: Data set on unsupported clipboard mode. QMimeData object will be deleted.
    GUIUtil::setClipboard(addressURI);
    QMessageBox::information(this, tr("Copy URI"),
        tr("Generated URI copied to clipboard."),
        QMessageBox::Ok);
}

void QRCodeDialog::on_btnCopyAddress_clicked()
{
    GUIUtil::setClipboard(address);
    QMessageBox::information(this, tr("Copy Address"),
        tr("Address: \"%1\"\ncopied to clipboard.")
           .arg(address.length()>35?address.left(35)+"...":address),
        QMessageBox::Ok);

}
void QRCodeDialog::on_btnClose_clicked()
{
     QRCodeDialog::close();
}
void QRCodeDialog::on_chkReqPayment_toggled(bool fChecked)
{
    //if (!fChecked)
        // if chkReqPayment is not active, don't display lnReqAmount as invalid
        ui->lnReqAmount->setValid(true);

    genCode();
}

void QRCodeDialog::updateDisplayUnit()
{
    if (model)
    {
        // Update lnReqAmount with the current unit
        ui->lnReqAmount->setDisplayUnit(model->getDisplayUnit());
    }
}
