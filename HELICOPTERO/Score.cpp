#include "Score.h"
#include <QFont>

Score::Score(QGraphicsItem *parent): QGraphicsTextItem(parent){
    //empieze en 0
    score =0;
    // Usar fuente más grande para mayor visibilidad
    setFont(QFont("Arial", 22, QFont::Bold));
    setDefaultTextColor(Qt::yellow);
    setPlainText("Score: 0");
}

void Score::increase(){
    score++;
}

int Score::getScore(){
    return score;
}

void Score::mostrarRescates(int rescatados, int objetivo){
    setPlainText(QString("Rescates: %1/%2").arg(rescatados).arg(objetivo));
}