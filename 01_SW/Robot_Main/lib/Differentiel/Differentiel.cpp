#include "Differentiel.hpp"

differentiel::differentiel(Stepper* moteurGauche, Stepper* moteurDroit, bool* StopLidar)
    : StepperG(moteurGauche), StepperD(moteurDroit),_stopLidar(StopLidar),  readyCount(0), semG(0), semD(0) 
{
    _positionX = _positionY = 0;
    _cibleposX = _cibleposY = 0;
    _Alpha = 0;
    _Speed = _SpeedAlpha = 0;
    _Move = _MoveAlpha = 0;
    _deltaG = _deltaD = 0;
     
    routineG.start(callback(this, &differentiel::routine_gauche));
    routineD.start(callback(this, &differentiel::routine_droite));
    threadOdometrie.start(callback(this, &differentiel::routine_odometrie));
    setPositionZero();
    resetPosition();
}

void differentiel::run()
{
    StepperG->run();
    StepperD->run();
}

void differentiel::pause()
{
    StepperG->pause();
    StepperD->pause();
}

void differentiel::resume()
{
    StepperG->resume();
    StepperD->resume();
}
void differentiel::stop()
{
    StepperG->stop();
    StepperD->stop();
}

void differentiel::setPosition(int positionX, int positionY, int Alpha, bool team)
{
     ScopedLock<Mutex> lock(mutexData);

    _positionX = team == 1  ? abs(3000-positionX) : positionX;
    _positionY = positionY;
    _cibleposX = _positionX;
    _cibleposY = _positionY;
    _Alpha = Alpha;
    lastPosG = -StepperG->getPosition();
    lastPosD = StepperD->getPosition();
}

void differentiel::setPositionZero()
{
    StepperG->setPositionZero();
    StepperD->setPositionZero();

}

void differentiel::resetPosition()
{
    ScopedLock<Mutex> lock(mutexData);
    _positionX = 0;
    _positionY = 0;
    _Alpha = 0;

    lastPosG = -StepperG->getPosition();
    lastPosD = StepperD->getPosition();
}

void differentiel::move(int Distance, int Alpha, float coefSpeed)
{
    float move = Distance;
    float moveAlpha = float(Alpha);

    {
        ScopedLock<Mutex> lock(mutexData);
        _Move = move;
        _MoveAlpha = moveAlpha;
        _Speed = (abs(_Move) / (abs(_Move) + abs(_MoveAlpha))) * SPEED * coefSpeed;
        _SpeedAlpha = (abs(_MoveAlpha) / (abs(_Move) + abs(_MoveAlpha))) * (SPEED * 0.5f *coefSpeed);
    }

    flags.set(0x1 | 0x2);
}

void differentiel::synchroniser() {
    ScopedLock<Mutex> lock(syncMutex);
    readyCount++;
    if (readyCount == 2) {  
        semG.release();
        semD.release();
        readyCount = 0;
    }
}

// ======================== Routines moteurs ========================= //

void differentiel::routine_gauche()
{
    while (true) {
        flags.wait_any(0x1); // attend ordre moteur gauche
        synchroniser();
        ScopedLock<Mutex> lock(mutexData);

        float speed = (_Speed + RADIUS * _SpeedAlpha * (M_PI/180.0f)) / KSTP;
        StepperG->setSpeed(speed);
        StepperG->setAcceleration(speed / ACC);
        StepperG->setDeceleration(speed / DEC);
        StepperG->move(-int((_Move + RADIUS * _MoveAlpha * (M_PI/180.0f)) / KSTP));
    }
}

void differentiel::routine_droite()
{
    while (true) {
        flags.wait_any(0x2); // attend ordre moteur droit
        synchroniser();
        ScopedLock<Mutex> lock(mutexData);

        float speed = (_Speed - RADIUS * _SpeedAlpha * (M_PI/180.0f)) / KSTP;
        StepperD->setSpeed(speed);
        StepperD->setAcceleration(speed / ACC);
        StepperD->setDeceleration(speed / DEC);
        StepperD->move(int((_Move - RADIUS * _MoveAlpha * (M_PI/180.0f)) / KSTP));
    }
}

void differentiel::routine_odometrie()
{
    while (true) {
        updatePosition();
        ThisThread::sleep_for(10ms);
    }
}

void differentiel::updatePosition()
{
    ScopedLock<Mutex> lock(mutexData);

    int posG = -StepperG->getPosition();
    int posD = StepperD->getPosition();

    _deltaG = posG - lastPosG;
    _deltaD = posD - lastPosD;

    lastPosG = posG;
    lastPosD = posD;

    float dG = _deltaG * KSTP;
    float dD = _deltaD * KSTP;

    float dC = (dG + dD) / 2.0f;
    float dAlpha = (dG - dD) / (2.0f * RADIUS); // radians

    float alpha_rad = _Alpha * (M_PI / 180.0f);
    _positionX += dC * sin(alpha_rad + dAlpha / 2.0f);
    _positionY += dC * cos(alpha_rad + dAlpha / 2.0f);

    alpha_rad += dAlpha;
    _Alpha = alpha_rad * (180.0f / M_PI);

    if (_Alpha > 180.0f) _Alpha -= 360.0f;
    if (_Alpha < -180.0f) _Alpha += 360.0f;
}

