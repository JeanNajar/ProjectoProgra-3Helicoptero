#ifndef ADMINMUSICA_H
#define ADMINMUSICA_H

#include <QObject>
#include <QString>

class QMediaPlayer;
class QAudioOutput;

// Musica de fondo (singleton): cada pantalla tiene SU tema (login/menu ->
// "Main Theme.mp3"/"Menu.mp3"; niveles -> "Level 1/2/3.mp3"). Al entrar a
// un nivel se detiene y al salir se reanuda. Se pide con instancia().
class AdminMusica : public QObject {
    Q_OBJECT

public:
    // Único punto de acceso (se crea la primera vez que se usa).
    static AdminMusica * instancia();

    // Silencia o reactiva TODA la musica; el estado persiste entre pantallas
    void setMute(bool silenciado);

    // true si la música está silenciada.
    bool estaSilenciado() const;

    // Reproductor (bucle) para el login y el registro.
    void reproducirLogin();

    // Reproductor (bucle) para el menú, el selector y los puntajes.
    void reproducirMenu();

    // Tema del nivel indicado (1, 2 o 3). Se reproduce en bucle.
    void reproducirNivel(int nivel);

    // Detiene la música (se llama al entrar a un nivel, que tiene su tema).
    void detener();

signals:
    // Avisa del cambio de mute (los botones de las pantallas se actualizan solos)
    void muteCambio(bool silenciado);

private:
    // Constructor privado: solo instancia() lo crea.
    AdminMusica(QObject *parent = nullptr);

    // Reproduce el archivo indicado en bucle (evita reiniciar el mismo tema).
    void reproducir(const QString &ruta);

    static AdminMusica * sInstancia;

    QMediaPlayer * reproductor;
    QAudioOutput * salida;
    QString temaActual;   // evita reiniciar el mismo tema al repetir llamadas
    bool silenciado;      // estado global del mute (música silenciada o no)
};

#endif // ADMINMUSICA_H
