#include "Menu.h"
#include "VentanaPrincipal.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QIcon>

//puntero global de la ventana única (definido en main.cpp)
extern VentanaPrincipal * ventanaPrincipal;

Menu::Menu(QWidget *parent) : QWidget(parent) {

    // Es una página de la ventana única: el tamaño lo define la ventana
    // (800x600). El fondo se estira para llenar toda la pantalla.

    fondo = QPixmap(":/Sprites/recursosh/menu_fondo_v2_640x880.png");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40,50,40,40);
    layout->setSpacing(18);

    QLabel *titulo = new QLabel("HELICOPTER RESCUE");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setAttribute(Qt::WA_StyledBackground, true);
    titulo->setStyleSheet(
        "QLabel {"
        "  color: #4de8ff;"
        "  font-family: 'Consolas';"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "  letter-spacing: 3px;"
        "  background-color: rgba(15, 10, 25, 160);"
        "  border: 1px solid #4de8ff;"
        "  border-radius: 4px;"
        "  padding: 12px;"
        "}"
        );
    layout->addWidget(titulo);
    layout->addSpacing(20);

    int btnW = 380, btnH = 52;

    QPushButton *btnJugar = crearBotonImagen(":/Sprites/recursosh/boton_jugar.png", btnW, btnH);
    QPushButton *btnPuntajes = crearBotonImagen(":/Sprites/recursosh/boton_puntajes.png", btnW, btnH);
    QPushButton *btnAyuda = crearBotonImagen(":/Sprites/recursosh/boton_como_jugar.png", btnW, btnH);
    QPushButton *btnSalir = crearBotonImagen(":/Sprites/recursosh/boton_salir.png", btnW, btnH);

    layout->addWidget(btnJugar, 0, Qt::AlignHCenter);
    layout->addWidget(btnPuntajes, 0, Qt::AlignHCenter);
    layout->addWidget(btnAyuda, 0, Qt::AlignHCenter);
    layout->addWidget(btnSalir, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(btnJugar, &QPushButton::clicked, this, &Menu::jugar);
    connect(btnPuntajes, &QPushButton::clicked, this, &Menu::puntajes);
    connect(btnAyuda, &QPushButton::clicked, this, &Menu::comoJugar);
    connect(btnSalir, &QPushButton::clicked, this, &Menu::salir);
}

void Menu::paintEvent(QPaintEvent *event)
{
    // Fondo estirado a TODA la ventana (sin barras laterales ni franjas)
    QPainter painter(this);
    painter.fillRect(rect(), QColor(21, 10, 43));
    painter.drawPixmap(rect(), fondo);

    QWidget::paintEvent(event);
}

QPushButton* Menu::crearBotonImagen(const QString &rutaImagen, int w, int h)
{
    QPushButton *boton = new QPushButton(this);
    boton->setIcon(QIcon(rutaImagen));
    boton->setIconSize(QSize(w, h));
    boton->setFixedSize(w, h);
    boton->setFlat(true);
    boton->setCursor(Qt::PointingHandCursor);
    boton->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { background: rgba(255, 255, 255, 25); border-radius: 4px; }"
        "QPushButton:pressed { background: rgba(0,0,0,60); border-radius: 4px; }"
        );
    return boton;
}

void Menu::jugar(){
    // Al darle PLAY se abre el selector de niveles: el 1 siempre disponible
    // y el 2 y 3 bloqueados hasta completar el nivel anterior.
    // Cambia de página en la ventana única (no abre una ventana nueva).
    ventanaPrincipal->mostrarSelector();
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
    // Cerrar la ventana única (cierra toda la aplicación)
    ventanaPrincipal->close();
}