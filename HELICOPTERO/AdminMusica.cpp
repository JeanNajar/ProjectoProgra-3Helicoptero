#include "AdminMusica.h"

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

// Puntero estático de la instancia única.
AdminMusica * AdminMusica::sInstancia = nullptr;

AdminMusica * AdminMusica::instancia(){
    if(sInstancia == nullptr){
        sInstancia = new AdminMusica();
    }
    return sInstancia;
}

AdminMusica::AdminMusica(QObject *parent) : QObject(parent){
    reproductor = new QMediaPlayer(this);
    salida = new QAudioOutput(this);
    reproductor->setAudioOutput(salida);
    salida->setVolume(0.35);   // Música ambiental, sin tapar los efectos
    reproductor->setLoops(QMediaPlayer::Infinite);
}

void AdminMusica::reproducir(const QString &ruta){
    // Si ya suena el MISMO tema, no reiniciarlo: evita que el login
    // "revuelva" la canción cada vez que se vuelve desde el registro.
    if(ruta == temaActual){
        if(reproductor->playbackState() == QMediaPlayer::PlayingState){
            return;
        }
    }

    temaActual = ruta;
    reproductor->stop();
    reproductor->setSource(QUrl(ruta));
    reproductor->play();
}

void AdminMusica::reproducirLogin(){
    reproducir(QStringLiteral("qrc:/Sounds/recursosh/Main Theme.mp3"));
}

void AdminMusica::reproducirMenu(){
    reproducir(QStringLiteral("qrc:/Sounds/recursosh/Menu.mp3"));
}

void AdminMusica::reproducirNivel(int nivel){
    switch(nivel){
    case 1:
        reproducir(QStringLiteral("qrc:/Sounds/recursosh/Level 1.mp3"));
        break;
    case 2:
        reproducir(QStringLiteral("qrc:/Sounds/recursosh/Level 2.mp3"));
        break;
    case 3:
        reproducir(QStringLiteral("qrc:/Sounds/recursosh/Level 3.mp3"));
        break;
    default:
        reproducir(QStringLiteral("qrc:/Sounds/recursosh/Level 1.mp3"));
        break;
    }
}

void AdminMusica::detener(){
    temaActual.clear();
    reproductor->stop();
}
