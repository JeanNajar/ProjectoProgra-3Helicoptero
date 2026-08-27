#include <QApplication>
#include "Game.h"

Game * game;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Crear la instancia del juego
    game = new Game();

    // Mostrar la ventana del juego
    game->show();

    return a.exec();
}