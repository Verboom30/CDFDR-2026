#include "Stepper.hpp"

Stepper::Stepper(PinName clk, PinName dir, bool inv)
    : _clk(clk), _dirPin(dir), _inv(inv), _dir(0), _pos(0)
{
    _clk = 0;
    _dirPin = (_inv) ? !_dir : _dir;
    _state = STOP;
    _prev_state = STOP;
    _pos = 0;
    _steps = 0;
    _spd = 0;
    _dt0 = 0;
    _n = 0;
    _nStartDec = 0;
    _dtn = 0;
    _dtmin = 0;
    _i = 0;
    _acc = 0;
    _dec = 0;
    _Pos_Cible_Done = 1;
    _pauseRequested = false;
    _pauseActive = false;
    _pausedWasContinuous = false;
    _pauseOriginalTargetSteps = 0;
    _pausedResumeSteps = 0;
    _pauseDecOverride = -1.0f;
    _resumeAccOverride = -1.0f;
}

void Stepper::setSpeed(float speed)
{
    _spd = (speed < 0) ? -speed : speed;
    if (_spd) _dtmin = 1000000 / _spd;
}

float Stepper::getSpeed(void)
{
    return _spd;
}

void Stepper::setAcceleration(float acc)
{
    _acc = (acc < 0) ? -acc : acc;
    if (_acc) _dt0 = 676000 * sqrt(2.0 / _acc);
}

float Stepper::getAcceleration(void)
{
    return _acc;
}

void Stepper::setDeceleration(float dec)
{
    _dec = (dec < 0) ? -dec : dec;
}

float Stepper::getDeceleration(void)
{
    return _dec;
}

void Stepper::setPosition(int position)
{
    _pos = position;
}

void Stepper::setPositionZero(void)
{
    _state = STOP;
    _prev_state = STOP;
    _pos = 0;
    _steps = 0;
    _spd = 0;
    _dt0 = 0;
    _n = 0;
    _nStartDec = 0;
    _dtn = 0;
    _dtmin = 0;
    _i = 0;
    _acc = 0;
    _dec = 0;
    _pauseRequested = false;
    _pauseActive = false;
    _pausedWasContinuous = false;
    _pauseOriginalTargetSteps = 0;
    _pausedResumeSteps = 0;
    _pauseDecOverride = -1.0f;
    _resumeAccOverride = -1.0f;
    _Pos_Cible_Done = 1;
}

int Stepper::getPosition(void)
{
    return _pos;
}

int Stepper::getStep(void)
{
    return _steps;
}

bool Stepper::stopped(void)
{
    return (_state == STOP);
}

bool Stepper::paused(void) const
{
    return _pauseActive;
}

bool Stepper::getPosCibleDone(void)
{
    return (_Pos_Cible_Done == 1);
}

void Stepper::run(void)
{
    handler();
}

void Stepper::stop(void)
{
    _clk = 0;
    remove();
    _state = STOP;
    _prev_state = STOP;
    _steps = 0;
    _pauseRequested = false;
    _pauseActive = false;
    _pausedWasContinuous = false;
    _pauseOriginalTargetSteps = 0;
    _pausedResumeSteps = 0;
    _pauseDecOverride = -1.0f;
    _resumeAccOverride = -1.0f;
}

void Stepper::rotate(bool direction)
{
    if (!_spd) return;

    _pauseRequested = false;
    _pauseActive = false;
    _pausedWasContinuous = false;
    _pauseOriginalTargetSteps = 0;
    _pausedResumeSteps = 0;
    _pauseDecOverride = -1.0f;

    _dir = direction;
    _dirPin = (_inv) ? !direction : direction;
    _steps = 0;
    _Pos_Cible_Done = 0;
    handler();
}

void Stepper::move(int steps)
{
    if (!steps || !_spd) return;

    _pauseRequested = false;
    _pauseActive = false;
    _pausedWasContinuous = false;
    _pauseOriginalTargetSteps = 0;
    _pausedResumeSteps = 0;
    _pauseDecOverride = -1.0f;

    if (steps < 0)
    {
        _dirPin = (_inv) ? CW : CCW;
        _dir = CCW;
        _steps = (unsigned int)(-steps);
    }
    else
    {
        _dirPin = (_inv) ? CCW : CW;
        _dir = CW;
        _steps = (unsigned int)steps;
    }

    _Pos_Cible_Done = 0;
    handler();
}

void Stepper::goesTo(int position)
{
    move(position - _pos);
}

void Stepper::pause(float deceleration)
{
    if (_state == STOP || _state == PAUSE || _pauseActive) return;

    _prev_state = _state;
    _pauseRequested = false;
    _pauseActive = true;
    _pausedWasContinuous = (_steps == 0);
    _pauseOriginalTargetSteps = _steps;
    _pausedResumeSteps = 0;
    _pauseDecOverride = (deceleration < 0.0f) ? -1.0f : deceleration;

    const float pauseDec = (_pauseDecOverride >= 0.0f) ? _pauseDecOverride : _dec;

    if (!pauseDec)
    {
        _clk = 0;
        remove();
        _state = PAUSE;
        _steps = 0;
        _pauseDecOverride = -1.0f;
        return;
    }

    _pauseRequested = true;
}

