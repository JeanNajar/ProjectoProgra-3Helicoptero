#include <QApplication>
#include <QThread>
#include "Game.h"
#include "Menu.h"
#include "ResourceLoader.h"
#include "UserManager.h"
#include "LoginScreen.h"

Game * game;
Menu * menu;
UserManager * userManager;
QString usuarioActual;   // usuario que inició sesión (se llena en LoginScreen)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ===== HILO DE PRECARGA DE RECURSOS =====
    // Carga los fondos y sprites de los 3 niveles en un hilo aparte para
    // que el menú no se congele y los niveles abran al instante.
    // Patrón Qt: QThread + moveToThread + conexión started -> preloadAll().
    QThread *hiloRecursos = new QThread;
    ResourceLoader *loader = new ResourceLoader;
    loader->moveToThread(hiloRecursos);

    // Al arrancar el hilo se ejecuta la carga (en el hilo de trabajo)
    QObject::connect(hiloRecursos, &QThread::started,
                     loader, &ResourceLoader::preloadAll);
    // Al terminar: el hilo se detiene y ambos objetos se liberan solos
    QObject::connect(loader, &ResourceLoader::preloadFinished,
                     hiloRecursos, &QThread::quit);
    QObject::connect(loader, &ResourceLoader::preloadFinished,
                     loader, &ResourceLoader::deleteLater);
    QObject::connect(hiloRecursos, &QThread::finished,
                     hiloRecursos, &QThread::deleteLater);
    hiloRecursos->start();

    // ===== SISTEMA DE CUENTAS =====
    // Cargar los usuarios guardados (usuarios.dat) al iniciar la app.
    userManager = new UserManager();
    userManager->cargarUsuarios();

    // La app arranca en la pantalla de LOGIN (antes del menú).
    // Al iniciar sesión correctamente se crea y muestra el Menu.
    LoginScreen *login = new LoginScreen();
    login->setAttribute(Qt::WA_DeleteOnClose);
    login->show();

    return a.exec();
}