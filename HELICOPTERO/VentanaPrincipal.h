#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include <QMainWindow>

class QStackedWidget;
class LoginScreen;
class RegisterScreen;
class Menu;
class LevelSelect;
class Game;

// Ventana ÚNICA del juego: todas las pantallas (login, registro, menú,
// selector de niveles y juego) son páginas de un QStackedWidget. Así no
// se abren varias ventanas: todo ocurre en una sola pantalla continua.
class VentanaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    VentanaPrincipal(QWidget *parent = nullptr);

    // Navegación entre páginas (cambia la página visible del stack)
    void mostrarLogin();
    void mostrarRegistro();
    void mostrarMenu();
    void mostrarSelector();
    void mostrarJuego(int nivel);
    void reintentarNivel();

private:
    QStackedWidget *stack;
    LoginScreen *login;
    RegisterScreen *registro;
    Menu *menu;
    LevelSelect *selector;
    Game *juego;

    // Elimina la página del juego si existe (al salir del nivel)
    void quitarJuego();
};

#endif // VENTANAPRINCIPAL_H