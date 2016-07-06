#include "guiutil.h"
#include "bitcoinaddressvalidator.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "util.h"
#include "init.h"

#include <QString>
#include <QDateTime>
#include <QDoubleValidator>
#include <QFont>
#include <QLineEdit>
#if QT_VERSION < 0x050000
#include <QUrl>
#else
#include <QUrlQuery>
#endif
#include <QTextDocument> // For Qt::escape
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDesktopWidget>

#include <QThread>
#include <QSettings>
#include <QScreen>
#include <QWindow>
#include <QWidget>
#include <QTextDocument> // for Qt::mightBeRichText

#include <QDebug>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "shlwapi.h"
#include "shlobj.h"
#include "shellapi.h"
#endif

namespace GUIUtil {

QString dateTimeStr(const QDateTime &date)
{
    return date.date().toString(Qt::SystemLocaleShortDate) + QString(" ") + date.toString("hh:mm");
}

QString dateTimeStr(qint64 nTime)
{
    return dateTimeStr(QDateTime::fromTime_t((qint32)nTime));
}

QFont bitcoinAddressFont()
{
    QFont font("Monospace");
#if QT_VERSION >= 0x040800
    font.setStyleHint(QFont::Monospace);
#else
    font.setStyleHint(QFont::TypeWriter);
#endif
    return font;
}

void setupAddressWidget(QLineEdit *widget, QWidget *parent)
{
    widget->setMaxLength(BitcoinAddressValidator::MaxAddressLength);
    widget->setValidator(new BitcoinAddressValidator(parent));
    widget->setFont(bitcoinAddressFont());

}

void setupAmountWidget(QLineEdit *widget, QWidget *parent)
{
    QDoubleValidator *amountValidator = new QDoubleValidator(parent);
    amountValidator->setDecimals(8);
    amountValidator->setBottom(0.0);
    widget->setValidator(amountValidator);
    widget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

bool parseBitcoinURI(const QUrl &uri, SendCoinsRecipient *out)
{
    // FriendshipCoin: check prefix
    if(uri.scheme() != QString("artboomerang"))
        return false;

    SendCoinsRecipient rv;
    rv.address = uri.path();
    rv.amount = 0;
#if QT_VERSION < 0x050000
    QList<QPair<QString, QString> > items = uri.queryItems();
#else
    QUrlQuery uriQuery(uri);
    QList<QPair<QString, QString> > items = uriQuery.queryItems();
#endif
    for (QList<QPair<QString, QString> >::iterator i = items.begin(); i != items.end(); i++)
    {
        bool fShouldReturnFalse = false;
        if (i->first.startsWith("req-"))
        {
            i->first.remove(0, 4);
            fShouldReturnFalse = true;
        }

        if (i->first == "label")
        {
            rv.label = i->second;
            fShouldReturnFalse = false;
        }
        else if (i->first == "amount")
        {
            if(!i->second.isEmpty())
            {
                if(!BitcoinUnits::parse(BitcoinUnits::BTC, i->second, &rv.amount))
                {
                    return false;
                }
            }
            fShouldReturnFalse = false;
        }

        if (fShouldReturnFalse)
            return false;
    }
    if(out)
    {
        *out = rv;
    }
    return true;
}

bool parseBitcoinURI(QString uri, SendCoinsRecipient *out)
{
    // Convert artboomerang:// to artboomerang:
    //
    //    Cannot handle this later, because bitcoin:// will cause Qt to see the part after // as host,
    //    which will lower-case it (and thus invalidate the address).
    if(uri.startsWith("artboomerang://"))
    {
        uri.replace(0, 12, "artboomerang:");
    }
    QUrl uriInstance(uri);
    return parseBitcoinURI(uriInstance, out);
}


bool validateEmailAddress (const QString& email, QString match){
    bool valid;
    QRegExp rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,8}\\b");
    //QRegExp rx("\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,8}\b");
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPatternSyntax(QRegExp::RegExp);
    valid = rx.exactMatch(email);
    if(valid && !match.isEmpty()){
        valid = email.contains(match);
    }
    return valid;

}

QString HtmlEscape(const QString& str, bool fMultiLine)
{
#if QT_VERSION < 0x050000
    QString escaped = Qt::escape(str);
#else
    QString escaped = str.toHtmlEscaped();
#endif
    if(fMultiLine)
    {
        escaped = escaped.replace("\n", "<br>\n");
    }
    return escaped;
}

QString HtmlEscape(const std::string& str, bool fMultiLine)
{
    return HtmlEscape(QString::fromStdString(str), fMultiLine);
}

void copyEntryData(QAbstractItemView *view, int column, int role)
{
    QApplication::clipboard()->clear();
    if(!view || !view->selectionModel())
        return;
    QModelIndexList selection = view->selectionModel()->selectedRows(column);

    if(!selection.isEmpty())
    {
        // Copy first item
        QApplication::clipboard()->setText(selection.at(0).data(role).toString());
    }
}

QString getSaveFileName(QWidget *parent, BitScreen *screen, const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 QString *selectedSuffixOut)
{
    QString result;
    QString myDir;
    if(dir.isEmpty()) // Default to user documents location
    {
#if QT_VERSION < 0x050000
        myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
        myDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#endif
    }
    else
    {
        myDir = dir;
    }
    /* Extract first suffix from filter pattern "Description (*.foo)" or "Description (*.foo *.bar ...) */
    QRegExp filter_re(".* \\(\\*\\.(.*)[ \\)]");
    QString selectedSuffix;
    if(filter_re.exactMatch(filter))
    {
        selectedSuffix = filter_re.cap(1);
    }
    /* Return selected suffix if asked to */
    if(selectedSuffixOut)
    {
        *selectedSuffixOut = selectedSuffix;
    }

#ifdef USE_FULLSCREEN
    QFileDialog *fd = new QFileDialog(parent, caption, myDir, filter);
    // change dialog size
    fd->setFixedSize(screen->virtualSize());
    if(fd->exec())
    {
        QStringList list = fd->selectedFiles();
        QString result = list.first();

        /* Add suffix if needed */
        QFileInfo info(result);
        if(!result.isEmpty())
        {
            if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
            {
                /* No suffix specified, add selected suffix */
                if(!result.endsWith("."))
                    result.append(".");
                result.append(selectedSuffix);
            }
        }
        QFile file(result);
        if(file.exists()){
          QMessageBox::StandardButton retval=
                  QMessageBox::question(fd, caption, "File exists. Do you want to replace it?",
                   QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Cancel);
          if(retval == QMessageBox::Yes){
              file.remove();
          }
          else{

          }
        }
        if (!file.open(QIODevice::WriteOnly)) {
          QMessageBox::information(fd, "Error Saving File", QString("The file could not be opened for writing.\n%1").arg(result));
        }
        file.close();
    }

#else

    result = QFileDialog::getSaveFileName(parent, caption, myDir, filter);
    /* Add suffix if needed */
    QFileInfo info(result);
    if(!result.isEmpty())
    {
        if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
        {
            /* No suffix specified, add selected suffix */
            if(!result.endsWith("."))
                result.append(".");
            result.append(selectedSuffix);
        }
    }

#endif
    return result;
}

Qt::ConnectionType blockingGUIThreadConnection()
{
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        return Qt::BlockingQueuedConnection;
    }
    else
    {
        return Qt::DirectConnection;
    }
}


