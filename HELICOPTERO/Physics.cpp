#include "Physics.h"
#include <cmath>

Physics :: Physics(){

    //constantes (px=pixeles)

    gravedad = 500.0; // px/s^2
    empuje = -800; // px/s^2 negativo = para arriba
    velocidadMaxCaida = 350.0; // px/s
    velocidadMaxAterrizaje = 200.0; // px/s
    velocidadMaxHorizontal = 300.0; // px/s

    friccion = 0.90; //va por frames
}

void Physics::applyGravity(double &velY,double dt){

    //formula de aceleracion  v = v0 + a*t

    velY += gravedad * dt;

    //limitar maxima velocidad de caida
    clampFallSpeed(velY);

}

void Physics::applyThrust(double &velY,double dt){

    //el empuje negativo par que el helicoptero suba
    velY += empuje * dt;

   //limite de ascenso

    if(velY < -velocidadMaxCaida){
        velY = -velocidadMaxCaida;
    }
}

void Physics::applyFriction(double &velX, double dt){
    //Inercia, funciona cuando se deja de tocar la tecla de
    //  derecha/izquierda hace que no se frene en seco
    velX *= std::pow(friccion,dt*60.0);

    //al momento que llegue a menos de uno se redondea a 0 para que no deslice
    if(std::abs(velX)< 1.0){
        velX=0.0;
    }

}

double Physics::calculateTilt(double velY, double velX) const{
    
    
    //angulo de rotacion
    
    double tilt = velY * 0.06 + velX * 0.02;
    
    if(tilt > 25.0) tilt = 25.0;
    if(tilt < -25.0) tilt = -25.0;

    return tilt;
}

bool Physics::SafeLanding(double velY)const{
    return velY <= velocidadMaxAterrizaje;
}

void Physics::clampFallSpeed(double &velY) const{
    // Velocidad terminal
    if(velY > velocidadMaxCaida){
        velY = velocidadMaxCaida;
    }
}

double Physics::getMaxHorizontalSpeed() const{
    return velocidadMaxHorizontal;
}

double Physics::getMaxFallSpeed() const{
    return velocidadMaxCaida;
}