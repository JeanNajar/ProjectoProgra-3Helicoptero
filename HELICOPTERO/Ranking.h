#ifndef RANKING_H
#define RANKING_H

#include <QWidget>
#include <QPixmap>

class QPushButton;
class QTableWidget;

// Ranking: usuarios ordenados por puntajeTotal (mayor a menor); los puntos
// se acumulan al terminar cada partida. La fila del usuario logueado se resalta.
class Ranking : public QWidget {
    Q_OBJECT

public:
    Ranking(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void volver();

private:
    QPixmap fondo;
    QTableWidget *tabla;
};

#endif // RANKING_H