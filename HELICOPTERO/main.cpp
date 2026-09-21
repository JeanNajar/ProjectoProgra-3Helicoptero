#include <QApplication>
#include <QThread>
#include "Game.h"
#include "VentanaPrincipal.h"
#include "UserManager.h"
#include "ResourceLoader.h"

Game * game;
VentanaPrincipal * ventanaPrincipal;
UserManager * userManager;
QString usuarioActual;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Hilo de precarga de recursos (sprites, sonidos): evita congelar la UI
    QThread hiloPrecarga;
    ResourceLoader loader;
    loader.moveToThread(&hiloPrecarga);
    QObject::connect(&hiloPrecarga, &QThread::started, &loader, &ResourceLoader::preloadAll);
    hiloPrecarga.start();

    userManager = new UserManager();
    userManager->cargarUsuarios();

    VentanaPrincipal w;
    ventanaPrincipal = &w;
    w.show();

    return a.exec();
}