void Stepper::resume(float acceleration)
{
    if (_state != PAUSE || !_pauseActive) return;

    const bool wasContinuous = _pausedWasContinuous;
    const int resumeSteps = _pausedResumeSteps;

    _pauseRequested = false;
    _pauseActive = false;
    _pausedWasContinuous = false;
    _pauseOriginalTargetSteps = 0;
    _pausedResumeSteps = 0;
    _pauseDecOverride = -1.0f;
    _resumeAccOverride = (acceleration < 0.0f) ? -1.0f : acceleration;

    _n = 0;
    _i = 0;
    _dtn = 0;
    _state = STOP;

    if (wasContinuous)
    {
        rotate(_dir);
        return;
    }

    if (resumeSteps == 0)
    {
        _Pos_Cible_Done = 1;
        stop();
        return;
    }

    if (resumeSteps < 0)
    {
        _dirPin = (_inv) ? CW : CCW;
        _dir = CCW;
        _steps = (unsigned int)(-resumeSteps);
    }
    else
    {
        _dirPin = (_inv) ? CCW : CW;
        _dir = CW;
        _steps = (unsigned int)resumeSteps;
    }

    _Pos_Cible_Done = 0;
    handler();
}

void Stepper::handler(void)
{
    if (_state == PAUSE) return;

    if (_pauseRequested)
    {
        const float pauseDec = (_pauseDecOverride >= 0.0f) ? _pauseDecOverride : _dec;

        if (!pauseDec)
        {
            _pauseRequested = false;
            _pauseDecOverride = -1.0f;
            _clk = 0;
            remove();
            _state = PAUSE;
            _steps = 0;
            return;
        }

        float currentSpeed = (_dtn > 0) ? (1000000.0f / _dtn) : _spd;
        unsigned int decelSteps = nTo(currentSpeed, pauseDec);
        if (!decelSteps) decelSteps = 1;

        _steps = _n + decelSteps;
        _nStartDec = _n;
        _state = DECEL;
        _pauseRequested = false;
        _pauseDecOverride = -1.0f;
    }

    switch (_state)
    {
        case STOP:
        {
            _n = 0;
            const float startAcc = (_resumeAccOverride >= 0.0f) ? _resumeAccOverride : _acc;
            const unsigned int startDt0 = (startAcc > 0.0f) ? (unsigned int)(676000 * sqrt(2.0f / startAcc)) : 0;

            if (startDt0 <= _dtmin || !startAcc)
            {
                _dtn = _dtmin;
                _state = CRUISE;
            }
            else
            {
                _dtn = startDt0;
                _state = ACCEL;
            }

            if (_steps)
            {
                if (_dec && startAcc)
                {
                    unsigned int nToSpeed = nTo(_spd, startAcc);
                    _nStartDec = (_steps * _dec) / (_dec + startAcc);
                    if (_nStartDec > nToSpeed)
                        _nStartDec = _steps - ((nToSpeed * startAcc) / _dec);
                }
                else
                {
                    _nStartDec = _steps;
                }
            }
            _i = _dtn;
            _resumeAccOverride = -1.0f;
        }
        break;

        case ACCEL:
            _i -= _i * 2.0f / ((_n << 2) + 1);
            _dtn = _i;
            if ((unsigned int)_dtn <= _dtmin)
            {
                _dtn = _dtmin;
                _i = _dtn;
                _state = CRUISE;
            }
            if (_steps && _dec && _n >= _nStartDec) _state = DECEL;
        break;

        case CRUISE:
            if (_steps && _dec && _n >= _nStartDec) _state = DECEL;
        break;

        case DECEL:
            _i += (_i * 2.0f) / (((_steps - _n) << 2) + 1);
            _dtn = _i;
        break;

        case PAUSE:
        break;
    }

    _clk = 0;
    if (!_n) insert(_dtn + us_ticker_read());
    else insert(event.timestamp + (unsigned int)_dtn);

    _n++;
    _pos += (_dir << 1) - 1;
    _clk = 1;

    if (_steps && _n >= _steps)
    {
        if (_pauseActive)
        {
            if (!_pausedWasContinuous && _pauseOriginalTargetSteps > _n)
            {
                const int remaining = (int)(_pauseOriginalTargetSteps - _n);
                _pausedResumeSteps = (_dir == CW) ? remaining : -remaining;
            }
            else
            {
                _pausedResumeSteps = 0;
            }

            _clk = 0;
            remove();
            _state = PAUSE;
            _steps = 0;
        }
        else
        {
            _Pos_Cible_Done = 1;
            stop();
        }
    }
}

unsigned int Stepper::nTo(float speed, float acc)
{
    if (speed < 0) speed = -speed;
    if (acc < 0) acc = -acc;
    return (!acc || !speed) ? 0 : (speed * speed) / (2 * acc);
}
