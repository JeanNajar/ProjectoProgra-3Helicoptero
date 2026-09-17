#ifndef REGISTERSCREEN_H
#define REGISTERSCREEN_H

#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QPushButton;

// Pantalla de registro de cuenta: usuario + contraseña + confirmar.
// Crea la cuenta vía UserManager y vuelve al LoginScreen.
// Es una página de la ventana única (VentanaPrincipal).
class RegisterScreen : public QWidget {
    Q_OBJECT

public:
    RegisterScreen(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void registrarse();
    void volver();

private:
    QPixmap fondo;
    QLineEdit *campoUsuario;
    QLineEdit *campoContrasena;
    QLineEdit *campoConfirmar;
    QPushButton* crearBotonImagen(const QString &rutaImagen, int w, int h);
};

#endif // REGISTERSCREEN_H