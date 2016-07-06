#ifndef TRANSACTIONDESCDIALOG_H
#define TRANSACTIONDESCDIALOG_H

#include <QDialog>
#include "guiutil.h"
#ifdef USE_FULLSCREEN
#include "actionbar/actionbar.h"
#endif

namespace Ui {
    class TransactionDescDialog;
}
QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Dialog showing transaction details. */
class TransactionDescDialog : public QDialog
{
    Q_OBJECT
protected:
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *e);
public:
    explicit TransactionDescDialog(const QModelIndex &idx, QWidget *parent = 0, BitScreen *screen=0);
    ~TransactionDescDialog();

private slots:
    void screenOrientationChanged();
    void copyDetails();
    void copyTxData(const QString &v, const QString &title);
    void copyAddress();
    void copyLabel();
    void copyAmount();
    void copyTxID();
private:
    BitScreen *screen;
    Ui::TransactionDescDialog *ui;
    QString address;
    QString label;
    QString txid;
    QString amount;

signals:
    void stopExec();
};

#endif // TRANSACTIONDESCDIALOG_H
