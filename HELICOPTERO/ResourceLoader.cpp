#include "ResourceLoader.h"

QHash<int, QImage> ResourceLoader::s_backgrounds;
QHash<int, QVector<QImage>> ResourceLoader::s_survivorFrames;
QMutex ResourceLoader::s_mutex;

ResourceLoader::ResourceLoader(QObject *parent) : QObject(parent) {}

void ResourceLoader::preloadAll(){
    // Este slot corre en el hilo de trabajo (QThread).
    // Cargar imágenes es lento (disco + decodificación): hacerlo aquí evita
    // que la UI se congele cuando el jugador abre un nivel.
    for(int nivel = 1; nivel <= 3; nivel++){
        QString rutaFondo;
        QString baseSurvivor;
        switch(nivel){
        case 2:
            rutaFondo = ":/Sprites/recursosh/nivel2_desierto_800x600.png";
            baseSurvivor = "superviviente_desierto_frame";
            break;
        case 3:
            rutaFondo = ":/Sprites/recursosh/nivel3_nieve_800x600.png";
            baseSurvivor = "superviviente_nieve_frame";
            break;
        default:
            rutaFondo = ":/Sprites/recursosh/fondo_ciudad_cyberpunk_800x600.png";
            baseSurvivor = "superviviente_frame";
        }

        // QImage es seguro de crear en cualquier hilo (a diferencia de
        // QPixmap, que solo debe usarse en el hilo de la UI).
        QImage fondo(rutaFondo);
        QVector<QImage> frames;
        frames.append(QImage(":/Sprites/recursosh/" + baseSurvivor + "1.png"));
        frames.append(QImage(":/Sprites/recursosh/" + baseSurvivor + "2.png"));

        // Escribir en la caché compartida (protegida por el mutex para que
        // el hilo de la UI no lea a medias).
        QMutexLocker locker(&s_mutex);
        s_backgrounds.insert(nivel, fondo);
        s_survivorFrames.insert(nivel, frames);
    }

    emit preloadFinished();
}

QPixmap ResourceLoader::background(int nivel){
    QMutexLocker locker(&s_mutex);
    QImage img = s_backgrounds.value(nivel);
    if(img.isNull()){
        return QPixmap();
    }
    // La conversión a QPixmap se hace AQUÍ (hilo de la UI), que es donde
    // QPixmap es seguro.
    return QPixmap::fromImage(img);
}

QVector<QPixmap> ResourceLoader::survivorFrames(int nivel){
    QMutexLocker locker(&s_mutex);
    QVector<QImage> imgs = s_survivorFrames.value(nivel);
    QVector<QPixmap> pixmaps;
    for(int i = 0; i < imgs.size(); i++){
        pixmaps.append(QPixmap::fromImage(imgs[i]));
    }
    return pixmaps;
}