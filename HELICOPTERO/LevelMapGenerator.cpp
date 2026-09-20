#include "LevelMapGenerator.h"

LevelMapGenerator::LevelMapGenerator(int levelDurationMs, int spawnIntervalMs,
                                     QObject *parent)
    : QThread(parent)
{
    this->levelDurationMs = levelDurationMs;
    this->spawnIntervalMs = spawnIntervalMs;
}

LevelMapGenerator::~LevelMapGenerator(){
    // Seguridad: si el hilo sigue corriendo cuando se destruye el objeto,
    // pedir que se detenga y ESPERAR a que termine. Sin esto, destruir un
    // QThread en ejecución es un crash garantizado.
    if(isRunning()){
        requestInterruption();
        wait();
    }
}

void LevelMapGenerator::run(){
    // ===== CUERPO DEL HILO (se ejecuta en el hilo secundario) =====
    // Genera el mapa completo del nivel: cuántos obstáculos aparecen,
    // de qué tipo, y cuáles llevan superviviente detrás.

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

        // Guardar en la cola. El QMutexLocker bloquea el mutex mientras
        // se modifica la lista y lo libera al salir del bloque {}.
        // Sin esto, el hilo principal podría leer la lista mientras
        // este hilo la modifica -> condición de carrera.
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