#include "Score.h"
#include <QFont>

Score::Score(QGraphicsItem *parent): QGraphicsTextItem(parent){
    //empieze en 0
    score =0;
    // Fuente más pequeña para que no se sobreponga con la barra de tiempo
    // (antes 24: "Rescates: 15/15" llegaba hasta la barra).
    setFont(QFont("Arial", 18, QFont::Bold));
    setDefaultTextColor(Qt::yellow);
    setPlainText("Rescates: 0/0");
}

void Score::increase(){
    score++;
}

void Score::sumarPuntos(int puntos){
    score += puntos;
}

int Score::getScore(){
    return score;
}

void Score::mostrarRescates(int rescatados, int objetivo){
    setPlainText(QString("Rescates: %1/%2").arg(rescatados).arg(objetivo));
}