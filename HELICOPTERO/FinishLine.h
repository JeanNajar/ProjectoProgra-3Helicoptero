#ifndef FINISHLINE_H
#define FINISHLINE_H

#include <QObject>
#include <QGraphicsRectItem>

class FinishLine : public QObject, public QGraphicsRectItem {
    Q_OBJECT

public:
    FinishLine(QGraphicsScene *scene, QObject *parent = nullptr);
};


#endif // FINISHLINE_H
