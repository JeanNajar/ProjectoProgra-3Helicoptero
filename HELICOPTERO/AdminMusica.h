#ifndef ADMINMUSICA_H
#define ADMINMUSICA_H

#include <QObject>

class QMediaPlayer;
class QAudioOutput;

// Música de fondo del juego (singleton).
//
// Cuando el usuario navega por la app, cada pantalla debe tener SU tema:
//   - Login / Registro          -> "Main Theme.mp3"
//   - Menú / Selector / Puntajes -> "Menu.mp3"
//   - Nivel 1/2/3               -> "Level 1.mp3" / "Level 2.mp3" / "Level 3.mp3"
//
// Esta clase centraliza la música de las pantallas NO jugables. El nivel
// (Game) reproduce su propio tema con su reproductor, así que al entrar a
// un nivel se detiene AdminMusica y al salir se reanuda la pantalla activa.
//
// Es un singleton: se pide con AdminMusica::instancia().
class AdminMusica : public QObject {
    Q_OBJECT

public:
    // Único punto de acceso (se crea la primera vez que se usa).
    static AdminMusica * instancia();

    // Reproductor (bucle) para el login y el registro.
    void reproducirLogin();

    // Reproductor (bucle) para el menú, el selector y los puntajes.
    void reproducirMenu();

    // Detiene la música (se llama al entrar a un nivel, que tiene su tema).
    void detener();

private:
    // Constructor privado: solo instancia() lo crea.
    AdminMusica();

    // Reproduce el archivo indicado en bucle (espacios codificados).
    void reproducir(const QString &ruta);

    QMediaPlayer * reproductor;
    QAudioOutput * salida;
    QString ultimaRuta;   // evita reiniciar el mismo tema al repetir llamadas
};

#endif // ADMINMUSICA_H
