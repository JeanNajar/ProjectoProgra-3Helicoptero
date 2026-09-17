#ifndef BALA_H
#define BALA_H

#include <QObject>
#include <QGraphicsPixmapItem>

class QGraphicsScene;

// Proyectil disparado por el helicóptero (solo desde el nivel 2).
// Avanza hacia la derecha y destruye los obstáculos que toca.
class Bala : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    Bala(qreal xPos, qreal yPos, QGraphicsScene *escena);

public slots:
    void move();

private:
    QGraphicsScene *escena;
};

#endif // BALA_H