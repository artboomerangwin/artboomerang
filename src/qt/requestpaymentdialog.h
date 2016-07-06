#ifndef REQUESTPAYMENTDIALOG_H
#define REQUESTPAYMENTDIALOG_H

#include <QDialog>
#include <QImage>
#include <QTextEdit>
#include <QMovie>
#include "guiutil.h"
#include "plugins/smtp/smtp.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif
namespace Ui {
    class RequestPaymentDialog;
}
class OptionsModel;

class RequestPaymentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RequestPaymentDialog(const QString &addr, const QString &label, QWidget *parent = 0, BitScreen *screen=0);
    ~RequestPaymentDialog();

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
    void on_btnCancel_clicked();
    void on_chkIncludeMessage_toggled(bool fChecked);
    void on_btnPreviewRequest_clicked();
    void on_btnSendRequest_clicked();
    void on_btnSendMore_clicked();
    void on_btnCancelRequest_clicked();
    void on_btnClear_clicked();
    void on_btnRequestSendTo_clicked();
    void on_btnRequestCopyTo_clicked();
    void on_btnPasteMessage_clicked();
    void mailSent(const QString &text);
    void updateRequestFields();
    bool checkRequestForm();
    void updateMessageBox(const QString &text);
    void optionsClicked();

    void updateDisplayUnit();

private:
#ifdef USE_FULLSCREEN
    ActionBar *actionBar;
#endif
    QWidget *fullScreenQRCode;
    QWidget *requestPreview;
    Ui::RequestPaymentDialog *ui;
    OptionsModel *model;
    QString address;
    QString addressURI;
    QImage myImage;
    BitScreen *screen;

    void genCode();
    QString getURI();
    QString createRequestPaymentMessage();
    //void sendMessage();
    bool eventFilter(QObject *object, QEvent *event);
protected:
    QString appTitle;
    QMovie *movie;
    Smtp* smtp;
    void keyPressEvent(QKeyEvent *event);
};

#endif // REQUESTPAYMENTDIALOG_H
