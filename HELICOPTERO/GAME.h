#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsScene>
#include "MyHeli.h"
#include "Score.h"
#include "Health.h"

class Game : public QGraphicsView {
    Q_OBJECT //

public:
    // Constructor
    Game(QWidget *parent = nullptr);

    // Atributos principales del juego
    QGraphicsScene * scene;
    MyHeli * heli;
    Score * score;
    Health * health;
};

#endif // GAME_H