bool checkPoint(const QPoint &p, const QWidget *w)
{
    QWidget *atW = qApp->widgetAt(w->mapToGlobal(p));
    if (!atW) return false;
    return atW->topLevelWidget() == w;
}

bool isObscured(QWidget *w)
{
    return !(checkPoint(QPoint(0, 0), w)
        && checkPoint(QPoint(w->width() - 1, 0), w)
        && checkPoint(QPoint(0, w->height() - 1), w)
        && checkPoint(QPoint(w->width() - 1, w->height() - 1), w)
        && checkPoint(QPoint(w->width() / 2, w->height() / 2), w));
}

void openDebugLogfile()
{
    boost::filesystem::path pathDebug = GetDataDir() / "debug.log";

    /* Open debug.log with the associated application */
    if (boost::filesystem::exists(pathDebug))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));
}

ToolTipToRichTextFilter::ToolTipToRichTextFilter(int size_threshold, QObject *parent) :
    QObject(parent), size_threshold(size_threshold)
{

}

bool ToolTipToRichTextFilter::eventFilter(QObject *obj, QEvent *evt)
{
    if(evt->type() == QEvent::ToolTipChange)
    {
        QWidget *widget = static_cast<QWidget*>(obj);
        QString tooltip = widget->toolTip();
        if(tooltip.size() > size_threshold && !tooltip.startsWith("<qt>") && !Qt::mightBeRichText(tooltip))
        {
            // Prefix <qt/> to make sure Qt detects this as rich text
            // Escape the current message as HTML and replace \n by <br>
            tooltip = "<qt>" + HtmlEscape(tooltip, true) + "<qt/>";
            widget->setToolTip(tooltip);
            return true;
        }
    }
    return QObject::eventFilter(obj, evt);
}

