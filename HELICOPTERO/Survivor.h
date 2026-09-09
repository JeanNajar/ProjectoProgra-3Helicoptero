#ifndef SURVIVOR_H
#define SURVIVOR_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsScene>


class Survivor : public QObject, public QGraphicsRectItem {
    Q_OBJECT

public:

    Survivor(qreal xPos, qreal yPos, QGraphicsScene *scene);

    ~Survivor();

    bool isRescued() const;

    bool isHeliOver(QRectF heliRect) const;

    void setHeliEncima(bool encima);

public slots
    :
    void updateProgress();

    void move();

private:
    QGraphicsScene *escena;     // Escena donde se dibuja
    QTimer *timer;              // Timer que avanza el progreso
    QTimer *moveTimer;          // Timer que mueve el superviviente
    QGraphicsRectItem *barra;   // Barra de progreso (hija, se llena)
    QGraphicsRectItem *fondoBarra; // Fondo oscuro de la barra
    double progreso;            // Progreso actual (0 a 100)
    bool rescatado;             // si ya a fue rescatado?
    bool heliEncima;            // ¿El heli está encima en el último tick?
};

#endif // SURVIVOR_H
