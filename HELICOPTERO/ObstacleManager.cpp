#include "ObstacleManager.h"
#include <QDebug>

ObstacleManager::ObstacleManager(QGraphicsScene *scene, QObject *parent):QObject(parent){

    obstaculos = nullptr;
    cantidad = 0;
    this->scene = scene;
}

ObstacleManager::~ObstacleManager(){

    //destructor envargado de liberar la memoria dinamica

    for(int i=0; i<cantidad; i++){
        //si sigue viendose en la escena
        if(obstaculos[i] != nullptr){
            if(obstaculos[i]->scene()!=nullptr){
                scene->removeItem(obstaculos[i]);
        }
        delete obstaculos[i];
        obstaculos[i] = nullptr; //para evitar puntero colgante
    }
}
//liberar el arreglo de punteros osea la  matriz

if(obstaculos != nullptr){
    delete[] obstaculos;
    obstaculos = nullptr;
   }
cantidad = 0;
}

void ObstacleManager::spawnObstacle(ObstacleType type,int yPos){

    ObstacleH *nuevo = new ObstacleH(type);

    qreal xPos = scene->width();

    switch(type){
     case ObstacleType::VERTICAL:
        if (yPos <=0)
             yPos = static_cast<int>(scene->height()-300);
        break;
     case ObstacleType::SMALL:
         if (yPos <= 0)
             yPos = 200;
         break;
     case ObstacleType::CEILING:
         yPos = 0;
             break;

    }
    nuevo->setPos(xPos, yPos);

    ObstacleH **nuevaMatriz = new ObstacleH*[cantidad + 1];
    // Copiar los punteros existentes
    for(int i = 0; i < cantidad; i++){
        nuevaMatriz[i] = obstaculos[i];
    }
    //ponemos el obstaculo nuevo al final
    nuevaMatriz[cantidad] = nuevo;

    if(obstaculos != nullptr){
        delete[] obstaculos;
    }

    // Apuntar a la nueva matriz
    obstaculos = nuevaMatriz;
    cantidad++;

    scene->addItem(nuevo);
}

void ObstacleManager::removerObstacle(int index){

    if(index < 0 || index >= cantidad){
        qDebug() << "ObstacleManager: Índice inválido:" << index;//prevencion para evitar segment fault
        return;
    }

    if(obstaculos[index] != nullptr){
        if(obstaculos[index]->scene() != nullptr){
            scene->removeItem(obstaculos[index]);
        }
        delete obstaculos[index];
        obstaculos[index] = nullptr;  // Evitar puntero colgante
    }

      if(cantidad == 1){
        delete[] obstaculos;
        obstaculos = nullptr;
    } else {
        ObstacleH **nuevaMatriz = new ObstacleH*[cantidad - 1];

        int nuevaPos = 0;
        for(int i = 0; i < cantidad; i++){
            // Saltar el índice que se eliminó
            if(i != index){
                nuevaMatriz[nuevaPos] = obstaculos[i];
                nuevaPos++;
            }
        }
        delete[] obstaculos;
        obstaculos = nuevaMatriz;
    }
       cantidad--;
}

int ObstacleManager::getCantidad() const{
    return cantidad;
}

ObstacleH* ObstacleManager::getObstacle(int index) const{
    // Validación para evitar segmentation fault
    if(index < 0 || index >= cantidad){
        return nullptr;
    }
    return obstaculos[index];
}