#ifdef WIN32
boost::filesystem::path static StartupShortcutPath()
{
    return GetSpecialFolderPath(CSIDL_STARTUP) / "ArtBoomerang.lnk";
}

bool GetStartOnSystemStartup()
{
    // check for Bitcoin.lnk
    return boost::filesystem::exists(StartupShortcutPath());
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    // If the shortcut exists already, remove it for updating
    boost::filesystem::remove(StartupShortcutPath());

    if (fAutoStart)
    {
        CoInitialize(NULL);

        // Get a pointer to the IShellLink interface.
        IShellLink* psl = NULL;
        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                                CLSCTX_INPROC_SERVER, IID_IShellLink,
                                reinterpret_cast<void**>(&psl));

        if (SUCCEEDED(hres))
        {
            // Get the current executable path
            TCHAR pszExePath[MAX_PATH];
            GetModuleFileName(NULL, pszExePath, sizeof(pszExePath));

            TCHAR pszArgs[5] = TEXT("-min");

            // Set the path to the shortcut target
            psl->SetPath(pszExePath);
            PathRemoveFileSpec(pszExePath);
            psl->SetWorkingDirectory(pszExePath);
            psl->SetShowCmd(SW_SHOWMINNOACTIVE);
            psl->SetArguments(pszArgs);

            // Query IShellLink for the IPersistFile interface for
            // saving the shortcut in persistent storage.
            IPersistFile* ppf = NULL;
            hres = psl->QueryInterface(IID_IPersistFile,
                                       reinterpret_cast<void**>(&ppf));
            if (SUCCEEDED(hres))
            {
                WCHAR pwsz[MAX_PATH];
                // Ensure that the string is ANSI.
                MultiByteToWideChar(CP_ACP, 0, StartupShortcutPath().string().c_str(), -1, pwsz, MAX_PATH);
                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(pwsz, TRUE);
                ppf->Release();
                psl->Release();
                CoUninitialize();
                return true;
            }
            psl->Release();
        }
        CoUninitialize();
        return false;
    }
    return true;
}

#elif defined(Q_OS_LINUX)

// Follow the Desktop Application Autostart Spec:
//  http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html

boost::filesystem::path static GetAutostartDir()
{
    namespace fs = boost::filesystem;

    char* pszConfigHome = getenv("XDG_CONFIG_HOME");
    if (pszConfigHome) return fs::path(pszConfigHome) / "autostart";
    char* pszHome = getenv("HOME");
    if (pszHome) return fs::path(pszHome) / ".config" / "autostart";
    return fs::path();
}

boost::filesystem::path static GetAutostartFilePath()
{
    return GetAutostartDir() / "artboomerang.desktop";
}

