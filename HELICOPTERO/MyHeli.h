#ifndef MYHELI_H
#define MYHELI_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QMediaPlayer>

class MyHeli: public QObject, public QGraphicsRectItem{

  Q_OBJECT

public:
    void keyPressEvent(QKeyEvent * event);
public slots:
    void spawn();

};



#endif // MYHELI_H
