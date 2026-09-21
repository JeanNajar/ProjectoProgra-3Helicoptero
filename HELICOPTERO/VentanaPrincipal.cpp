#include "VentanaPrincipal.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "Menu.h"
#include "LevelSelect.h"
#include "Puntajes.h"
#include "Game.h"
#include "AdminMusica.h"

#include <QStackedWidget>

//puntero global del juego (definido en main.cpp)
extern Game * game;

VentanaPrincipal::VentanaPrincipal(QWidget *parent) : QMainWindow(parent) {

    setWindowTitle("Helicopter Rescue");

    // La ventana usa el tamaño nativo del juego (800x600). Las demás
    // pantallas estiran su fondo para llenar toda la ventana.
    resize(800, 600);
    setMinimumSize(640, 480);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // Páginas fijas: se crean UNA sola vez y se reutilizan al navegar
    login = new LoginScreen();
    registro = new RegisterScreen();
    menu = new Menu();
    selector = new LevelSelect();
    puntajes = new Puntajes();

    stack->addWidget(login);     // índice 0
    stack->addWidget(registro);  // índice 1
    stack->addWidget(menu);      // índice 2
    stack->addWidget(selector);  // índice 3
    stack->addWidget(puntajes);  // índice 4

    juego = nullptr;

    // La app arranca en el login
    stack->setCurrentWidget(login);
}

void VentanaPrincipal::quitarJuego(){
    if(juego != nullptr){
        // Resetear el juego en su lugar (sin crear objetos nuevos).
        // Esto cumple el requisito de "todo en un solo frame".
        juego->reset(juego->nivelActual);
        // El juego ya no se ve: pausarlo por completo (timers + música).
        // reset() los reinicia, así que hay que pararlos después.
        juego->pausar();
    }
}

void VentanaPrincipal::mostrarLogin(){
    quitarJuego();
    // Tema del login/registro (Main Theme).
    AdminMusica::instancia()->reproducirLogin();
    stack->setCurrentWidget(login);
}

void VentanaPrincipal::mostrarRegistro(){
    // El registro usa el mismo tema que el login.
    AdminMusica::instancia()->reproducirLogin();
    stack->setCurrentWidget(registro);
}

void VentanaPrincipal::mostrarMenu(){
    quitarJuego();
    // Tema del menú (se usa en menú, selector y puntajes).
    AdminMusica::instancia()->reproducirMenu();
    stack->setCurrentWidget(menu);
}

void VentanaPrincipal::mostrarSelector(){
    quitarJuego();

    // Recrear el selector para que lea el progreso ACTUALIZADO del usuario
    // (las cards de niveles desbloqueados cambian al completar un nivel).
    stack->removeWidget(selector);
    delete selector;
    selector = new LevelSelect();
    stack->addWidget(selector);

    stack->setCurrentWidget(selector);
}

void VentanaPrincipal::mostrarPuntajes(){
    quitarJuego();

    // Recrear la pantalla para que lea el progreso ACTUALIZADO del usuario
    // (las notas de cada nivel cambian al completar un nivel).
    stack->removeWidget(puntajes);
    delete puntajes;
    puntajes = new Puntajes();
    stack->addWidget(puntajes);

    stack->setCurrentWidget(puntajes);
}

void VentanaPrincipal::mostrarJuego(int nivel){
    if(juego == nullptr){
        juego = new Game(nivel);
        game = juego;
        stack->addWidget(juego);
    }
    juego->reset(nivel);
    stack->setCurrentWidget(juego);
}

void VentanaPrincipal::reintentarNivel(){
    if(juego != nullptr){
        int nivel = juego->nivelActual;
        juego->reset(nivel);
        stack->setCurrentWidget(juego);
    }
}