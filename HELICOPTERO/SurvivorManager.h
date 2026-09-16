#ifndef SURVIVORMANAGER_H
#define SURVIVORMANAGER_H

#include <QObject>
#include <QGraphicsScene>
#include <QList>
#include "Survivor.h"
#include "ObstacleManager.h"


class SurvivorManager : public QObject {
    Q_OBJECT

public:

    SurvivorManager(QGraphicsScene *scene, ObstacleManager *obstacleManager,
                    QObject *parent = nullptr);

    ~SurvivorManager();

    bool spawnSurvivorBehindObstacle(ObstacleH *obstaculo);

    void updateAll(QRectF heliRect);

    int countActive() const;

    bool allSpawned() const;

    // Cuántos supervivientes se rescataron de verdad (barra completada)
    int getTotalRescatados() const;

    // Progreso total de rescate: rescatados*100 + progreso de los activos.
    // Con 2 supervivientes el máximo es 200 (→ 100%).
    int getProgresoTotal() const;

private:
    QGraphicsScene *scene;              // Escena donde se dibujan
    ObstacleManager *obstacleManager;   // Para saber dónde están los obstáculos
    QList<Survivor*> supervivientes;    // Lista de supervivientes activos
    int totalGenerados;                 // Cuántos supervivientes se han generado (máx 2)
    int totalRescatados;                // Cuántos se rescataron completando la barra

    bool posicionLibre(qreal xPos, qreal yPos) const;
};

#endif // SURVIVORMANAGER_H
