#include "Menu.h"
#include "VentanaPrincipal.h"
#include "AdminMusica.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
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
    QPushButton *btnRanking = crearBotonImagen(":/Sprites/recursosh/boton_ranking.png", btnW, btnH);
    QPushButton *btnAyuda = crearBotonImagen(":/Sprites/recursosh/boton_como_jugar.png", btnW, btnH);
    QPushButton *btnSalir = crearBotonImagen(":/Sprites/recursosh/boton_salir.png", btnW, btnH);

    layout->addWidget(btnJugar, 0, Qt::AlignHCenter);
    layout->addWidget(btnPuntajes, 0, Qt::AlignHCenter);
    layout->addWidget(btnRanking, 0, Qt::AlignHCenter);
    layout->addWidget(btnAyuda, 0, Qt::AlignHCenter);
    layout->addWidget(btnSalir, 0, Qt::AlignHCenter);
    layout->addStretch();

    // Botón de mute abajo a la derecha: silencia/reactiva la música.
    // El símbolo cambia según el estado (🔊 sonido / 🔇 silenciado).
    QPushButton *btnMute = new QPushButton();
    btnMute->setFixedSize(48, 48);
    btnMute->setCursor(Qt::PointingHandCursor);
    btnMute->setToolTip("Silenciar / Activar sonido");
    btnMute->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #4de8ff;"
        " border: 2px solid #4de8ff; border-radius: 6px;"
        " font-size: 20px; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    btnMute->setText(AdminMusica::instancia()->estaSilenciado()
        ? QString::fromUtf8("\xF0\x9F\x94\x87")   // 🔇
        : QString::fromUtf8("\xF0\x9F\x94\x8A")); // 🔊
    connect(btnMute, &QPushButton::clicked, this, [this]() {
        AdminMusica::instancia()->setMute(!AdminMusica::instancia()->estaSilenciado());
    });
    // Mantener el símbolo al día aunque el mute cambie desde otra pantalla
    connect(AdminMusica::instancia(), &AdminMusica::muteCambio, this,
            [btnMute](bool silenciado) {
        btnMute->setText(silenciado
            ? QString::fromUtf8("\xF0\x9F\x94\x87")
            : QString::fromUtf8("\xF0\x9F\x94\x8A"));
    });

    QHBoxLayout *filaMute = new QHBoxLayout();
    filaMute->addStretch();
    filaMute->addWidget(btnMute);
    layout->addLayout(filaMute);

    connect(btnJugar, &QPushButton::clicked, this, &Menu::jugar);
    connect(btnPuntajes, &QPushButton::clicked, this, &Menu::puntajes);
    connect(btnRanking, &QPushButton::clicked, this, &Menu::ranking);
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
    // PLAY abre el selector: nivel 1 disponible, 2 y 3 bloqueados hasta completar
    ventanaPrincipal->mostrarSelector();
}

void Menu::puntajes(){
    // Abrir la pantalla de puntajes y logros (página de la ventana única)
    ventanaPrincipal->mostrarPuntajes();
}

void Menu::ranking(){
    // Abrir la pantalla de ranking (página de la ventana única)
    ventanaPrincipal->mostrarRanking();
}

void Menu::comoJugar(){
    // Mostrar la imagen del manual de usuario en lugar de un MessageBox.
    QDialog dialog(this);
    dialog.setWindowTitle("Cómo Jugar");
    dialog.setFixedSize(800, 600);

    QLabel *label = new QLabel(&dialog);
    label->setPixmap(QPixmap(":/Sprites/recursosh/ManualDeUsuario.jpeg")
                         .scaled(800, 600, Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation));
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(label);
    dialog.setLayout(layout);

    dialog.exec();
}

void Menu::salir(){
    // Cerrar la ventana única (cierra toda la aplicación)
    ventanaPrincipal->close();
}