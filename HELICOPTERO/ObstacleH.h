#ifndef OBSTACLEH_H
#define OBSTACLEH_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QMediaPlayer>
#include <QGraphicsRectItem>
#include "ObstacleType.h"
//esto para que la memoria unicamente la controle obstacle manager, evita dependencia circular
class ObstacleManager;
class ObstacleH: public QObject, public QGraphicsPixmapItem{
    Q_OBJECT
public:
    ObstacleH(ObstacleType type, ObstacleManager *manager, int nivel = 1);
    ~ObstacleH();
    ObstacleType getType() const;
    // Una bala le pegó al obstáculo: muestra la barra de vida (en el primer
    // impacto) y resta vida; al llegar a 0 el obstáculo se destruye.
    void recibirImpacto();
public slots:
    void move();
private:
    // Sonido de crash COMPARTIDO entre todos los obstaculos (evita que Qt
    // extraiga el .wav del qrc a un temporal por cada uno: spam de ffprobe)
    static QMediaPlayer *CrashSound;
    static QAudioOutput *audioOutput;
    ObstacleType tipo;
    ObstacleManager *manager;
    int nivel;                  // nivel actual (elige el pixmap del obstáculo)

    // Barra de vida estilo Survivor: oculta hasta el primer impacto de bala
    int vida;                   // impactos que aguanta
    int vidaMaxima;             // vida inicial (para calcular el ancho)
    bool barraVisible;          // false hasta el primer impacto
    QGraphicsRectItem *barraFondo;  // fondo oscuro de la barra (hija)
    QGraphicsRectItem *barra;       // relleno amarillo que se reduce (hija)
};
#endif // OBSTACLEH_H