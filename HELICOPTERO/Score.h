#ifndef SCORE_H
#define SCORE_H

#include <QGraphicsTextItem>

class Score: public QGraphicsTextItem{
public:
    Score(QGraphicsItem * parent=0);
    void increase();
    int getScore();
    void mostrarRescates(int rescatados, int objetivo);
private:
    int score;
};

#endif // SCORE_H
