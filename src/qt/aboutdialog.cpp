// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "dialogwindowflags.h"
#include "clientmodel.h"
#include "version.h"
#include <QKeyEvent>
#include <QScroller>
#include <QTouchDevice>

AboutDialog::AboutDialog(QWidget *parent, BitScreen* screen) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    const std::string urlMain("http://www.artboomerang.win");
    const std::string urlExplorer("http://www.artboomerang.win");
    ui->setupUi(this);
    this->setWindowTitle("About ArtBoomerang");
    ui->scrollAreaAboutDlg->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(ui->scrollAreaAboutDlg->viewport(), QScroller::TouchGesture);
    QScroller::grabGesture(this->ui->scrollAreaAboutDlg->viewport(), QScroller::LeftMouseButtonGesture);
    ui->scrollAreaAboutDlg->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef USE_FULLSCREEN
        ui->pushButton->setIconSize(screen->iconSize());
        ActionBar *actionBar =new ActionBar(this);
        QAction *closeAction = new QAction(QIcon(":/android_icons/action_close"), tr("&Close"), this);
        connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
        actionBar->addButton(closeAction);
        ui->actionBarLayout->addWidget(actionBar);
        actionBar->setTitle(windowTitle(), false);
        setWindowState(this->windowState() ^ Qt::WindowMaximized);
#endif
    //transparent window with bg-image
    //setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    //setParent(0); // Create TopLevel-Widget
    //setAttribute(Qt::WA_NoSystemBackground, true);
    //setAttribute(Qt::WA_TranslucentBackground, true);
    //setAttribute(Qt::WA_PaintOnScreen);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(close()));
    //std::replace( s.begin(), s.end(), 'x', 'y');
    QString cpTxt = ui->copyrightLabel->text();
    cpTxt.replace(QString("[urlMain]"), QString::fromStdString(urlMain))
            .replace(QString("[urlExplorer]"), QString::fromStdString(urlExplorer));
    ui->copyrightLabel->setText(cpTxt);
}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_pushButton_clicked()
{
    close();
}
void AboutDialog::keyPressEvent(QKeyEvent *event)
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
