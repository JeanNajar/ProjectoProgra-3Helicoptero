#ifndef LEVELMAPGENERATOR_H
#define LEVELMAPGENERATOR_H

#include <QThread>
#include <QMutex>
#include <QList>
#include "ObstacleType.h"

/**
 * @brief Hilo PRODUCTOR que pre-genera el "mapa" del nivel.
 *
 * PATRÓN PRODUCTOR-CONSUMIDOR:
 *  - Este hilo (productor) genera TODA la secuencia de obstáculos y
 *    supervivientes del nivel ANTES de que el juego los necesite.
 *  - El hilo principal (consumidor) va tomando los datos de la cola
 *    con hasNext()/next() cada vez que el timer de spawn dispara.
 *
 * ¿POR QUÉ UN HILO Y NO GENERARLO EN EL JUEGO?
 *  - Separa el DISEÑO del nivel (datos) del RENDERIZADO (gráficos).
 *  - El mapa queda "pre-hecho": el juego solo consume, nunca calcula.
 *  - El hilo NUNCA toca QGraphicsScene (no es thread-safe): solo
 *    produce datos planos (tipo de obstáculo, si lleva superviviente).
 *
 * SEGURIDAD:
 *  - La cola está protegida con QMutex (evita condiciones de carrera).
 *  - El destructor espera (wait()) a que el hilo termine antes de
 *    liberar memoria (evita crash al cerrar el juego).
 */
class LevelMapGenerator : public QThread {
    Q_OBJECT

public:
    /**
     * @brief Un "bloque" del mapa: qué obstáculo aparece y si lleva
     *        un superviviente detrás.
     */
    struct SpawnData {
        ObstacleType type;      // Tipo de obstáculo (VERTICAL/SMALL/CEILING)
        bool spawnSurvivor;     // ¿Generar un superviviente detrás?
    };

    /**
     * @param levelDurationMs  Duración del nivel en milisegundos (30s = 30000)
     * @param spawnIntervalMs  Cada cuántos ms aparece un obstáculo (3000)
     */
    LevelMapGenerator(int levelDurationMs, int spawnIntervalMs,
                      QObject *parent = nullptr);
    ~LevelMapGenerator();

    /**
     * @brief Cuerpo del hilo: genera el mapa completo y lo guarda en la cola.
     *        Se ejecuta en el hilo secundario cuando se llama start().
     */
    void run() override;

    // ===== API PARA EL HILO PRINCIPAL (consumidor) =====

    /**
     * @brief ¿Quedan datos en la cola? (consulta segura con mutex)
     */
    bool hasNext() const;

    /**
     * @brief Toma el siguiente bloque del mapa y lo saca de la cola.
     */
    SpawnData next();

    /**
     * @brief Cuántos bloques quedan por consumir.
     */
    int count() const;

private:
    int levelDurationMs;    // Duración total del nivel
    int spawnIntervalMs;    // Intervalo entre obstáculos

    QList<SpawnData> cola;  // Cola de datos generados (compartida)
    mutable QMutex mutex;   // Candado: solo un hilo accede a la cola a la vez
};

#endif // LEVELMAPGENERATOR_H