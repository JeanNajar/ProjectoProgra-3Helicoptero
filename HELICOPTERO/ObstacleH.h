#ifndef OBSTACLEH_H
#define OBSTACLEH_H
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QMediaPlayer>
#include "ObstacleType.h"
//esto para que la memoria unicamente la controle obstacle manager, evita dependencia circular
class ObstacleManager;
class ObstacleH: public QObject, public QGraphicsPixmapItem{
    Q_OBJECT
public:
    ObstacleH(ObstacleType type, ObstacleManager *manager);
    ~ObstacleH();
    ObstacleType getType() const;
public slots:
    void move();
private:
    QMediaPlayer *CrashSound;
    QAudioOutput *audioOutput;  // Se guarda para liberarlo en el destructor
    ObstacleType tipo;
    ObstacleManager *manager;
};
#endif // OBSTACLEH_H