#ifndef OBSTACLEH_H
#define OBSTACLEH_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QMediaPlayer>

class ObstacleH: public QObject, public QGraphicsRectItem{
    Q_OBJECT
public:
    ObstacleH();

public slots:
    void move();

private:
    QMediaPlayer *CrashSound;
};


#endif // OBSTACLEH_H
