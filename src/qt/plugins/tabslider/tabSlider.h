
#ifndef TABSLIDER_H
#define TABSLIDER_H

#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif
#include <QWidget>
#include <QTabWidget>

class TabSlider: public QObject
{
   Q_OBJECT

public:

   TabSlider(QWidget *parent = NULL, QTabWidget *tabObj=NULL);
   virtual ~TabSlider();

public slots:
   void listenEvent(QObject *object, QEvent *event);

protected:


   int swipeDelay;
   QPoint swipePointX1;
   QPoint swipePointX2;
   QWidget *parentObj;
   QTabWidget *tab;

protected slots:
   void moveTab();

signals:
   void maybeMoveTab();
};

#endif
