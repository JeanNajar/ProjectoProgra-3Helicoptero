#include <QApplication>
#include "Game.h"
#include "Menu.h"

Game * game;
Menu * menu;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    menu = new Menu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->show();

    return a.exec();
}