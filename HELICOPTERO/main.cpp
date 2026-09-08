#include <QApplication>
#include "Game.h"
#include "Menu.h"

Game * game;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Mostrar el menu principal
    Menu menu;
    menu.show();

    return a.exec();
}