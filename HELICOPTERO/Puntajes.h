#ifndef PUNTAJES_H
#define PUNTAJES_H

#include <QWidget>
#include <QPixmap>

class QPushButton;

// Pantalla de puntajes y logros: muestra la mejor nota (letra) obtenida
// en cada nivel y el logro dorado "Todos los A" si está desbloqueado.
// Las notas las guarda Game::guardarLogro en QSettings (clave logro_nivel_N).
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