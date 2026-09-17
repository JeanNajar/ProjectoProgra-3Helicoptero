#include "VentanaPrincipal.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "Menu.h"
#include "LevelSelect.h"
#include "Game.h"

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

    stack->addWidget(login);     // índice 0
    stack->addWidget(registro);  // índice 1
    stack->addWidget(menu);      // índice 2
    stack->addWidget(selector);  // índice 3

    juego = nullptr;

    // La app arranca en el login
    stack->setCurrentWidget(login);
}

void VentanaPrincipal::quitarJuego(){
    if(juego != nullptr){
        // Pausar el juego antes de eliminarlo: sus timers (física, spawns,
        // supervivientes) consultan game->juegoTerminado y se detienen.
        juego->juegoTerminado = true;
        stack->removeWidget(juego);
        delete juego;   // inmediato: destruye timers y escena al instante
        juego = nullptr;
        game = nullptr;
    }
}

void VentanaPrincipal::mostrarLogin(){
    quitarJuego();
    stack->setCurrentWidget(login);
}

void VentanaPrincipal::mostrarRegistro(){
    stack->setCurrentWidget(registro);
}

void VentanaPrincipal::mostrarMenu(){
    quitarJuego();
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

void VentanaPrincipal::mostrarJuego(int nivel){
    quitarJuego();
    juego = new Game(nivel);
    game = juego;
    stack->addWidget(juego);
    stack->setCurrentWidget(juego);
}

void VentanaPrincipal::reintentarNivel(){
    if(juego != nullptr){
        int nivel = juego->nivelActual;
        quitarJuego();
        mostrarJuego(nivel);
    }
}