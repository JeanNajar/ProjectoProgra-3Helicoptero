#include "LevelManager.h"

LevelManager::LevelManager(QObject *parent)
    : QObject(parent)
{
    active = false;
    finished = false;
    timeRemaining = 0;
    levelNumber = 1;

    //timer, cuenta 1 segundo a la vez

    levelTimer = new QTimer(this);
    connect(levelTimer, &QTimer::timeout, this, [this]() {
        if(timeRemaining > 0){
            timeRemaining--;
        }
        if(timeRemaining <= 0){
            levelTimer->stop();
            active = false;
            finished = true;
        }
    });

    // Timer de spawn
    spawnTimer = new QTimer(this);
}


LevelManager::~LevelManager(){
    levelTimer->stop();
    spawnTimer->stop();
}

void LevelManager::startLevel(int timeLimit, int spawnIntervalMs){
    timeRemaining = timeLimit;
    active = true;
    finished = false;

    // Iniciar timers
    levelTimer->start(1000);              // Cuenta 1 segundo a la vez
    spawnTimer->start(spawnIntervalMs);   // Spawn cada X milisegundos
}

bool LevelManager::isActive() const{
    return active;//si esta activo el timer
}

bool LevelManager::isFinished() const{
    return finished;//si ya termino el timer
}

int LevelManager::getTimeRemaining() const{
    return timeRemaining;//tiempo restante
}

int LevelManager::getLevelNumber() const{
    return levelNumber;//el # de nivel
}