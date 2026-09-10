#include "SurvivorManager.h"
#include <QDebug>

SurvivorManager::SurvivorManager(QGraphicsScene *scene, ObstacleManager *obstacleManager,
                                 QObject *parent)
    : QObject(parent)
{
    this->scene = scene;
    this->obstacleManager = obstacleManager;
    totalGenerados = 0;
}

SurvivorManager::~SurvivorManager(){
    // Liberar todos los supervivientes
    for(int i = 0; i < supervivientes.size(); i++){
        if(supervivientes[i] != nullptr){
            if(supervivientes[i]->scene() != nullptr){
                scene->removeItem(supervivientes[i]);
            }
            delete supervivientes[i];
            supervivientes[i] = nullptr;
        }
    }
    supervivientes.clear();
}

bool SurvivorManager::posicionLibre(qreal xPos, qreal yPos) const{

  // Verifica que la posición (xPos, yPos) no esté dentro de un obstáculo.

    QRectF zonaSuperviviente(xPos, yPos, 40, 40);

    int cantidad = obstacleManager->getCantidad();
    for(int i = 0; i < cantidad; i++){
        ObstacleH *obs = obstacleManager->getObstacle(i);
        if(obs == nullptr){
            continue;  // Slot vacío (obstáculo ya muerto)
        }
        // Solo considerar obstáculos que siguen en la escena
        if(obs->scene() == nullptr){
            continue;
        }
        QRectF zonaObstaculo = obs->boundingRect().translated(obs->pos());
        if(zonaSuperviviente.intersects(zonaObstaculo)){
            return false;  // Hay un obstáculo ahí
        }
    }
    return true;
}

bool SurvivorManager::spawnSurvivorBehindObstacle(ObstacleH *obstaculo){
//genera un superviviente adelante de los obstaculos
    if(totalGenerados >= 2){
        return false;  // Ya se generaron los 2
    }

    if(obstaculo == nullptr || obstaculo->scene() == nullptr){
        return false;
    }

    // El suelo está en la parte inferior de la pantalla
    qreal yPos = scene->height() - 40;

    // Posición X: ADELANTE del obstáculo (a la derecha de él), fuera de
    // pantalla. El obstáculo aparece en x = scene->width(). El superviviente
    // aparece más a la derecha (por ejemplo 200px más), así entra después.
    qreal xPos = obstaculo->pos().x() + 200;

    // Crear el superviviente (fuera de pantalla, entrará gradualmente)
    Survivor *nuevo = new Survivor(xPos, yPos, scene);
    scene->addItem(nuevo);
    supervivientes.append(nuevo);
    totalGenerados++;
    qDebug() << "SurvivorManager: Superviviente creado adelante del obstáculo en x=" << xPos
             << "(total:" << totalGenerados << ")";
    return true;
}

void SurvivorManager::updateAll(QRectF heliRect){
//verificar posicion y estado(si esta aun o no)
    for(int i = supervivientes.size() - 1; i >= 0; i--){
        Survivor *s = supervivientes[i];
        if(s == nullptr){
            supervivientes.removeAt(i);
            continue;
        }

        // Decirle al superviviente si el heli está encima
        s->setHeliEncima(s->isHeliOver(heliRect));

        // Si ya fue rescatado, eliminarlo
        if(s->isRescued()){
            // Ya fue removido de la escena por el propio Survivor
            delete s;
            supervivientes.removeAt(i);
        }
    }
}

int SurvivorManager::countActive() const{
    return supervivientes.size();
}

bool SurvivorManager::allSpawned() const{
    return totalGenerados >= 2;
}
