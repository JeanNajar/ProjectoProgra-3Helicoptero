#ifndef OBSTACLEMANAGER_H
#define OBSTACLEMANAGER_H

#include <QObject>
#include <QGraphicsScene>
#include "ObstacleH.h"
#include "ObstacleType.h"

class ObstacleManager : public QObject {
 Q_OBJECT

public:

 ObstacleManager(QGraphicsScene *scene,QObject *parent = nullptr);

 //destructor
 ~ObstacleManager();

 void spawnObstacle(ObstacleType type, int yPos);
 void removerObstacle(int index);
 int getCantidad() const;

 ObstacleH* getObstacle(int index) const;

 private:
     ObstacleH **obstaculos;
     int cantidad;
     QGraphicsScene *scene;
};




#endif // OBSTACLEMANAGER_H
