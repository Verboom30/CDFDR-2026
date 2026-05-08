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

void Holonome::setPosition(int positionX, int positionY, int theta, bool team)
{
    ScopedLock<Mutex> lock(mutexData);

    _positionX = team ? abs(3000 - positionX) : positionX;
    _positionY = positionY;

    _cibleposX = _positionX;
    _cibleposY = _positionY;

    _Theta = team ? -theta : theta;

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
        _MoveY = -moveY;
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
            _SpeedTheta = (fabsf(_MoveTheta) / sum) * SPEED * 0.5f * coefSpeed;
        }

        computeKinematics();
    }

    flags.set(0x1);
}

void Holonome::computeKinematics()
{
    float thetaRad = _Theta * M_PI / 180.0f;

    float rotMove  = RADIUS * _MoveTheta  * M_PI / 180.0f;
    float rotSpeed = RADIUS * _SpeedTheta * M_PI / 180.0f;

    float moveA =
        -rotMove
        - cosf(thetaRad) * _MoveX
        + sinf(thetaRad) * _MoveY;

    float moveB =
        -rotMove
        + cosf((60.0f * M_PI / 180.0f) + thetaRad) * _MoveX
        - sinf((60.0f * M_PI / 180.0f) + thetaRad) * _MoveY;

    float moveC =
        -rotMove
        + cosf((60.0f * M_PI / 180.0f) - thetaRad) * _MoveX
        + sinf((60.0f * M_PI / 180.0f) - thetaRad) * _MoveY;

    float speedA =
        -rotSpeed
        - cosf(thetaRad) * _SpeedX
        + sinf(thetaRad) * _SpeedY;

    float speedB =
        -rotSpeed
        + cosf((60.0f * M_PI / 180.0f) + thetaRad) * _SpeedX
        - sinf((60.0f * M_PI / 180.0f) + thetaRad) * _SpeedY;

    float speedC =
        -rotSpeed
        + cosf((60.0f * M_PI / 180.0f) - thetaRad) * _SpeedX
        + sinf((60.0f * M_PI / 180.0f) - thetaRad) * _SpeedY;

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

    float dA = _deltaA * KSTP;
    float dB = _deltaB * KSTP;
    float dC = _deltaC * KSTP;

    float dTheta = -(dA + dB + dC) / (3.0f * RADIUS);

    float dX_robot = (-2.0f * dA + dB + dC) / 3.0f;
    float dY_robot = (dA - dB + dC) / 1.7320508f;

    float thetaRad = _Theta * M_PI / 180.0f;

    float dX_table = dX_robot * cosf(thetaRad) + dY_robot * sinf(thetaRad);
    float dY_table = -dX_robot * sinf(thetaRad) + dY_robot * cosf(thetaRad);

    _positionX += dX_table;
    _positionY += dY_table;

    thetaRad += dTheta;
    _Theta = normalizeAngle(thetaRad * 180.0f / M_PI);
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

void Holonome::Robotgoto(int positionX, int positionY, int theta, bool team, float coefSpeed)
{
    float targetX = team ? abs(3000 - positionX) : positionX;
    float targetY = positionY;

    float dx = targetX - getPositionX();
    float dy = targetY - getPositionY();

    {
        ScopedLock<Mutex> lock(mutexData);
        _cibleposX = targetX;
        _cibleposY = targetY;
    }

    if (dx < 0.1f && dx > -0.1f) dx = 0.0f;
    if (dy < 0.1f && dy > -0.1f) dy = 0.0f;

    float finalTheta = team ? -theta : theta;
    float moveTheta = normalizeAngle(finalTheta - getTheta());

    Robotmove((int)dx, (int)dy, (int)moveTheta, true, coefSpeed);
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