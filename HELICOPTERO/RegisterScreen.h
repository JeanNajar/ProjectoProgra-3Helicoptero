#ifndef REGISTERSCREEN_H
#define REGISTERSCREEN_H

#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QPushButton;
class LoginScreen;

// Pantalla de registro de cuenta: usuario + contraseña + confirmar.
// Crea la cuenta vía UserManager y vuelve al LoginScreen.
class RegisterScreen : public QWidget {
    Q_OBJECT

public:
    RegisterScreen(LoginScreen *loginScreen, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void registrarse();
    void volver();

private:
    LoginScreen *loginScreen;   // para volver a mostrar el login
    QPixmap fondo;
    QLineEdit *campoUsuario;
    QLineEdit *campoContrasena;
    QLineEdit *campoConfirmar;
    QPushButton* crearBotonImagen(const QString &rutaImagen, int w, int h);
};

#endif // REGISTERSCREEN_H