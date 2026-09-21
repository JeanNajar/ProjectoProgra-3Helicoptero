#ifndef LEVELMAPGENERATOR_H
#define LEVELMAPGENERATOR_H

#include <QThread>
#include <QMutex>
#include <QList>
#include "ObstacleType.h"

// Hilo PRODUCTOR del "mapa" del nivel (productor-consumidor): genera la
// secuencia de obstaculos/supervivientes ANTES de que el juego los necesite;
// el hilo principal los consume con hasNext()/next(). Nunca toca
// QGraphicsScene (no es thread-safe). Cola protegida con QMutex.
class LevelMapGenerator : public QThread {
    Q_OBJECT

public:
    // Un "bloque" del mapa: que obstaculo aparece y si lleva superviviente
    struct SpawnData {
        ObstacleType type;      // Tipo de obstáculo (VERTICAL/SMALL/CEILING)
        bool spawnSurvivor;     // ¿Generar un superviviente detrás?
    };

    // @param levelDurationMs  Duracion del nivel en ms (30s = 30000)
    // @param spawnIntervalMs  Cada cuantos ms aparece un obstaculo (3000)
    LevelMapGenerator(int levelDurationMs, int spawnIntervalMs,
                      QObject *parent = nullptr);
    ~LevelMapGenerator();

    // Cuerpo del hilo: genera el mapa completo y lo guarda en la cola
    void run() override;

    // ===== API PARA EL HILO PRINCIPAL (consumidor) =====

    // ¿Quedan datos en la cola? (consulta segura con mutex)
    bool hasNext() const;

    // Toma el siguiente bloque del mapa y lo saca de la cola
    SpawnData next();

    // Cuantos bloques quedan por consumir
    int count() const;

private:
    int levelDurationMs;    // Duración total del nivel
    int spawnIntervalMs;    // Intervalo entre obstáculos

    QList<SpawnData> cola;  // Cola de datos generados (compartida)
    mutable QMutex mutex;   // Candado: solo un hilo accede a la cola a la vez
};

#endif // LEVELMAPGENERATOR_H