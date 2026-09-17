#include "LoginScreen.h"
#include "UserManager.h"
#include "VentanaPrincipal.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <QMessageBox>

//punteros globales (definidos en main.cpp)
extern UserManager * userManager;
extern VentanaPrincipal * ventanaPrincipal;
extern QString usuarioActual;

LoginScreen::LoginScreen(QWidget *parent) : QWidget(parent) {

    // Es una página de la ventana única: el tamaño lo define la ventana
    // (800x600). El fondo se estira para llenar toda la pantalla.

    fondo = QPixmap(":/Sprites/recursosh/login_fondo_640x880.png");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(120, 150, 120, 60);
    layout->setSpacing(16);

    QLabel *titulo = new QLabel("INICIAR SESIÓN");
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
        "  padding: 10px;"
        "}"
        );
    layout->addWidget(titulo);
    layout->addSpacing(10);

    QString estiloCampo =
        "QLineEdit { background-color: rgba(15, 10, 25, 200); color: #ffffff;"
        " border: 1px solid #4de8ff; border-radius: 4px; padding: 10px;"
        " font-size: 16px; }"
        "QLineEdit:focus { border: 2px solid #4de8ff; }";

    campoUsuario = new QLineEdit();
    campoUsuario->setPlaceholderText("Usuario");
    campoUsuario->setStyleSheet(estiloCampo);
    layout->addWidget(campoUsuario);

    campoContrasena = new QLineEdit();
    campoContrasena->setPlaceholderText("Contraseña");
    campoContrasena->setEchoMode(QLineEdit::Password);
    campoContrasena->setStyleSheet(estiloCampo);
    layout->addWidget(campoContrasena);

    layout->addSpacing(10);

    // Botones como QIcon sin borde (igual que en el menú principal)
    QPushButton *btnIniciar = crearBotonImagen(":/Sprites/recursosh/boton_iniciar_sesion.png", 380, 52);
    QPushButton *btnCrear = crearBotonImagen(":/Sprites/recursosh/boton_crear_cuenta.png", 380, 52);
    layout->addWidget(btnIniciar, 0, Qt::AlignHCenter);
    layout->addWidget(btnCrear, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(btnIniciar, &QPushButton::clicked, this, &LoginScreen::iniciarSesion);
    connect(btnCrear, &QPushButton::clicked, this, &LoginScreen::crearCuenta);

    // Enter en el campo de contraseña también inicia sesión
    connect(campoContrasena, &QLineEdit::returnPressed, this, &LoginScreen::iniciarSesion);
}

void LoginScreen::paintEvent(QPaintEvent *event)
{
    // Fondo estirado a TODA la ventana (sin barras laterales ni franjas).
    // La imagen 640x880 se adapta a cualquier tamaño de la ventana.
    QPainter painter(this);
    painter.fillRect(rect(), QColor(21, 10, 43));
    painter.drawPixmap(rect(), fondo);

    QWidget::paintEvent(event);
}

QPushButton* LoginScreen::crearBotonImagen(const QString &rutaImagen, int w, int h)
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

void LoginScreen::iniciarSesion(){

    QString nombre = campoUsuario->text().trimmed();
    QString contrasena = campoContrasena->text();

    if(nombre.isEmpty() || contrasena.isEmpty()){
        QMessageBox::warning(this, "Iniciar Sesión", "Ingresá usuario y contraseña.");
        return;
    }

    if(userManager == nullptr || !userManager->verificarLogin(nombre, contrasena)){
        QMessageBox::warning(this, "Iniciar Sesión", "Usuario o contraseña incorrectos.");
        return;
    }

    // Login correcto: guardar el usuario logueado y pasar al menú
    // (cambia de página en la ventana única, no abre una ventana nueva)
    usuarioActual = nombre;
    ventanaPrincipal->mostrarMenu();
}

void LoginScreen::crearCuenta(){
    // Cambiar a la página de registro (dentro de la misma ventana)
    ventanaPrincipal->mostrarRegistro();
}