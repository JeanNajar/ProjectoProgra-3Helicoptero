#ifndef PHYSICS_H
#define PHYSICS_H

class Physics{
public:

    Physics();

    void applyGravity(double &velY,double dt);
    void applyThrust(double &velY,double dt);
    void applyFriction(double &velX,double dt);
    double calculateTilt(double velY, double velX) const;
    bool SafeLanding(double velY) const;

    void clampFallSpeed(double &velY) const;

    double getMaxHorizontalSpeed() const;
    double getMaxFallSpeed() const;

private:
    double gravedad;
    double empuje;
    double velocidadMaxCaida;
    double velocidadMaxAterrizaje;
    double velocidadMaxHorizontal;
    double friccion; // factor de desaceleración horizontal


};

#endif // PHYSICS_H