bool GetStartOnSystemStartup()
{
    boost::filesystem::ifstream optionFile(GetAutostartFilePath());
    if (!optionFile.good())
        return false;
    // Scan through file for "Hidden=true":
    std::string line;
    while (!optionFile.eof())
    {
        getline(optionFile, line);
        if (line.find("Hidden") != std::string::npos &&
            line.find("true") != std::string::npos)
            return false;
    }
    optionFile.close();

    return true;
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    if (!fAutoStart)
        boost::filesystem::remove(GetAutostartFilePath());
    else
    {
        char pszExePath[MAX_PATH+1];
        memset(pszExePath, 0, sizeof(pszExePath));
        if (readlink("/proc/self/exe", pszExePath, sizeof(pszExePath)-1) == -1)
            return false;

        boost::filesystem::create_directories(GetAutostartDir());

        boost::filesystem::ofstream optionFile(GetAutostartFilePath(), std::ios_base::out|std::ios_base::trunc);
        if (!optionFile.good())
            return false;
        // Write a bitcoin.desktop file to the autostart directory:
        optionFile << "[Desktop Entry]\n";
        optionFile << "Type=Application\n";
        optionFile << "Name=ArtBoomerang\n";
        optionFile << "Exec=" << pszExePath << " -min\n";
        optionFile << "Terminal=false\n";
        optionFile << "Hidden=false\n";
        optionFile.close();
    }
    return true;
}
#else

// TODO: OSX startup stuff; see:
// https://developer.apple.com/library/mac/#documentation/MacOSX/Conceptual/BPSystemStartup/Articles/CustomLogin.html

bool GetStartOnSystemStartup() { return false; }
bool SetStartOnSystemStartup(bool fAutoStart) { return false; }

#endif

void saveWindowGeometry(const QString& strSetting, QWidget *parent)
{
    QSettings settings;
    settings.setValue(strSetting + "Pos", parent->pos());
    settings.setValue(strSetting + "Size", parent->size());
}

void restoreWindowGeometry(const QString& strSetting,
                           const QSize& defaultSize, QWidget *parent, BitScreen *scr)
{
//#ifndef USE_FULLSCREEN
    // first move layer position on overview style
//#endif
    if(false == scr->maybeMoveOrResize(parent)){
        QSettings settings;
        QPoint pos = settings.value(strSetting + "Pos").toPoint();
        QSize size = settings.value(strSetting + "Size", defaultSize).toSize();

        if (!pos.x() && !pos.y()) {
            QRect screen = QApplication::desktop()->screenGeometry();
            pos.setX((screen.width() - size.width()) / 2);
            pos.setY((screen.height() - size.height()) / 2);
        }
        parent->resize(size);
        parent->move(pos);
    }
}
void setClipboard(const QString& str)
{
    QApplication::clipboard()->setText(str, QClipboard::Clipboard);
    QApplication::clipboard()->setText(str, QClipboard::Selection);
}

HelpMessageBox::HelpMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    header = tr("ArtBoomerang-Qt") + " " + tr("version") + " " +
        QString::fromStdString(FormatFullVersion()) + "\n\n" +
        tr("Usage:") + "\n" +
        "  artboomerang-qt [" + tr("command-line options") + "]                     " + "\n";

    coreOptions = QString::fromStdString(HelpMessage());

    uiOptions = tr("UI options") + ":\n" +
        "  -lang=<lang>           " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                   " + tr("Start minimized") + "\n" +
        "  -splash                " + tr("Show splash screen on startup (default: 1)") + "\n";

    setWindowTitle(tr("ArtBoomerang-Qt"));
    setTextFormat(Qt::PlainText);
    // setMinimumWidth is ignored for QMessageBox so put in non-breaking spaces to make it wider.
    setText(header + QString(QChar(0x2003)).repeated(50));
    setDetailedText(coreOptions + "\n" + uiOptions);
}

void HelpMessageBox::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions;
    fprintf(stdout, "%s", strUsage.toStdString().c_str());
}

void HelpMessageBox::showOrPrint()
{
#if defined(WIN32)
        // On Windows, show a message box, as there is no stderr/stdout in windowed applications
        exec();
#else
        // On other operating systems, print help text to console
        printToConsole();
#endif
}

} // namespace GUIUtil



