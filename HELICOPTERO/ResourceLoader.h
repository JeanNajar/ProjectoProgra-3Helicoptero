#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QHash>
#include <QMutex>
#include <QVector>

// Carga los recursos pesados (fondos y sprites de los niveles) en un hilo
// de trabajo para no congelar la interfaz al abrir un nivel.
//
// Cómo funciona:
//  - El slot preloadAll() corre en un QThread aparte (ver main.cpp).
//  - En el hilo se cargan QImage (seguras para crear fuera del hilo de UI).
//  - Quedan en una caché estática protegida con QMutex.
//  - El juego consulta background()/survivorFrames() desde el hilo de la UI;
//    ahí se convierten a QPixmap (que solo se debe usar en el hilo de UI).
class ResourceLoader : public QObject {
    Q_OBJECT

public:
    explicit ResourceLoader(QObject *parent = nullptr);

    // Consultas desde el hilo de la UI (protegidas con mutex).
    // Devuelven un QPixmap vacío si el hilo aún no terminó de cargar.
    static QPixmap background(int nivel);
    static QVector<QPixmap> survivorFrames(int nivel);

public slots:
    // Corre en el hilo de trabajo: carga TODO y emite preloadFinished().
    void preloadAll();

signals:
    void preloadFinished();

private:
    static QHash<int, QImage> s_backgrounds;
    static QHash<int, QVector<QImage>> s_survivorFrames;
    static QMutex s_mutex;
};

#endif // RESOURCELOADER_H