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
    _positionX = _positionY = 0;
    _cibleposX = _cibleposY = 0;
    _Theta = 0;

    _MoveX = _MoveY = _MoveTheta = 0;
    _SpeedX = _SpeedY = _SpeedTheta = 0;

    _SpeedA = _SpeedB = _SpeedC = 0;
    _StepA = _StepB = _StepC = 0;

    _deltaA = _deltaB = _deltaC = 0;

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

    _cibleposX = _positionX;
    _cibleposY = _positionY;

    lastPosA = StepperA->getPosition();
    lastPosB = StepperB->getPosition();
    lastPosC = StepperC->getPosition();
}

void Holonome::setPositionZero()
{
    StepperA->setPositionZero();
    StepperB->setPositionZero();
    StepperC->setPositionZero();
}

void Holonome::resetPosition()
{
    ScopedLock<Mutex> lock(mutexData);

    _positionX = 0;
    _positionY = 0;
    _Theta = 0;

    lastPosA = StepperA->getPosition();
    lastPosB = StepperB->getPosition();
    lastPosC = StepperC->getPosition();
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
        _MoveY = moveY;       // inversion Y conservée car Robotmove fonctionne avec ça
        _MoveTheta = moveTheta;

        float sum = fabsf(_MoveX) + fabsf(_MoveY) + fabsf(_MoveTheta);

        if (sum < 0.001f)
        {
            _SpeedX = 0;
            _SpeedY = 0;
            _SpeedTheta = 0;
        }
        else
        {
            _SpeedX     = (fabsf(_MoveX)     / sum) * SPEED * coefSpeed;
            _SpeedY     = (fabsf(_MoveY)     / sum) * SPEED * coefSpeed;
            _SpeedTheta = (fabsf(_MoveTheta) / sum) * SPEED * (1.2f/RADIUS) * coefSpeed;
        }

        computeKinematics();
    }

    flags.set(0x1);
}

void Holonome::computeKinematics()
{
    float alphaRad = _Theta * M_PI / 180.0f;
    float thetaPlusRad  = (60 + _Theta) * M_PI / 180.0f;
    float thetaMinusRad = (60 - _Theta) * M_PI / 180.0f;

    float moveAlphaRad = _MoveTheta * M_PI / 180.0f;

    float rotMove = RADIUS * moveAlphaRad;

    float rotSpeed = RADIUS * _SpeedTheta;

    float moveA = -rotMove - cosf(alphaRad) * _MoveX+ sinf(alphaRad) * _MoveY;

    float moveB = -rotMove + cosf(thetaPlusRad) * _MoveX + sinf(thetaPlusRad) * _MoveY;

    float moveC = -rotMove + cosf(thetaMinusRad) * _MoveX - sinf(thetaMinusRad) * _MoveY;

    float speedA = -rotSpeed - cosf(alphaRad) * _SpeedX + sinf(alphaRad) * _SpeedY;

    float speedB = -rotSpeed + cosf(thetaPlusRad) * _SpeedX + sinf(thetaPlusRad) * _SpeedY;

    float speedC = -rotSpeed + cosf(thetaMinusRad) * _SpeedX - sinf(thetaMinusRad) * _SpeedY;

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

    float alphaRad = _Theta * M_PI / 180.0f;

    float dAlpha =( (-1.0f / (3.0f * RADIUS)) * _deltaB + (-1.0f / (3.0f * RADIUS)) * _deltaA + (-1.0f / (3.0f * RADIUS)) * _deltaC ) * KSTP;

    float dX = ((-cosf(alphaRad) / 6.0f) * _deltaA + ((cosf(alphaRad) - sqrtf(3.0f) * sinf(alphaRad)) / 12.0f) * _deltaB + ((sqrtf(3.0f) * sinf(alphaRad) + cosf(alphaRad)) / 12.0f) * _deltaC ) * KSTP * 4.0f;

    float dY =-((sinf(alphaRad) / 6.0f) * _deltaA -((sinf(alphaRad) + sqrtf(3.0f) * cosf(alphaRad)) / 12.0f) * _deltaB + ((sqrtf(3.0f) * cosf(alphaRad) - sinf(alphaRad)) / 12.0f) * _deltaC) * KSTP * 4.0f;

    _Theta += dAlpha * 180.0f / M_PI;
    _Theta = normalizeAngle(_Theta);

    _positionX += dX;
    _positionY += dY;
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

void Holonome::Robotgoto(int positionX, int positionY, int theta, Team team, float coefSpeed)
{
    float targetX;
    float targetY;
    float targetTheta;

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

    float currentX = getPositionX();
    float currentY = getPositionY();
    float currentTheta = getTheta();

    float moveX = targetX - currentX;
    float moveY = targetY - currentY;
    float moveTheta = normalizeAngle(targetTheta - currentTheta);

    if (moveX > -0.1f && moveX < 0.1f) moveX = 0.0f;
    if (moveY > -0.1f && moveY < 0.1f) moveY = 0.0f;
    if (moveTheta > -0.1f && moveTheta < 0.1f) moveTheta = 0.0f;

    {
        ScopedLock<Mutex> lock(mutexData);
        _cibleposX = targetX;
        _cibleposY = targetY;
    }

    Robotmove((int)moveX, (int)moveY, (int)moveTheta, true, coefSpeed);
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
