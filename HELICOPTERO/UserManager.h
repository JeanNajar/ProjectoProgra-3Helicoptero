#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QList>
#include <QString>

// Cuenta de un jugador. La contraseña NUNCA se guarda en texto plano:
// solo su hash SHA-256.
struct Usuario {
    QString nombreUsuario;
    QString hashContrasena;
    int mejorScore;     // mejor puntaje de UNA partida (por ahora siempre 0)
    int puntajeTotal;    // puntaje ACUMULADO de todas las partidas (ranking)
};

// Maneja las cuentas en un archivo de TEXTO PLANO "usuarios.dat"
// (una linea por usuario, formato "usuario:hash:mejorScore:puntajeTotal").
// La seguridad la da el hash, no el formato del archivo.
class UserManager : public QObject {
    Q_OBJECT

public:
    explicit UserManager(QObject *parent = nullptr);

    // Carga todos los usuarios del archivo a la lista (al iniciar la app)
    bool cargarUsuarios();
    // Guarda la lista completa al archivo
    bool guardarUsuarios();

    // Busca un usuario por nombre (nullptr si no existe)
    Usuario* buscarPorNombre(const QString &nombre);

    // Registra un usuario nuevo. Devuelve false y llena `error` si el
    // usuario ya existe o los datos no son validos.
    bool registrar(const QString &nombre, const QString &contrasena, QString &error);

    // Verifica usuario + contraseña (compara hashes)
    bool verificarLogin(const QString &nombre, const QString &contrasena);

    // Suma puntos al puntajeTotal acumulado del usuario y guarda a disco.
    // Nunca baja: se usa al terminar cada partida (victoria o derrota).
    bool sumarPuntaje(const QString &nombre, int puntos);

    // Copia de todos los usuarios (para armar el ranking). Se devuelve
    // por valor a propósito: quien arma el ranking ordena SU copia con
    // su propio bubble sort, sin tocar el orden interno de UserManager.
    QList<Usuario> obtenerTodos() const;

    // Hash SHA-256 en hexadecimal de una contraseña
    QString hashDe(const QString &contrasena) const;

    // Ruta del archivo de usuarios (directorio de la app)
    QString rutaArchivo() const;

private:
    QList<Usuario> usuarios;
};

#endif // USERMANAGER_H