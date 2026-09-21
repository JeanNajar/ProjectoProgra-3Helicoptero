#ifndef BIDONCOMBUSTIBLE_H
#define BIDONCOMBUSTIBLE_H

#include <QObject>
#include <QGraphicsPixmapItem>

class QGraphicsScene;

// Bidon de combustible: pickup que se mueve a la izquierda; al colisionar
// con el heli le resta combustible. Se destruye al salir o al ser recogido.
class BidonCombustible : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    BidonCombustible(qreal xPos, qreal yPos, QGraphicsScene *escena);

public slots:
    void move();

private:
    QGraphicsScene *escena;
};

#endif // BIDONCOMBUSTIBLE_H
