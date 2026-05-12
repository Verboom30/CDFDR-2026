#include "Holonome.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Holonome::Holonome(Stepper* moteurA, Stepper* moteurB, Stepper* moteurC, bool* StopLidar)
    : StepperA(moteurA),
      StepperB(moteurB),
      StepperC(moteurC),
      _stopLidar(StopLidar)
{
    _positionX = 0.0f;
    _positionY = 0.0f;
    _Theta = 0.0f;

    _positionX_Save = 0.0f;
    _positionY_Save = 0.0f;
    _Theta_Save = 0.0f;

    _cibleposX = 0.0f;
    _cibleposY = 0.0f;

    _MoveX = 0.0f;
    _MoveY = 0.0f;
    _MoveTheta = 0.0f;

    _SpeedX = 0.0f;
    _SpeedY = 0.0f;
    _SpeedTheta = 0.0f;

    _SpeedA = 0.0f;
    _SpeedB = 0.0f;
    _SpeedC = 0.0f;

    _StepA = 0;
    _StepB = 0;
    _StepC = 0;

    lastPosA = 0;
    lastPosB = 0;
    lastPosC = 0;

    _deltaA = 0;
    _deltaB = 0;
    _deltaC = 0;

    routineMove.start(callback(this, &Holonome::routine_mouvement));
    threadOdometrie.start(callback(this, &Holonome::routine_odometrie));

    setPositionZero();
    resetPosition();
}

void Holonome::run()
{
    StepperA->run();
    StepperB->run();
    StepperC->run();
}

void Holonome::pause()
{
    StepperA->pause();
    StepperB->pause();
    StepperC->pause();
}

void Holonome::resume()
{
    StepperA->resume();
    StepperB->resume();
    StepperC->resume();
}

void Holonome::stop()
{
    StepperA->stop();
    StepperB->stop();
    StepperC->stop();
}

void Holonome::setPosition(int positionX, int positionY, int theta, Team team)
{
    ScopedLock<Mutex> lock(mutexData);

    if (team == BLUE)
    {
        _positionX = 3000.0f - positionX;
        _positionY = positionY;
        _Theta = -theta;
    }
    else
    {
        _positionX = positionX;
        _positionY = positionY;
        _Theta = theta;
    }

    _positionX_Save = _positionX;
    _positionY_Save = _positionY;
    _Theta_Save = _Theta;

    _cibleposX = _positionX;
    _cibleposY = _positionY;

    StepperA->setPositionZero();
    StepperB->setPositionZero();
    StepperC->setPositionZero();

    lastPosA = 0;
    lastPosB = 0;
    lastPosC = 0;
}

void Holonome::setPositionZero()
{
    StepperA->setPositionZero();
    StepperB->setPositionZero();
    StepperC->setPositionZero();

    lastPosA = 0;
    lastPosB = 0;
    lastPosC = 0;
}

void Holonome::resetPosition()
{
    ScopedLock<Mutex> lock(mutexData);

    _positionX = 0.0f;
    _positionY = 0.0f;
    _Theta = 0.0f;

    _positionX_Save = 0.0f;
    _positionY_Save = 0.0f;
    _Theta_Save = 0.0f;

    _cibleposX = 0.0f;
    _cibleposY = 0.0f;

    StepperA->setPositionZero();
    StepperB->setPositionZero();
    StepperC->setPositionZero();

    lastPosA = 0;
    lastPosB = 0;
    lastPosC = 0;
}

float Holonome::normalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

void Holonome::move(int moveX, int moveY, int moveTheta, float coefSpeed)
{
    {
        ScopedLock<Mutex> lock(mutexData);

        _MoveX = moveX;
        _MoveY = moveY;
        _MoveTheta = moveTheta;

        _positionX_Save = _positionX;
        _positionY_Save = _positionY;
        _Theta_Save = _Theta;

        StepperA->setPositionZero();
        StepperB->setPositionZero();
        StepperC->setPositionZero();

        lastPosA = 0;
        lastPosB = 0;
        lastPosC = 0;

        float sum = fabsf(_MoveX) + fabsf(_MoveY) + fabsf(_MoveTheta);

        if (sum < 0.001f)
        {
            _SpeedX = 0.0f;
            _SpeedY = 0.0f;
            _SpeedTheta = 0.0f;
        }
        else
        {
            _SpeedX     = (_MoveX / sum) * SPEED * coefSpeed;
            _SpeedY     = (_MoveY / sum) * SPEED * coefSpeed;
            _SpeedTheta = (_MoveTheta / sum) * (2.0f * SPEED / RADIUS) * coefSpeed;
        }

        computeKinematics();
    }

    flags.set(0x1);
}

