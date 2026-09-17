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

    // nivel: 1 = ciudad, 2 = desierto, 3 = nieve (sprites del superviviente)
    SurvivorManager(QGraphicsScene *scene, ObstacleManager *obstacleManager,
                    int nivel = 1, QObject *parent = nullptr);

    ~SurvivorManager();

    bool spawnSurvivorBehindObstacle(ObstacleH *obstaculo);

    void updateAll(QRectF heliRect);

    int countActive() const;

    bool allSpawned() const;

    // Cuántos supervivientes se rescataron de verdad (barra completada)
    int getTotalRescatados() const;

    // Cuántos supervivientes hay que rescatar en este nivel
    // (5 en el nivel 1, 10 en el nivel 2, 15 en el nivel 3).
    int getTotalObjetivo() const;

    // Progreso total de rescate: rescatados*100 + progreso de los activos.
    // El máximo es getTotalObjetivo()*100 (→ 100%).
    int getProgresoTotal() const;

private:
    QGraphicsScene *scene;              // Escena donde se dibujan
    ObstacleManager *obstacleManager;   // Para saber dónde están los obstáculos
    QList<Survivor*> supervivientes;    // Lista de supervivientes activos
    int nivel;                          // Nivel actual (elige los sprites)
    int totalGenerados;                 // Cuántos supervivientes se han generado
    int maximoSupervivientes;           // Máximo por nivel (2 o 3)
    int totalRescatados;                // Cuántos se rescataron completando la barra

    bool posicionLibre(qreal xPos, qreal yPos) const;
};

#endif // SURVIVORMANAGER_H
