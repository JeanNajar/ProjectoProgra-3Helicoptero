#include "SurvivorManager.h"

SurvivorManager::SurvivorManager(QGraphicsScene *scene, ObstacleManager *obstacleManager,
                                 int nivel, QObject *parent)
    : QObject(parent)
{
    this->scene = scene;
    this->obstacleManager = obstacleManager;
    this->nivel = nivel;
    totalGenerados = 0;
    totalRescatados = 0;
    // Supervivientes por nivel: nivel 1 = 5, nivel 2 = 10, nivel 3 = 15
    switch(nivel){
    case 2:
        maximoSupervivientes = 10;
        break;
    case 3:
        maximoSupervivientes = 15;
        break;
    default:
        maximoSupervivientes = 5;
        break;
    }
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

    // Zona ampliada 40px a cada lado (media heli): garantiza la
    // separacion horizontal minima, no solo que no quede encima
    QRectF zonaSuperviviente(xPos - 40, yPos, 40 + 80, 40);

    int cantidad = obstacleManager->getCantidad();
    for(int i = 0; i < cantidad; i++){
        ObstacleH *obs = obstacleManager->getObstacle(i);
        if(obs == nullptr){
            continue;  // Slot vacio
        }
        //solo se fija en los obstaculos que sigen en la escena
        if(obs->scene() == nullptr){
            continue;
        }
        // basicamente devuevle el size del pixmap
        QRectF zonaObstaculo = obs->boundingRect().translated(obs->pos());
        if(zonaSuperviviente.intersects(zonaObstaculo)){
            return false;  // Hay un obstaculo ahi
        }
    }
    return true;
}

bool SurvivorManager::spawnSurvivorBehindObstacle(ObstacleH *obstaculo){
    //genera un superviviente adelante de los obstaculos
    if(totalGenerados >= maximoSupervivientes){
        return false;  // Ya se generaron todos los de este nivel
    }

    if(obstaculo == nullptr || obstaculo->scene() == nullptr){
        return false;
    }

    // El suelo esta en la parte inferior de la pantalla
    qreal yPos = scene->height() - 40;

    // Posicioan X ADELANTE del obstáculo
    qreal xPos = obstaculo->pos().x() + 200;

    // Separacion minima: si queda pegada a otro obstaculo, correr el
    // superviviente a la derecha hasta tener 40px de espacio
    while(!posicionLibre(xPos, yPos)){
        xPos += 10;
    }

    // Crear el superviviente (con los sprites del nivel actual)
    Survivor *nuevo = new Survivor(xPos, yPos, scene, nivel);
    scene->addItem(nuevo);
    supervivientes.append(nuevo);
    totalGenerados++;
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

        // Si ya fue rescatado eliminarlo
        if(s->isRescued()){

            // Solo cuenta como rescatado si completó la barra de progreso
            if(s->fueRescatado()){
                totalRescatados++;
            }

            delete s;
            supervivientes.removeAt(i);
        }
    }
}

int SurvivorManager::countActive() const{
    return supervivientes.size();
}

bool SurvivorManager::allSpawned() const{
    return totalGenerados >= maximoSupervivientes;
}

int SurvivorManager::getTotalRescatados() const{
    return totalRescatados;
}

int SurvivorManager::getTotalObjetivo() const{
    return maximoSupervivientes;
}

int SurvivorManager::getProgresoTotal() const{
    int total = totalRescatados * 100;
    for(int i = 0; i < supervivientes.size(); i++){
        if(supervivientes[i] != nullptr){
            total += static_cast<int>(supervivientes[i]->getProgreso());
        }
    }
    return total;
}
