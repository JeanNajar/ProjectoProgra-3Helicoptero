#ifndef BIDONCOMBUSTIBLE_H
#define BIDONCOMBUSTIBLE_H

#include <QObject>
#include <QGraphicsPixmapItem>

class QGraphicsScene;

// Bidón de combustible: pickup que se mueve hacia la izquierda (igual que los
// obstáculos). Al colisionar con el helicóptero, le resta combustible instantáneo.
// Se destruye al salir de la pantalla o al ser recogido.
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
