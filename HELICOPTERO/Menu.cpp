#include "Menu.h"
#include "Game.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

//puntero global del juego (definido en main.cpp)
extern Game * game;

Menu::Menu(QWidget *parent) : QWidget(parent) {

    setWindowTitle("Helicopter Rescue");
    setFixedSize(400, 320);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titulo = new QLabel("HELICOPTER RESCUE");
    titulo->setAlignment(Qt::AlignCenter);
    layout->addWidget(titulo);

    QPushButton *btnJugar = new QPushButton("Jugar");
    QPushButton *btnNivel = new QPushButton("Seleccionar Nivel");
    QPushButton *btnPuntajes = new QPushButton("Puntajes");
    QPushButton *btnAyuda = new QPushButton("Como Jugar");
    QPushButton *btnSalir = new QPushButton("Salir");

    layout->addWidget(btnJugar);
    layout->addWidget(btnNivel);
    layout->addWidget(btnPuntajes);
    layout->addWidget(btnAyuda);
    layout->addWidget(btnSalir);

    connect(btnJugar, &QPushButton::clicked, this, &Menu::jugar);
    connect(btnNivel, &QPushButton::clicked, this, &Menu::seleccionarNivel);
    connect(btnPuntajes, &QPushButton::clicked, this, &Menu::puntajes);
    connect(btnAyuda, &QPushButton::clicked, this, &Menu::comoJugar);
    connect(btnSalir, &QPushButton::clicked, this, &Menu::salir);
}

void Menu::jugar(){
    game = new Game();
    game->setAttribute(Qt::WA_DeleteOnClose);
    game->show();
    this->close();
}

void Menu::seleccionarNivel(){
    QMessageBox::information(this, "Seleccionar Nivel", "Disponible proximamente.");
}

void Menu::puntajes(){
    QMessageBox::information(this, "Puntajes", "Disponible proximamente.");
}

void Menu::comoJugar(){
    QMessageBox::information(this, "Como Jugar",
        "Flecha arriba / Espacio: subir\n"
        "Flecha izquierda / derecha: moverse\n"
        "Evita los obstaculos y aterriza en la zona verde.");
}

void Menu::salir(){
    this->close();
}