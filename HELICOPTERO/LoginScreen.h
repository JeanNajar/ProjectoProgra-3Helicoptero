#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QPushButton;
class RegisterScreen;

// Pantalla de inicio de sesión: aparece ANTES del menú. Pide
// usuario/contraseña y valida contra UserManager (usuarios.dat).
class LoginScreen : public QWidget {
    Q_OBJECT

public:
    LoginScreen(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void iniciarSesion();
    void crearCuenta();

private:
    QPixmap fondo;
    QLineEdit *campoUsuario;
    QLineEdit *campoContrasena;
    QPushButton* crearBotonImagen(const QString &rutaImagen, int w, int h);
};

#endif // LOGINSCREEN_H