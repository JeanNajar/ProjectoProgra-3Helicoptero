#ifndef OBSTACLEH_H
#define OBSTACLEH_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QMediaPlayer>
#include "ObstacleType.h"

class ObstacleH: public QObject, public QGraphicsRectItem{
    Q_OBJECT
public:
    ObstacleH(ObstacleType type = ObstacleType::VERTICAL);
    ~ObstacleH();
    ObstacleType getType() const;
public slots:
    void move();

private:
    QMediaPlayer *CrashSound;
    QAudioOutput *audioOutput;  // Se guarda para liberarlo en el destructor
    ObstacleType tipo;
};


#endif // OBSTACLEH_H
