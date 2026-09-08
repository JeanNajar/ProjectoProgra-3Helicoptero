#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <QObject>
#include <QTimer>
#include <QGraphicsTextItem>

class LevelManager : public QObject{
    Q_OBJECT

public:
    LevelManager(QObject *parent = nullptr);
    ~LevelManager();

    void startLevel(int timeLimit,int spawnIntervalMs);

    bool isActive() const;
    bool isFinished() const;
    int getTimeRemaining() const;
    int getLevelNumber() const;

    // Temporizador del nivel segundos restantes
    QTimer *levelTimer;
    // Timer que dispara el spawn de obstáculos
    QTimer *spawnTimer;


private:
    bool active;       //revisa si el nivel está en curso
    bool finished;     // revisa si el nivel ya terminó
    int timeRemaining; // segundos restantes
    int levelNumber;   // Número del nivel actual

};

#endif // LEVELMANAGER_H
