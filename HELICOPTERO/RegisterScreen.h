#ifndef REGISTERSCREEN_H
#define REGISTERSCREEN_H

#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QPushButton;

// Pantalla de registro: usuario + contrasena + confirmar; crea la cuenta
// via UserManager y vuelve al LoginScreen (pagina de la ventana unica).
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