#ifndef HEALTH_H
#define HEALTH_H

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>

class Health: public QGraphicsTextItem{
public:

    Health(QGraphicsItem * parent=0);
    void decrease();
    int getHealth();
    void updateDisplay();

private:
    int health;
    QGraphicsRectItem *fondoBarra;
    QGraphicsRectItem *barraVida;
};


#endif // HEALTH_H
