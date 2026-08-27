#include "Score.h"
#include <QFont>

Score::Score(QGraphicsItem *parent): QGraphicsTextItem(parent){
    //empieze en 0
    score =0;
    //dibujar el texto
    setPlainText("Score: "+QString::number(score));
    setDefaultTextColor(Qt::black);
    setFont(QFont("times",16));

}

void Score::increase(){
    score++;
}

int Score::getScore(){
    return score;
}