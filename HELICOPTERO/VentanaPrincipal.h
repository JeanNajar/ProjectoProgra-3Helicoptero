#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include <QMainWindow>

class QStackedWidget;
class LoginScreen;
class RegisterScreen;
class Menu;
class LevelSelect;
class Puntajes;
class Ranking;
class Game;

// Ventana UNICA: todas las pantallas (login, registro, menu, selector y
// juego) son paginas de un QStackedWidget; todo ocurre en una sola ventana.
class VentanaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    VentanaPrincipal(QWidget *parent = nullptr);

    // Navegación entre páginas (cambia la página visible del stack)
    void mostrarLogin();
    void mostrarRegistro();
    void mostrarMenu();
    void mostrarSelector();
    void mostrarPuntajes();
    void mostrarRanking();
    void mostrarJuego(int nivel);
    void reintentarNivel();

private:
    QStackedWidget *stack;
    LoginScreen *login;
    RegisterScreen *registro;
    Menu *menu;
    LevelSelect *selector;
    Puntajes *puntajes;
    Ranking *ranking;
    Game *juego;

    // Resetea la página del juego en su lugar (sin crear objetos nuevos)
    void quitarJuego();
};

#endif // VENTANAPRINCIPAL_H