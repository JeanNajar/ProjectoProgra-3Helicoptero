#include "UserManager.h"

#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

UserManager::UserManager(QObject *parent) : QObject(parent) {}

QString UserManager::rutaArchivo() const{
    // El archivo vive en el directorio de la aplicacion
    return QCoreApplication::applicationDirPath() + "/usuarios.dat";
}

QString UserManager::hashDe(const QString &contrasena) const{
    // SHA-256 en hexadecimal: nunca se guarda la contraseña en texto plano
    return QString(QCryptographicHash::hash(contrasena.toUtf8(),
                                            QCryptographicHash::Sha256).toHex());
}

bool UserManager::cargarUsuarios(){

    usuarios.clear();

    QFile archivo(rutaArchivo());
    if(!archivo.exists()){
        return true;  // primera vez: todavia no hay usuarios
    }
    if(!archivo.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }

    QTextStream in(&archivo);
    while(!in.atEnd()){
        QString linea = in.readLine().trimmed();
        if(linea.isEmpty()){
            continue;
        }
        // Formato: usuario:hash:mejorScore:puntajeTotal
        QStringList partes = linea.split(':');
        if(partes.size() < 2){
            continue;  // linea corrupta, se ignora
        }
        Usuario u;
        u.nombreUsuario = partes[0];
        u.hashContrasena = partes[1];
        u.mejorScore = (partes.size() >= 3) ? partes[2].toInt() : 0;
        // puntajeTotal es el campo NUEVO: si el archivo es viejo (cuentas
        // creadas antes de este cambio) no va a existir, se asume 0.
        u.puntajeTotal = (partes.size() >= 4) ? partes[3].toInt() : 0;
        usuarios.append(u);
    }
    archivo.close();
    return true;
}

bool UserManager::guardarUsuarios(){

    QFile archivo(rutaArchivo());
    if(!archivo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        return false;
    }

    QTextStream out(&archivo);
    for(const Usuario &u : usuarios){
        out << u.nombreUsuario << ':' << u.hashContrasena << ':'
            << u.mejorScore << ':' << u.puntajeTotal << '\n';
    }
    archivo.close();
    return true;
}

Usuario* UserManager::buscarPorNombre(const QString &nombre){

    for(int i = 0; i < usuarios.size(); i++){
        if(usuarios[i].nombreUsuario == nombre){
            return &usuarios[i];
        }
    }
    return nullptr;
}

bool UserManager::registrar(const QString &nombre, const QString &contrasena, QString &error){

    if(nombre.trimmed().isEmpty()){
        error = "El usuario no puede estar vacio.";
        return false;
    }
    if(nombre.contains(':')){
        error = "El usuario no puede contener ':'.";
        return false;
    }
    if(contrasena.isEmpty()){
        error = "La contraseña no puede estar vacia.";
        return false;
    }
    if(buscarPorNombre(nombre) != nullptr){
        error = "El usuario ya existe.";
        return false;
    }

    Usuario u;
    u.nombreUsuario = nombre;
    u.hashContrasena = hashDe(contrasena);
    u.mejorScore = 0;
    u.puntajeTotal = 0;
    usuarios.append(u);

    if(!guardarUsuarios()){
        error = "No se pudo guardar el archivo de usuarios.";
        return false;
    }
    return true;
}

bool UserManager::verificarLogin(const QString &nombre, const QString &contrasena){

    Usuario *u = buscarPorNombre(nombre);
    if(u == nullptr){
        return false;
    }
    // Comparar el hash de lo que escribio con el hash guardado
    return u->hashContrasena == hashDe(contrasena);
}

bool UserManager::sumarPuntaje(const QString &nombre, int puntos){

    Usuario *u = buscarPorNombre(nombre);
    if(u == nullptr){
        return false;
    }
    u->puntajeTotal += puntos;
    return guardarUsuarios();
}

QList<Usuario> UserManager::obtenerTodos() const{
    return usuarios;
}