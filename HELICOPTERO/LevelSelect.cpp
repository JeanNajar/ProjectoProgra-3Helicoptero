#include "LevelSelect.h"
#include "Game.h"
#include "Menu.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QPainter>
#include <QSettings>

//punteros globales (definidos en main.cpp)
extern Game * game;
extern Menu * menu;
extern QString usuarioActual;

LevelSelect::LevelSelect(QWidget *parent) : QWidget(parent) {

    setWindowTitle("Seleccionar Nivel");

    // El fondo del selector es 800x600, mismo tamaño que la ventana
    resize(800, 600);
    setMinimumSize(640, 480);

    fondo = QPixmap(":/Sprites/recursosh/seleccionar_nivel_fondo_800x600.png");

    // Nivel más alto desbloqueado (1 por defecto). Completar el nivel N
    // desbloquea el N+1 (lo guarda Game::mostrarVictoria en QSettings).
    // El progreso es POR USUARIO: cada cuenta tiene su propio desbloqueo.
    QSettings settings("HelicopterRescue", "Progreso");
    QString clave = QString("nivelDesbloqueado_%1").arg(usuarioActual);
    int desbloqueado = settings.value(clave, 1).toInt();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 70, 30, 30);
    layout->setSpacing(20);

    layout->addStretch();

    // Las 3 cards en una fila (230x148 escaladas desde 280x180 para que
    // las tres entren en los 800px del fondo)
    QHBoxLayout *fila = new QHBoxLayout();
    fila->setSpacing(20);
    for(int i = 1; i <= 3; i++){
        QPushButton *card = crearCardNivel(i, i <= desbloqueado);
        fila->addWidget(card);
    }
    layout->addLayout(fila);

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

    connect(btnVolver, &QPushButton::clicked, this, &LevelSelect::volver);
}

void LevelSelect::paintEvent(QPaintEvent *event)
{
    // Fondo oscuro + imagen del selector escalada y centrada
    QPainter painter(this);
    painter.fillRect(rect(), QColor(21, 10, 43));

    QPixmap escalado = fondo.scaled(size(), Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
    int x = (width() - escalado.width()) / 2;
    int y = (height() - escalado.height()) / 2;
    painter.drawPixmap(x, y, escalado);

    QWidget::paintEvent(event);
}

QPushButton* LevelSelect::crearCardNivel(int nivel, bool desbloqueado)
{
    QPushButton *card = new QPushButton(this);
    card->setFixedSize(230, 148);

    if(desbloqueado){
        // Card desbloqueada: imagen a color y clickeable
        card->setIcon(QIcon(QString(":/Sprites/recursosh/nivel_card_%1_unlocked.png").arg(nivel)));
        card->setCursor(Qt::PointingHandCursor);
        connect(card, &QPushButton::clicked, this,
                [this, nivel]() { jugarNivel(nivel); });
    }else{
        // Card bloqueada: imagen con candado y NO clickeable
        card->setIcon(QIcon(QString(":/Sprites/recursosh/nivel_card_%1_locked.png").arg(nivel)));
        card->setEnabled(false);
    }

    card->setIconSize(QSize(230, 148));
    card->setFlat(true);
    card->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { border: 2px solid #4de8ff; border-radius: 6px; }"
        "QPushButton:pressed { background: rgba(0, 0, 0, 80); }");

    return card;
}

void LevelSelect::jugarNivel(int nivel){
    // Crear el juego con el nivel elegido y cerrar este menú
    game = new Game(nivel);
    game->setAttribute(Qt::WA_DeleteOnClose);
    game->show();
    this->close();
}

void LevelSelect::volver(){
    // Regresar al menú principal
    menu->show();
    this->close();
}