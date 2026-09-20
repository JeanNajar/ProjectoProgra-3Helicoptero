#include "Puntajes.h"
#include "VentanaPrincipal.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QSettings>

//puntero global de la ventana única (definido en main.cpp)
extern VentanaPrincipal * ventanaPrincipal;

// Color del borde según el nivel (igual que el popup de victoria)
static QColor colorNivel(int nivel){
    switch(nivel){
    case 1: return QColor(77, 232, 255);   // cian (ciudad)
    case 2: return QColor(255, 149, 0);    // naranja (desierto)
    case 3: return QColor(170, 220, 255);  // azul hielo (nieve)
    }
    return QColor(77, 232, 255);
}

// Color de la letra según la nota obtenida
static QColor colorNota(const QString &nota){
    if(nota == "A") return QColor(0, 255, 150);      // verde
    if(nota.startsWith("B")) return QColor(255, 255, 100); // amarillo
    if(nota.startsWith("C")) return QColor(255, 170, 60);  // naranja
    if(nota == "—") return QColor(150, 150, 150);    // gris: no jugado
    return QColor(255, 100, 100);                    // rojo: D/F
}

Puntajes::Puntajes(QWidget *parent) : QWidget(parent) {

    // Es una página de la ventana única: el tamaño lo define la ventana
    // (800x600). El fondo se estira para llenar toda la pantalla.
    fondo = QPixmap(":/Sprites/recursosh/menu_fondo_v2_640x880.png");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(16);

    QLabel *titulo = new QLabel("PUNTAJES Y LOGROS");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setAttribute(Qt::WA_StyledBackground, true);
    titulo->setStyleSheet(
        "QLabel {"
        "  color: #4de8ff;"
        "  font-family: 'Consolas';"
        "  font-size: 20px;"
        "  font-weight: bold;"
        "  letter-spacing: 3px;"
        "  background-color: rgba(15, 10, 25, 160);"
        "  border: 1px solid #4de8ff;"
        "  border-radius: 4px;"
        "  padding: 12px;"
        "}"
        );
    layout->addWidget(titulo);
    layout->addSpacing(10);

    // Mejor nota de cada nivel (guardada por Game::guardarLogro en QSettings)
    QSettings settings("HelicopterRescue", "Progreso");

    for(int i = 1; i <= 3; i++){
        QString clave = QString("logro_nivel_%1").arg(i);
        QString nota = settings.value(clave).toString();
        if(nota.isEmpty()) nota = "—";

        QColor cNivel = colorNivel(i);

        QWidget *fila = new QWidget(this);
        QHBoxLayout *filaLayout = new QHBoxLayout(fila);
        filaLayout->setContentsMargins(0, 0, 0, 0);
        filaLayout->setSpacing(20);

        QLabel *lblNivel = new QLabel(QString("NIVEL %1").arg(i));
        lblNivel->setFixedWidth(220);
        lblNivel->setAlignment(Qt::AlignCenter);
        lblNivel->setAttribute(Qt::WA_StyledBackground, true);
        lblNivel->setStyleSheet(QString(
            "QLabel {"
            "  color: %1;"
            "  font-family: 'Consolas';"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  letter-spacing: 2px;"
            "  background-color: rgba(15, 10, 25, 160);"
            "  border: 1px solid %1;"
            "  border-radius: 4px;"
            "  padding: 10px;"
            "}").arg(cNivel.name()));
        filaLayout->addWidget(lblNivel);

        filaLayout->addStretch();

        QLabel *lblNota = new QLabel(nota);
        lblNota->setAlignment(Qt::AlignCenter);
        lblNota->setAttribute(Qt::WA_StyledBackground, true);
        lblNota->setStyleSheet(QString(
            "QLabel {"
            "  color: %1;"
            "  font-family: 'Consolas';"
            "  font-size: 40px;"
            "  font-weight: bold;"
            "  background-color: rgba(15, 10, 25, 160);"
            "  border: 1px solid %1;"
            "  border-radius: 4px;"
            "  padding: 4px 24px;"
            "}").arg(colorNota(nota).name()));
        filaLayout->addWidget(lblNota);

        layout->addWidget(fila);
    }

    layout->addSpacing(10);

    // Logro dorado: todos los niveles en A
    bool todosA = true;
    for(int i = 1; i <= 3; i++){
        QString clave = QString("logro_nivel_%1").arg(i);
        if(settings.value(clave).toString() != "A"){ todosA = false; break; }
    }

    QLabel *lblLogro = new QLabel(todosA
        ? "★ ¡Consigue todos los logros en A! ★"
        : "★ ¡Consigue todos los logros en A! ★  (bloqueado)");
    lblLogro->setAlignment(Qt::AlignCenter);
    lblLogro->setAttribute(Qt::WA_StyledBackground, true);
    QString colorLogro = todosA ? "#ffd700" : "#888888";
    lblLogro->setStyleSheet(QString(
        "QLabel {"
        "  color: %1;"
        "  font-family: 'Consolas';"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  background-color: rgba(15, 10, 25, 160);"
        "  border: 1px solid %1;"
        "  border-radius: 4px;"
        "  padding: 12px;"
        "}").arg(colorLogro));
    layout->addWidget(lblLogro);

    layout->addStretch();

    QPushButton *btnVolver = new QPushButton("VOLVER");
    btnVolver->setFixedSize(220, 50);
    btnVolver->setCursor(Qt::PointingHandCursor);
    btnVolver->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #ff6b6b;"
        " border: 2px solid #ff6b6b; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    layout->addWidget(btnVolver, 0, Qt::AlignHCenter);

    connect(btnVolver, &QPushButton::clicked, this, &Puntajes::volver);
}

void Puntajes::paintEvent(QPaintEvent *event)
{
    // Fondo estirado a TODA la ventana (igual que el menú)
    QPainter painter(this);
    painter.fillRect(rect(), QColor(21, 10, 43));
    painter.drawPixmap(rect(), fondo);

    QWidget::paintEvent(event);
}

void Puntajes::volver(){
    // Regresar al menú principal (cambia de página en la ventana única)
    ventanaPrincipal->mostrarMenu();
}