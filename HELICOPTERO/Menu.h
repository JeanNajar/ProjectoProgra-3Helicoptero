#ifndef MENU_H
#define MENU_H

#include <QWidget>

class Menu : public QWidget {
    Q_OBJECT

public:
    Menu(QWidget *parent = nullptr);

private slots:
    void jugar();
    void seleccionarNivel();
    void puntajes();
    void comoJugar();
    void salir();
};

#endif // MENU_H