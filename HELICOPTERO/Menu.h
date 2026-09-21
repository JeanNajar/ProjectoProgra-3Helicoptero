#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QPixmap>

class QPushButton;

class Menu : public QWidget {
    Q_OBJECT

public:
    Menu(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void jugar();
    void puntajes();
    void ranking();
    void comoJugar();
    void salir();

private:
    QPixmap fondo;
    QPushButton* crearBotonImagen(const QString &rutaImagen, int w, int h);
};

#endif // MENU_H