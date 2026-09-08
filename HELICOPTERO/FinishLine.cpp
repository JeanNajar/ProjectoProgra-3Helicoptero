#include "FinishLine.h"
#include <QGraphicsScene>
#include <QBrush>
#include <QPen>
#include <QColor>

FinishLine::FinishLine(QGraphicsScene *scene, QObject *parent)
    : QObject(parent), QGraphicsRectItem()
{

    int zonaAncho = 150;
    int zonaAlto = 40;
    int zonaX = static_cast<int>(scene->width()) - 200;
    int zonaY = static_cast<int>(scene->height()) - zonaAlto;

    setRect(zonaX, zonaY, zonaAncho, zonaAlto);


    QBrush brush(QColor(0, 200, 0, 40));   // Verde
    QPen pen(Qt::white, 3);                // Borde blanco
    setBrush(brush);
    setPen(pen);

    // Agregar a la escena
    scene->addItem(this);
}