void differentiel::Robotmoveto(int distance, int alpha, bool enableLidar, float coefspeed)
{
  move(distance, alpha, coefspeed);
  do
  {
    if(enableLidar)(*_stopLidar) ? pause() : resume();
    ThisThread::sleep_for(100ms);
  } while (!PosCibleDone());
}

void differentiel::Robotgoto(int positionX, int positionY, int alpha, bool team, float coefSpeed)
{
    
  float dx = team == 1  ? abs(3000-positionX) - getPositionX() : positionX - getPositionX();
  float dy = positionY - getPositionY();
  _cibleposX = team == 1  ? abs(3000-positionX) : positionX;
  _cibleposY = positionY;
   
  if (dx < 0.1f and dx > -0.1f) dx = 0.0f;
  if (dy < 0.1f and dy > -0.1f) dy = 0.0f;
  // printf("dx = %d,dy = %d\n",dx,dy);
  int move = sqrt(dx * dx + dy * dy);
  // if ( move < 0.01f) move = 0.0f;

  float targetAlpha = ((180.0f / M_PI) * atan2(dx, dy));
  if (targetAlpha < 0.01f and targetAlpha > -0.01f) targetAlpha = 0.0f;
  float moveAlpha = targetAlpha - getAlpha();
  float finalAlpha = team == 1  ? (alpha*-1.0) - targetAlpha : alpha - targetAlpha;

  // Gérer la différence d'angle pour éviter les rotations inutiles
  // L'angle est borné entre -180° et 180°
  if (moveAlpha > 180) moveAlpha -= 360;
  if (moveAlpha < -180) moveAlpha += 360;

  if (finalAlpha > 180) finalAlpha -= 360;
  if (finalAlpha < -180) finalAlpha += 360;

  // Si la différence d'angle est proche de 180° ou -180°, on inverse le mouvement
  if (std::abs(moveAlpha) > 90)
  {
    moveAlpha -= 180;
    finalAlpha -= 180;
    move = -move;
    if (moveAlpha > 180) moveAlpha -= 360;
    if (moveAlpha < -180) moveAlpha += 360;
    if (finalAlpha > 180) finalAlpha -= 360;
    if (finalAlpha < -180) finalAlpha += 360;
  }
  // 1. Rotation vers direction
  updatePosition();
  ThisThread::sleep_for(10ms);
  Robotmoveto(0, moveAlpha, false, coefSpeed);

  // 2. Translation
  updatePosition();
  ThisThread::sleep_for(10ms);
  Robotmoveto(move, 0, true, coefSpeed);

  // 3. Rotation finale vers Alpha
  updatePosition();
  ThisThread::sleep_for(10ms);
  Robotmoveto(0, finalAlpha, false, coefSpeed);
}

// ======================== Getters ========================= //

float differentiel::getSpeedG()      { return StepperG->getSpeed(); }
float differentiel::getSpeedD()      { return StepperD->getSpeed(); }
int   differentiel::getPosG()        { return -StepperG->getPosition(); }
int   differentiel::getPosD()        { return StepperD->getPosition(); }
int   differentiel::getStepG()       { return StepperG->getStep(); }
int   differentiel::getStepD()       { return StepperD->getStep(); }

float differentiel::getPositionX()   { ScopedLock<Mutex> lock(mutexData); updatePosition(); return _positionX; }
float differentiel::getPositionY()   { ScopedLock<Mutex> lock(mutexData); updatePosition(); return _positionY; }
float differentiel::getPosCibleX()   { ScopedLock<Mutex> lock(mutexData); return _cibleposX; }
float differentiel::getPosCibleY()   { ScopedLock<Mutex> lock(mutexData); return _cibleposY; }
float differentiel::getAlpha()       { ScopedLock<Mutex> lock(mutexData); updatePosition(); return _Alpha; }
float differentiel::getSpeed()       { ScopedLock<Mutex> lock(mutexData); return _Speed; }
float differentiel::getSpeedAlpha()  { ScopedLock<Mutex> lock(mutexData); return _SpeedAlpha; }
int   differentiel::getDeltaG()        { ScopedLock<Mutex> lock(mutexData); return _deltaG; }
int   differentiel::getDeltaD()        { ScopedLock<Mutex> lock(mutexData); return _deltaD; }


bool differentiel::stopped()
{
    return StepperG->stopped() && StepperD->stopped();
}

bool differentiel::PosCibleDone()
{
    return StepperG->getPosCibleDone() && StepperD->getPosCibleDone();
}
