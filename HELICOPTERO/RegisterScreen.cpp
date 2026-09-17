#include "RegisterScreen.h"
#include "UserManager.h"
#include "VentanaPrincipal.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <QMessageBox>

//puntero global (definido en main.cpp)
extern UserManager * userManager;
extern VentanaPrincipal * ventanaPrincipal;

RegisterScreen::RegisterScreen(QWidget *parent)
    : QWidget(parent)
{
    // Es una página de la ventana única: el tamaño lo define la ventana
    // (800x600). El fondo se estira para llenar toda la pantalla.

    fondo = QPixmap(":/Sprites/recursosh/login_fondo_640x880.png");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(120, 120, 120, 60);
    layout->setSpacing(16);

    QLabel *titulo = new QLabel("CREAR CUENTA");
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

    campoConfirmar = new QLineEdit();
    campoConfirmar->setPlaceholderText("Confirmar contraseña");
    campoConfirmar->setEchoMode(QLineEdit::Password);
    campoConfirmar->setStyleSheet(estiloCampo);
    layout->addWidget(campoConfirmar);

    layout->addSpacing(10);

    // Botones como QIcon sin borde (igual que en el menú principal)
    QPushButton *btnRegistrar = crearBotonImagen(":/Sprites/recursosh/boton_registrarse.png", 380, 52);
    QPushButton *btnVolver = crearBotonImagen(":/Sprites/recursosh/boton_volver.png", 380, 52);
    layout->addWidget(btnRegistrar, 0, Qt::AlignHCenter);
    layout->addWidget(btnVolver, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(btnRegistrar, &QPushButton::clicked, this, &RegisterScreen::registrarse);
    connect(btnVolver, &QPushButton::clicked, this, &RegisterScreen::volver);

    // Enter en el campo de confirmar también registra
    connect(campoConfirmar, &QLineEdit::returnPressed, this, &RegisterScreen::registrarse);
}

void RegisterScreen::paintEvent(QPaintEvent *event)
{
    // Fondo estirado a TODA la ventana (sin barras laterales ni franjas)
    QPainter painter(this);
    painter.fillRect(rect(), QColor(21, 10, 43));
    painter.drawPixmap(rect(), fondo);

    QWidget::paintEvent(event);
}

QPushButton* RegisterScreen::crearBotonImagen(const QString &rutaImagen, int w, int h)
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

void RegisterScreen::registrarse(){

    QString nombre = campoUsuario->text().trimmed();
    QString contrasena = campoContrasena->text();
    QString confirmar = campoConfirmar->text();

    if(nombre.isEmpty() || contrasena.isEmpty() || confirmar.isEmpty()){
        QMessageBox::warning(this, "Registrarse", "Completá todos los campos.");
        return;
    }
    if(contrasena != confirmar){
        QMessageBox::warning(this, "Registrarse", "Las contraseñas no coinciden.");
        return;
    }

    QString error;
    if(userManager == nullptr || !userManager->registrar(nombre, contrasena, error)){
        QMessageBox::warning(this, "Registrarse",
                             error.isEmpty() ? "No se pudo crear la cuenta." : error);
        return;
    }

    QMessageBox::information(this, "Registrarse", "Cuenta creada. Ya podés iniciar sesión.");
    volver();
}

void RegisterScreen::volver(){
    // Volver al login (cambia de página en la ventana única)
    ventanaPrincipal->mostrarLogin();
}