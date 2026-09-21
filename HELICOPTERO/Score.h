#ifndef SCORE_H
#define SCORE_H

#include <QGraphicsTextItem>

class Score: public QGraphicsTextItem{
public:
    Score(QGraphicsItem * parent=0);
    void increase();
    // Suma un puntaje arbitrario (p. ej. +5 por rescate u obstáculo
    // destruido) y actualiza el texto del HUD inmediatamente.
    void sumarPuntos(int puntos);
    int getScore();
    void mostrarRescates(int rescatados, int objetivo);
private:
    int score;
};

#endif // SCORE_H
