#include <QApplication>
#include <QThread>
#include "Game.h"
#include "VentanaPrincipal.h"
#include "ResourceLoader.h"
#include "UserManager.h"

Game * game;
VentanaPrincipal * ventanaPrincipal;
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

    // ===== VENTANA ÚNICA =====
    // Todas las pantallas (login, registro, menú, selector y juego) son
    // páginas de un QStackedWidget dentro de esta ventana. La app arranca
    // en el login y navega cambiando de página (sin abrir ventanas nuevas).
    ventanaPrincipal = new VentanaPrincipal();
    ventanaPrincipal->setAttribute(Qt::WA_DeleteOnClose);
    ventanaPrincipal->show();

    return a.exec();
}