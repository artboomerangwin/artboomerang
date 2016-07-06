
#include "tabSlider.h"


TabSlider::TabSlider(QWidget *parent, QTabWidget *tabObj) : QObject(),
    parentObj(parent)
{
  swipeDelay = 0;
  swipePointX1.setX(0);swipePointX1.setY(0);
  swipePointX2.setX(0);swipePointX2.setY(0);
  tab = tabObj;
  connect(this, SIGNAL(maybeMoveTab()), this, SLOT(moveTab()));

}

TabSlider::~TabSlider()
{
  swipeDelay = 0;
  swipePointX1.setX(0);swipePointX1.setY(0);
  swipePointX2.setX(0);swipePointX2.setY(0);

   delete tab;
}

void TabSlider::moveTab()
{
    if(swipeDelay>15){
        int index = tab->currentIndex();
        if(swipePointX1.x()-swipePointX2.x() > 0){ index++; }
        else{  index--;  }
        if (index >= tab->count()){  index = 0; }
        if (index < 0){  index = tab->count()-1; }
        tab->setCurrentIndex(index);
    }

}
void TabSlider::listenEvent(QObject *object, QEvent *event)
{
  if(object == tab){
      if(event->type() == QEvent::MouseButtonPress){
          swipeDelay = 0; swipePointX1 = parentObj->mapFromGlobal(QCursor::pos());
      }
      if(event->type() == QEvent::MouseButtonDblClick){
          swipeDelay = 0;
      }
      if(event->type() == QEvent::MouseMove){
          swipeDelay++;
      }
      if(event->type() == QEvent::MouseButtonRelease){
          swipePointX2 = parentObj->mapFromGlobal(QCursor::pos());
          emit maybeMoveTab();
      }
  }else{ swipeDelay = 0; }

}
