#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QPushButton;

// Pantalla de login: aparece ANTES del menu; valida contra UserManager
// (usuarios.dat). Es una pagina de la ventana unica (VentanaPrincipal).
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