void Holonome::computeKinematics()
{
    float alphaRad      = -_Theta * M_PI / 180.0f;
    float thetaPlusRad  = (60.0f -_Theta) * M_PI / 180.0f;
    float thetaMinusRad = (60.0f + _Theta) * M_PI / 180.0f;

    float moveAlphaRad = _MoveTheta * M_PI / 180.0f;

    float moveY  = -_MoveY;
    float speedY = -_SpeedY;

    float moveA =(-RADIUS * moveAlphaRad)- cosf(alphaRad) * _MoveX+ sinf(alphaRad) * moveY;

    float moveB =(-RADIUS * moveAlphaRad)+ cosf(thetaPlusRad) * _MoveX- sinf(thetaPlusRad) * moveY;

    float moveC =(-RADIUS * moveAlphaRad)+ cosf(thetaMinusRad) * _MoveX+ sinf(thetaMinusRad) * moveY;

    float speedA =(-RADIUS * _SpeedTheta)- cosf(alphaRad) * _SpeedX+ sinf(alphaRad) * speedY;

    float speedB =(-RADIUS * _SpeedTheta)+ cosf(thetaPlusRad) * _SpeedX- sinf(thetaPlusRad) * speedY;

    float speedC =(-RADIUS * _SpeedTheta)+ cosf(thetaMinusRad) * _SpeedX+ sinf(thetaMinusRad) * speedY;

    _StepA = (int)(moveA / KSTP);
    _StepB = (int)(moveB / KSTP);
    _StepC = (int)(moveC / KSTP);

    _SpeedA = fabsf(speedA / KSTP);
    _SpeedB = fabsf(speedB / KSTP);
    _SpeedC = fabsf(speedC / KSTP);

    if (_SpeedA < 1.0f && _StepA != 0) _SpeedA = 1.0f;
    if (_SpeedB < 1.0f && _StepB != 0) _SpeedB = 1.0f;
    if (_SpeedC < 1.0f && _StepC != 0) _SpeedC = 1.0f;
}

void Holonome::routine_mouvement()
{
    while (true)
    {
        flags.wait_any(0x1);

        float speedA;
        float speedB;
        float speedC;

        int stepA;
        int stepB;
        int stepC;

        {
            ScopedLock<Mutex> lock(mutexData);

            speedA = _SpeedA;
            speedB = _SpeedB;
            speedC = _SpeedC;

            stepA = _StepA;
            stepB = _StepB;
            stepC = _StepC;
        }

        StepperA->setSpeed(speedA);
        StepperB->setSpeed(speedB);
        StepperC->setSpeed(speedC);

        StepperA->setAcceleration(speedA / ACC);
        StepperB->setAcceleration(speedB / ACC);
        StepperC->setAcceleration(speedC / ACC);

        StepperA->setDeceleration(speedA / DEC);
        StepperB->setDeceleration(speedB / DEC);
        StepperC->setDeceleration(speedC / DEC);

        StepperA->move(stepA);
        StepperB->move(stepB);
        StepperC->move(stepC);
    }
}

void Holonome::routine_odometrie()
{
    while (true)
    {
        updatePosition();
        ThisThread::sleep_for(10ms);
    }
}

void Holonome::updatePosition()
{
    ScopedLock<Mutex> lock(mutexData);

    int posA = StepperA->getPosition();
    int posB = StepperB->getPosition();
    int posC = StepperC->getPosition();

    _deltaA = posA - lastPosA;
    _deltaB = posB - lastPosB;
    _deltaC = posC - lastPosC;

    lastPosA = posA;
    lastPosB = posB;
    lastPosC = posC;

    float alpha = _Theta_Save + ((-1.0f / (3.0f * RADIUS)) * posB + (-1.0f / (3.0f * RADIUS)) * posA + (-1.0f / (3.0f * RADIUS)) * posC) * KSTP / (M_PI / 180.0f);

    _Theta = normalizeAngle(alpha);

    float alphaRad = -alpha * M_PI / 180.0f;


    _positionX = _positionX_Save + ((-cosf(alphaRad) / 6.0f) * posA + ((cosf(alphaRad) - sqrtf(3.0f) * sinf(alphaRad)) / 12.0f) * posB + ((sqrtf(3.0f) * sinf(alphaRad) + cosf(alphaRad)) / 12.0f) * posC ) * KSTP * 4.0f;

    _positionY = _positionY_Save - ((sinf(alphaRad) / 6.0f) * posA - ((sinf(alphaRad) + sqrtf(3.0f) * cosf(alphaRad)) / 12.0f) * posB + ((sqrtf(3.0f) * cosf(alphaRad) - sinf(alphaRad)) / 12.0f) * posC) * KSTP * 4.0f;
}

