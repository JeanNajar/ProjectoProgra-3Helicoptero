#include "Ranking.h"
#include "UserManager.h"
#include "VentanaPrincipal.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>

//punteros globales (definidos en main.cpp)
extern VentanaPrincipal * ventanaPrincipal;
extern UserManager * userManager;
extern QString usuarioActual;

Ranking::Ranking(QWidget *parent) : QWidget(parent) {

    // Es una página de la ventana única: el tamaño lo define la ventana
    // (800x600). El fondo se estira para llenar toda la pantalla.
    fondo = QPixmap(":/Sprites/recursosh/menu_fondo_v2_640x880.png");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(16);

    QLabel *titulo = new QLabel("RANKING");
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

    // Copia de usuarios: se ordena sin tocar el orden interno de UserManager
    QList<Usuario> lista = userManager->obtenerTodos();

    // Bubble sort: de mayor a menor puntajeTotal
    for(int i = 0; i < lista.size() - 1; i++){
        for(int j = 0; j < lista.size() - 1 - i; j++){
            if(lista[j].puntajeTotal < lista[j + 1].puntajeTotal){
                Usuario tmp = lista[j];
                lista[j] = lista[j + 1];
                lista[j + 1] = tmp;
            }
        }
    }

    // Tabla: posicion, usuario y puntaje acumulado
    tabla = new QTableWidget(lista.size(), 3, this);
    tabla->setHorizontalHeaderLabels(QStringList() << "#" << "USUARIO" << "PUNTAJE");
    tabla->verticalHeader()->setVisible(false);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabla->setSelectionMode(QAbstractItemView::NoSelection);
    tabla->setFocusPolicy(Qt::NoFocus);
    tabla->setShowGrid(false);
    tabla->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tabla->setColumnWidth(0, 70);
    tabla->setColumnWidth(2, 130);
    tabla->setStyleSheet(
        "QTableWidget {"
        "  background-color: rgba(15, 10, 25, 200);"
        "  color: #ffffff;"
        "  border: 1px solid #4de8ff;"
        "  border-radius: 4px;"
        "  font-family: 'Consolas';"
        "  font-size: 15px;"
        "}"
        "QHeaderView::section {"
        "  background-color: #1a1a2e;"
        "  color: #4de8ff;"
        "  border: 1px solid #4de8ff;"
        "  font-family: 'Consolas';"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 6px;"
        "}"
        "QTableWidget::item { padding: 4px; }"
        "QTableWidget::item:selected { background-color: #2a2a4e; }");

    for(int i = 0; i < lista.size(); i++){
        QTableWidgetItem *itemPos = new QTableWidgetItem(QString::number(i + 1));
        itemPos->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *itemUser = new QTableWidgetItem(lista[i].nombreUsuario);
        itemUser->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *itemPts = new QTableWidgetItem(QString::number(lista[i].puntajeTotal));
        itemPts->setTextAlignment(Qt::AlignCenter);

        tabla->setItem(i, 0, itemPos);
        tabla->setItem(i, 1, itemUser);
        tabla->setItem(i, 2, itemPts);

        // Resaltar la fila del usuario logueado
        if(lista[i].nombreUsuario == usuarioActual){
            QColor resaltado(42, 42, 78);
            itemPos->setBackground(resaltado);
            itemUser->setBackground(resaltado);
            itemPts->setBackground(resaltado);
            QFont negrita = itemUser->font();
            negrita.setBold(true);
            itemPos->setFont(negrita);
            itemUser->setFont(negrita);
            itemPts->setFont(negrita);
        }
    }
    layout->addWidget(tabla, 1);

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

    connect(btnVolver, &QPushButton::clicked, this, &Ranking::volver);
}

void Ranking::paintEvent(QPaintEvent *event)
{
    // Fondo estirado a TODA la ventana (igual que el menú y puntajes)
    QPainter painter(this);
    painter.fillRect(rect(), QColor(21, 10, 43));
    painter.drawPixmap(rect(), fondo);

    QWidget::paintEvent(event);
}

void Ranking::volver(){
    // Regresar al menú principal (cambia de página en la ventana única)
    ventanaPrincipal->mostrarMenu();
}