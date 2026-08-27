#ifndef MYHELI_H
#define MYHELI_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QMediaPlayer>

class MyHeli: public QObject, public QGraphicsRectItem{

  Q_OBJECT

public:
    void keyPressEvent(QKeyEvent * event);


};



#endif // MYHELI_H