void Holonome::Robotmove(int moveX, int moveY, int moveTheta, bool enableLidar, float coefSpeed)
{
    move(moveX, moveY, moveTheta, coefSpeed);

    do
    {
        if (enableLidar && _stopLidar != nullptr)
        {
            (*_stopLidar) ? pause() : resume();
        }

        ThisThread::sleep_for(100ms);

    } while (!PosCibleDone());
}

void Holonome::Robotgoto(int positionX,int positionY,int theta,Team team,float coefSpeed)
{
    float targetX;
    float targetY;
    float targetTheta;

    // gestion symétrie terrain
    if (team == BLUE)
    {
        targetX = 3000.0f - positionX;
        targetY = positionY;
        targetTheta = -theta;
    }
    else
    {
        targetX = positionX;
        targetY = positionY;
        targetTheta = theta;
    }

    // position actuelle
    float currentX = getPositionX();
    float currentY = getPositionY();
    float currentTheta = getTheta();

    // déplacement absolu table
    float moveX = targetX - currentX;
    float moveY = targetY - currentY;

    // rotation finale uniquement
    float moveTheta =
        normalizeAngle(targetTheta - currentTheta);

    // filtrage petites erreurs
    if (fabsf(moveX) < 0.1f) moveX = 0.0f;
    if (fabsf(moveY) < 0.1f) moveY = 0.0f;
    if (fabsf(moveTheta) < 0.1f) moveTheta = 0.0f;

    {
        ScopedLock<Mutex> lock(mutexData);

        _cibleposX = targetX;
        _cibleposY = targetY;
    }

    // -----------------------------
    // 1) translation
    // -----------------------------
    if (moveX != 0.0f || moveY != 0.0f)
    {
        Robotmove(
            (int)moveX,
            (int)moveY,
            0,
            true,
            coefSpeed
        );
    }

    // -----------------------------
    // 2) rotation
    // -----------------------------
    if (moveTheta != 0.0f)
    {
        Robotmove(
            0,
            0,
            (int)moveTheta,
            true,
            coefSpeed
        );
    }
}

bool Holonome::stopped()
{
    return StepperA->stopped() &&
           StepperB->stopped() &&
           StepperC->stopped();
}

bool Holonome::PosCibleDone()
{
    return StepperA->getPosCibleDone() &&
           StepperB->getPosCibleDone() &&
           StepperC->getPosCibleDone();
}

float Holonome::getPositionX()
{
    ScopedLock<Mutex> lock(mutexData);
    return _positionX;
}

float Holonome::getPositionY()
{
    ScopedLock<Mutex> lock(mutexData);
    return _positionY;
}

float Holonome::getTheta()
{
    ScopedLock<Mutex> lock(mutexData);
    return _Theta;
}

float Holonome::getPosCibleX()
{
    ScopedLock<Mutex> lock(mutexData);
    return _cibleposX;
}

float Holonome::getPosCibleY()
{
    ScopedLock<Mutex> lock(mutexData);
    return _cibleposY;
}

float Holonome::getSpeedA()
{
    return StepperA->getSpeed();
}

float Holonome::getSpeedB()
{
    return StepperB->getSpeed();
}

float Holonome::getSpeedC()
{
    return StepperC->getSpeed();
}

int Holonome::getPosA()
{
    return StepperA->getPosition();
}

int Holonome::getPosB()
{
    return StepperB->getPosition();
}

int Holonome::getPosC()
{
    return StepperC->getPosition();
}

int Holonome::getDeltaA()
{
    ScopedLock<Mutex> lock(mutexData);
    return _deltaA;
}

int Holonome::getDeltaB()
{
    ScopedLock<Mutex> lock(mutexData);
    return _deltaB;
}

int Holonome::getDeltaC()
{
    ScopedLock<Mutex> lock(mutexData);
    return _deltaC;
}