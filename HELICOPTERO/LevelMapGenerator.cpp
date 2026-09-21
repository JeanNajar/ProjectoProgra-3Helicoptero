#include "LevelMapGenerator.h"

LevelMapGenerator::LevelMapGenerator(int levelDurationMs, int spawnIntervalMs,
                                     QObject *parent)
    : QThread(parent)
{
    this->levelDurationMs = levelDurationMs;
    this->spawnIntervalMs = spawnIntervalMs;
}

LevelMapGenerator::~LevelMapGenerator(){
    // Seguridad: esperar a que el hilo termine antes de destruirlo
    if(isRunning()){
        requestInterruption();
        wait();
    }
}

void LevelMapGenerator::run(){
    // ===== CUERPO DEL HILO (hilo secundario) =====
    // Genera el mapa: cuantos obstaculos, de que tipo y cuales llevan superviviente

    int totalSpawns = levelDurationMs / spawnIntervalMs;  // 30s / 3s = 10
    int contador = 0;

    for(int i = 0; i < totalSpawns; i++){
        // Permitir detener el hilo limpiamente (requestInterruption)
        if(isInterruptionRequested()){
            break;
        }

        SpawnData dato;

        // Tipo cíclico: VERTICAL -> SMALL -> CEILING -> VERTICAL -> ...
        switch(contador % 3){
        case 0:
            dato.type = ObstacleType::VERTICAL;
            break;
        case 1:
            dato.type = ObstacleType::SMALL;
            break;
        case 2:
            dato.type = ObstacleType::CEILING;
            break;
        }

        // Los primeros 2 obstáculos del mapa llevan un superviviente detrás
        dato.spawnSurvivor = (contador < 2);

        // QMutexLocker protege la lista mientras el hilo principal la lee
        {
            QMutexLocker candado(&mutex);
            cola.append(dato);
        }

        contador++;
    }
}

bool LevelMapGenerator::hasNext() const{
    QMutexLocker candado(&mutex);
    return !cola.isEmpty();
}

LevelMapGenerator::SpawnData LevelMapGenerator::next(){
    QMutexLocker candado(&mutex);
    SpawnData dato = cola.first();
    cola.removeFirst();
    return dato;
}

int LevelMapGenerator::count() const{
    QMutexLocker candado(&mutex);
    return cola.size();
}