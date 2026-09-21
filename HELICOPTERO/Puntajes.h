#ifndef PUNTAJES_H
#define PUNTAJES_H

#include <QWidget>
#include <QPixmap>

class QPushButton;

// Puntajes y logros: mejor nota (letra) por nivel y el logro dorado
// "Todos los A" si esta desbloqueado (QSettings, clave logro_nivel_N).
class Puntajes : public QWidget {
    Q_OBJECT

public:
    Puntajes(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void volver();

private:
    QPixmap fondo;
};

#endif // PUNTAJES_H