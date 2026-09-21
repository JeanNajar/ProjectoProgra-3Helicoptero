#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QHash>
#include <QMutex>
#include <QVector>

// Carga recursos pesados (fondos/sprites) en un hilo de trabajo para no
// congelar la UI: preloadAll() corre en un QThread (main.cpp), carga QImage
// a una cache estatica con QMutex; el juego consulta background()/
// survivorFrames() desde el hilo de UI y ahi se convierten a QPixmap.
class ResourceLoader : public QObject {
    Q_OBJECT

public:
    explicit ResourceLoader(QObject *parent = nullptr);

    // Consultas desde el hilo de la UI (protegidas con mutex); devuelven
    // un QPixmap vacio si el hilo aun no termino de cargar
    static QPixmap background(int nivel);
    static QVector<QPixmap> survivorFrames(int nivel);

public slots:
    // Corre en el hilo de trabajo: carga TODO y emite preloadFinished()
    void preloadAll();

signals:
    void preloadFinished();

private:
    static QHash<int, QImage> s_backgrounds;
    static QHash<int, QVector<QImage>> s_survivorFrames;
    static QMutex s_mutex;
};

#endif // RESOURCELOADER_H