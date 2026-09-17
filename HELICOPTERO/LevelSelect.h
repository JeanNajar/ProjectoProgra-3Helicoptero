#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include <QWidget>
#include <QPixmap>

class QPushButton;

// Menú de selección de niveles con cards de imagen (280x180).
// Los niveles 2 y 3 aparecen BLOQUEADOS hasta que se complete el nivel
// anterior (progreso guardado en QSettings). Las cards bloqueadas no se
// pueden pulsar.
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