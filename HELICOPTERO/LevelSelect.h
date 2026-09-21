#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include <QWidget>
#include <QPixmap>

class QPushButton;

// Seleccion de niveles con cards (280x180); 2 y 3 bloqueados hasta
// completar el anterior (progreso en QSettings). Las cards bloqueadas no se pulsan.
class LevelSelect : public QWidget {
    Q_OBJECT

public:
    LevelSelect(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void jugarNivel(int nivel);
    void volver();

private:
    QPixmap fondo;
    QPushButton* crearCardNivel(int nivel, bool desbloqueado);
};

#endif // LEVELSELECT_H