BitScreen::BitScreen(QWidget *parent, QWidget *win) : QObject(parent)
{
  this->screen = QApplication::screens().at(0);
#ifdef USE_FULLSCREEN

    int wS = screen->size().width(); int hS = screen->size().height();
  this->myWindowWidth = wS;
  this->screenFixed = true;
  this->checkScreenType();
  this->maybeMoveOrResize(win);

#else
  this->screenFixed = false;
  setScreenType("desc");
#endif

  connect(screen, SIGNAL(primaryOrientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged(Qt::ScreenOrientation)));
  connect(screen, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(screenOrientationChanged(Qt::ScreenOrientation)));
}
BitScreen::~BitScreen()
{
    //QSettings settings;
    //QByteArray stateArray = saveState();
    //settings.setValue("mainWindowState", stateArray);
}
void BitScreen::checkScreenType()
{
#ifdef USE_FULLSCREEN

    /*_ldpi_320x480 _mdpi_480x800 _hdpi_720x1280 _xldpi_900x1600 _xhdpi_1080x1920*/

    int wS = size().width(); int hS = size().height();
    if((wS<=480 && hS<=320) || (hS<=480 && wS<=320)){
        this->setScreenType("ldpi");
    }else if((wS<=800 && hS<=480) || (hS<=800 && wS<=480)){
        setScreenType("mdpi");
    }else if((wS<=1280 && hS<=720) || (hS<=1280 && wS<=720)){
        setScreenType("hdpi");
    }else if((wS<=1600 && hS<=900) || (hS<=1600 && wS<=900)){
        setScreenType("xdpi");
    }else if((wS>=1920 && hS>=1080) || (hS>=1920 && wS>=1080)){
        setScreenType("xxhdpi");
    }else{
        setScreenType("unknown");
    }

#else
  setScreenType("desc");
#endif

}

QSize BitScreen::size() const{
    return screen->size();
}
QSize BitScreen::virtualSize() const{
    // all "available" parameters first load giving correct result.
    // after change orientation - giving out of preview sizes
    // hack size issue
    // first load - correct parameters, return correct function
    if(    (screen->size().width() == screen->availableSize().width())
        || (screen->size().height()==screen->availableSize().height())
       )
    {
        return screen->availableVirtualSize();
    }
    int w = screen->size().width(); int h = screen->size().height();
    if(w>screen->availableSize().height() && h==screen->availableSize().width()){ // changed width only
        return QSize(w, h-(w-screen->availableSize().height()));
    }
    else if(w==screen->availableSize().height() && h>screen->availableSize().width()){ // changed height only
        return QSize(w-(h-screen->availableSize().width()), h);
    }
    else if(w>screen->availableSize().height() && h>screen->availableSize().width()){
        return QSize(w-(h-screen->availableSize().width()), h-(w-screen->availableSize().height()));
    }
    //orientation changed, return w=h, h=w
    return screen->availableVirtualSize();
}
bool BitScreen::isPortrait(Qt::ScreenOrientation orientation) const{
    return screen->isPortrait(orientation);
}
bool BitScreen::isPortrait() const{
    //return screen->isPortrait(screen->orientation());
    return screen->isPortrait(screen->primaryOrientation());
}
//bool isPortrait(Qt::ScreenOrientation contentOrientation()) const;
//bool isLandscape(Qt::ScreenOrientation contentOrientation())const;
Qt::ScreenOrientation BitScreen::orientation() const{
    return screen->orientation();
}

void BitScreen::adjustButtonSize(QPushButton *btn, const QString &txt){
   btn->setIconSize(iconSize());
   btn->setMaximumSize(buttonFixedSize());
   //if(txt.isEmpty()){ txt = ""; }
   btn->setText(txt);
}

bool BitScreen::maybeMoveOrResize(QWidget *win){
#ifdef USE_FULLSCREEN
      if(win->isWindow())
          win->setFixedSize(virtualSize());
          //win->setWindowState(win->windowState() ^ Qt::WindowMaximized);
          //win->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
          // win->setFixedWidth(size().width());
          //win->move(0,0);

    return true;
#else
  #ifdef USE_MINISCREEN
       //useMiniScreen==true &&
      if(win->isMaximized()==false){
        if(this->size().width()<640){ //vertical turn
            //parent->setWindowState(parent->windowState() ^ Qt::WindowFullScreen); // this set fullfull screen
            win->setWindowState(win->windowState() ^ Qt::WindowMaximized);
            return true;
        }
      }
  #endif
#endif
    return false;
}

