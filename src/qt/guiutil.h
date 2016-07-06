#ifndef GUIUTIL_H
#define GUIUTIL_H

#include <QString>
#include <QObject>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLayout>
#include <QScreen>

QT_BEGIN_NAMESPACE
class QFont;
class QLineEdit;
class QWidget;
class QDateTime;
class QUrl;
class QAbstractItemView;
QT_END_NAMESPACE
class SendCoinsRecipient;


/**
 * screen and window container
 */
//class BitScreen;
class BitScreen : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString screenType READ screenType WRITE setScreenType NOTIFY screenChanged)
    //Q_PROPERTY(bool isPortrait READ isPortrait NOTIFY orientationChanged)
    Q_PROPERTY(QSize iconSize READ iconSize)
public:
    explicit
    BitScreen(QWidget *parent = 0, QWidget *win=0);
    ~BitScreen();
    //BitScreen(QWidget  *parent = 0)
    //{

    //}
    bool maybeMoveOrResize(QWidget *win);
    bool isScreenFixed(){ return screenFixed;   }
    int getMyWindowWidth(){ return myWindowWidth; }
    QString screenType() const{ return m_screenType; }

    bool isPortrait() const;
    bool isPortrait(Qt::ScreenOrientation orientation) const;
    Qt::ScreenOrientation orientation() const;
    QSize size() const;
    QSize virtualSize() const;
    QRect geometry() const { return screen->geometry(); }
    QSize buttonFixedSize() {
        return QSize(iconSize().width()*2,iconSize().height()*2);
    }
    QSize iconSize(){
        if(screenType().isEmpty())
            checkScreenType();
        if(screenIconSize.isEmpty()){
            if(screenType()=="desc")
                screenIconSize = QSize(16,16);
            if(screenType()=="ldpi")
                screenIconSize = QSize(24,24);
            if(screenType()=="mdpi")
                screenIconSize = QSize(32,32);
            if(screenType()=="hdpi")
                screenIconSize = QSize(48,48);
            if(screenType()=="xhdpi")
                screenIconSize = QSize(64,64);
            if(screenType()=="xxhdpi")
                screenIconSize = QSize(96,96);
        }
        return screenIconSize;
    }
    void setLabelIcon(QLabel *label, const QString &icon){
       QPixmap pix(icon);  // ":/images/myimage"
       label->setPixmap( pix.scaled(iconSize().width(),iconSize().height(),  Qt::KeepAspectRatio));
    }
    void setDialogMask(QWidget &widget);
    void setDialogMask(QWidget *widget);
    void adjustDialogFullScreen(QDialog *dlg);
    void adjustPopupDialogSize(QDialog *dlg, bool move=false);
    void adjustPopupDialogButtonsSize(QDialog *dlg, QDialogButtonBox *buttonBox);

public slots:
    void adjustButtonSize(QPushButton* btn, const QString &txt);

private:
    QScreen *screen;
    bool screenFixed;
    int myWindowWidth;
    QString m_screenType;
    QSize screenIconSize;

private slots:
    void screenOrientationChanged(Qt::ScreenOrientation orientation);
    void virtualGeometryChanged(const QRect &rect);
    void checkScreenType();
    void setScreenType(const QString &v){
        m_screenType = v;
        emit screenChanged();
    }

signals:
    void screenChanged();
    void orientationChanged(Qt::ScreenOrientation orientation);
};
/** Utility functions used by the Bitcoin Qt UI.
 */
namespace GUIUtil
{
    // Create human-readable string from date
    QString dateTimeStr(const QDateTime &datetime);
    QString dateTimeStr(qint64 nTime);

    // Render Bitcoin addresses in monospace font
    QFont bitcoinAddressFont();

    // Set up widgets for address and amounts
    void setupAddressWidget(QLineEdit *widget, QWidget *parent);
    void setupAmountWidget(QLineEdit *widget, QWidget *parent);

    // Parse "artboomerang:" URI into recipient object, return true on successful parsing
    // See Bitcoin URI definition discussion here: https://bitcointalk.org/index.php?topic=33490.0
    bool parseBitcoinURI(const QUrl &uri, SendCoinsRecipient *out);
    bool parseBitcoinURI(QString uri, SendCoinsRecipient *out);
    bool validateEmailAddress(const QString &email, QString match="");

    // HTML escaping for rich text controls
    QString HtmlEscape(const QString& str, bool fMultiLine=false);
    QString HtmlEscape(const std::string& str, bool fMultiLine=false);

    /** Copy a field of the currently selected entry of a view to the clipboard. Does nothing if nothing
        is selected.
       @param[in] column  Data column to extract from the model
       @param[in] role    Data role to extract from the model
       @see  TransactionView::copyLabel, TransactionView::copyAmount, TransactionView::copyAddress
     */
    void copyEntryData(QAbstractItemView *view, int column, int role=Qt::EditRole);
    void setClipboard(const QString& str);

    /** Get save filename, mimics QFileDialog::getSaveFileName, except that it appends a default suffix
        when no suffix is provided by the user.

      @param[in] parent  Parent window (or 0)
      @param[in] caption Window caption (or empty, for default)
      @param[in] dir     Starting directory (or empty, to default to documents directory)
      @param[in] filter  Filter specification such as "Comma Separated Files (*.csv)"
      @param[out] selectedSuffixOut  Pointer to return the suffix (file type) that was selected (or 0).
                  Can be useful when choosing the save file format based on suffix.
     */
    QString getSaveFileName(QWidget *parent=0, BitScreen *screen=0, const QString &caption=QString(),
                                   const QString &dir=QString(), const QString &filter=QString(),
                                   QString *selectedSuffixOut=0);

    /** Get connection type to call object slot in GUI thread with invokeMethod. The call will be blocking.

       @returns If called from the GUI thread, return a Qt::DirectConnection.
                If called from another thread, return a Qt::BlockingQueuedConnection.
    */
    Qt::ConnectionType blockingGUIThreadConnection();

    // Determine whether a widget is hidden behind other windows
    bool isObscured(QWidget *w);

    // Open debug.log
    void openDebugLogfile();

    /** Qt event filter that intercepts ToolTipChange events, and replaces the tooltip with a rich text
      representation if needed. This assures that Qt can word-wrap long tooltip messages.
      Tooltips longer than the provided size threshold (in characters) are wrapped.
     */
    class ToolTipToRichTextFilter : public QObject
    {
        Q_OBJECT

    public:
        explicit ToolTipToRichTextFilter(int size_threshold, QObject *parent = 0);

    protected:
        bool eventFilter(QObject *obj, QEvent *evt);

    private:
        int size_threshold;
    };

    bool GetStartOnSystemStartup();
    bool SetStartOnSystemStartup(bool fAutoStart);
    /** Save window size and position */
    void saveWindowGeometry(const QString& strSetting, QWidget *parent);
    /** Restore window size and position */
    void restoreWindowGeometry(const QString& strSetting, const QSize &defaultSizeIn, QWidget *parent, BitScreen *scr);


    /** Help message for Bitcoin-Qt, shown with --help. */
    class HelpMessageBox : public QMessageBox
    {
        Q_OBJECT

    public:
        HelpMessageBox(QWidget *parent = 0);

        /** Show message box or print help message to standard output, based on operating system. */
        void showOrPrint();

        /** Print help message to console */
        void printToConsole();

    private:
        QString header;
        QString coreOptions;
        QString uiOptions;
    };

} // namespace GUIUtil

#endif // GUIUTIL_H