void BitScreen::adjustPopupDialogButtonsSize(QDialog *dlg, QDialogButtonBox *buttonBox)
{
    buttonBox->setFixedWidth(dlg->width()
                             -dlg->layout()->contentsMargins().left()-dlg->layout()->contentsMargins().right());
    int w = dlg->width()
            -dlg->layout()->contentsMargins().left()-dlg->layout()->contentsMargins().right()
            -buttonBox->layout()->contentsMargins().left()-buttonBox->layout()->contentsMargins().right();
    int cnt = buttonBox->buttons().size();
    int bw = ((w/cnt)-(6*cnt)); // width/num buttons - num_btn*btn spacing
    foreach(QAbstractButton *p, buttonBox->buttons())
    {
        p->setMinimumWidth( bw );
    }
    buttonBox->setCenterButtons(true);
}

/**
 * @brief BitScreen::adjustPopupDialogSize
 * not a fullscreen dialog, adapted for small screen only
 */
void BitScreen::adjustPopupDialogSize(QDialog *dlg, bool move)
{
    dlg->setStyleSheet("QDialog {border: 2px solid grey;}");
    dlg->adjustSize();
    int w = (virtualSize().width()<1000 ? virtualSize().width():1000); //max dialog width
    int x = (w*10/100);//decrease on 10%
    if(isScreenFixed()){
        dlg->setFixedWidth(w-x);
    }
    if(move)
       dlg->move((virtualSize().width()-dlg->width())/2, (virtualSize().height() - dlg->height())/2);
}
/**
 * adjust dialog full screen size (visible area)
*/
void BitScreen::adjustDialogFullScreen(QDialog *dlg)
{
    if(isScreenFixed()){
        dlg->resize(virtualSize());
        dlg->setFixedWidth(virtualSize().width());
        dlg->setMaximumHeight(virtualSize().height());
        dlg->move(0,0);
    }
}


void BitScreen::screenOrientationChanged(Qt::ScreenOrientation orientation){
    emit orientationChanged(orientation);
}
void BitScreen::virtualGeometryChanged(const QRect &rect){
    emit screenChanged();
}


void BitScreen::setDialogMask(QWidget *widget){

}

void BitScreen::setDialogMask(QWidget &widget)
{
    // grab the geometry
    QRect fg = widget.frameGeometry();
    QRect rg = widget.geometry();

    // get some reusable values
    int nCaptionHeight = rg.top()	- fg.top();
    int nFrameWidth = rg.left()	- fg.left();

    // map the global coords to local for the frame
    fg.moveTo(widget.mapFromGlobal(fg.topLeft()));

    // create a rectangle for the frame
    QRect rectFrame(fg.left() - nFrameWidth,
    fg.top() - nCaptionHeight,
    fg.width() + ( nFrameWidth * 2 ),
    fg.height() + ( nCaptionHeight + nFrameWidth ) );

    // create a region from the frame rectangle
    QRegion regionFrame( rectFrame );

    // map the global coords to local for the client area
    rg.moveTo(widget.mapFromGlobal(rg.topLeft()));

    // move the rect to 0,0
    rg.moveTo( QPoint(0,0) );

    // create a region from the client rect
    QRegion regionClient( rg );

    // set a mask to be the frame region minus the client region
    //createMaskFromColor(const QColor & maskColor, Qt::MaskMode mode = Qt::MaskInColor) ;
    //regionFrame
    widget.setMask( regionFrame - regionClient );
    //QPixmap pix(icon);QColor(0, 0, 0, 0), Qt::MaskMode, mode = Qt::MaskInColor
    //regionClient. createMaskFromColor(QColor(0,0,0,0)) ;
    //widget.setMask( createMaskFromColor );
}
