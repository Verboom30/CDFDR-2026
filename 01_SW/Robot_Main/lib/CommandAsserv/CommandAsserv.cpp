#include "CommandAsserv.hpp"
#include <cstdio>
#include <cstring>

CommandAsserv::CommandAsserv(PinName tx, PinName rx, int baudrate)
    : _uart(tx, rx, baudrate),
      _stopWaitArrived(false),
      _lastResponse(RESULT_NONE),
      _controlStatus(STATUS_UNKNOWN)
{
    _uart.set_blocking(false);

    _broadcast.x = 0;
    _broadcast.y = 0;
    _broadcast.alpha = 0;
    _broadcast.vx = 0;
    _broadcast.vy = 0;
    _broadcast.valpha = 0;
    _broadcast.valid = false;

    _rxThread.start(callback(this, &CommandAsserv::rxTask));
}

bool CommandAsserv::stop()
{
    _stopWaitArrived = true;
    return sendAndWaitAck("S\n");
}

bool CommandAsserv::pause()
{
    return sendAndWaitAck("P\n");
}

bool CommandAsserv::resume()
{
    return sendAndWaitAck("R\n");
}

bool CommandAsserv::goTo(int x, int y)
{
    char buffer[64];

    sprintf(buffer, "G %d %d\n", x, y);

    return sendAndWaitAckThenArrived(buffer);
}

bool CommandAsserv::move(int x, int y)
{
    char buffer[64];

    sprintf(buffer, "M %d %d\n", x, y);

    return sendAndWaitAckThenArrived(buffer);
}

bool CommandAsserv::rotate(int alpha)
{
    char buffer[32];

    sprintf(buffer, "A %d\n", alpha);

    return sendAndWaitAckThenArrived(buffer);
}

bool CommandAsserv::setPosition(int x, int y, int alpha)
{
    char buffer[64];

    sprintf(buffer, "C %d %d %d\n", x, y, alpha);

    return sendAndWaitAck(buffer);
}

bool CommandAsserv::enableBroadcast()
{
    return sendAndWaitAck("B\n");
}

bool CommandAsserv::requestStatus()
{
    return sendAndWaitAck("E\n");
}

bool CommandAsserv::sendAndWaitAckThenArrived(const char* cmd)
{
    _stopWaitArrived = false;

    if (!sendAndWaitAck(cmd))
        return false;

    return waitArrived();
}

bool CommandAsserv::sendAndWaitAck(const char* cmd)
{
    {
        ScopedLock<Mutex> lock(_uartMutex);

        _lastResponse = RESULT_NONE;

        flushRx();

        _uart.write(cmd, strlen(cmd));
    }

    Timer timer;
    timer.start();

    while (timer.elapsed_time().count() < 300000)
    {
        if (_lastResponse == RESULT_OK)
            return true;

        if (_lastResponse == RESULT_FAIL)
            return false;

        ThisThread::sleep_for(1ms);
    }

    return false;
}

bool CommandAsserv::waitArrived()
{
    while (true)
    {
        if (_stopWaitArrived)
            return false;

        if (_lastResponse == RESULT_ARRIVED)
            return true;

        if (_lastResponse == RESULT_FAIL)
            return false;

        ThisThread::sleep_for(1ms);
    }
}

void CommandAsserv::rxTask()
{
    char line[128];
    size_t index = 0;

    while (true)
    {
        char c;

        if (_uart.read(&c, 1) == 1)
        {
            if (c == '\r')
                continue;

            if (c == '\n')
            {
                line[index] = '\0';

                parseLine(line);

                index = 0;
            }
            else
            {
                if (index < sizeof(line) - 1)
                {
                    line[index++] = c;
                }
                else
                {
                    index = 0;
                }
            }
        }

        ThisThread::sleep_for(1ms);
    }
}

void CommandAsserv::parseLine(const char* line)
{
    if (line[0] == 'O')
    {
        _lastResponse = RESULT_OK;
        return;
    }

    if (line[0] == 'F')
    {
        _lastResponse = RESULT_FAIL;
        return;
    }

    if (line[0] == 'A')
    {
        _lastResponse = RESULT_ARRIVED;
        return;
    }

    if (line[0] == 'B')
    {
        BroadcastData data;

        int ret = sscanf(line,
                         "B %d %d %d %d %d %d",
                         &data.x,
                         &data.y,
                         &data.alpha,
                         &data.vx,
                         &data.vy,
                         &data.valpha);

        if (ret == 6)
        {
            data.valid = true;

            ScopedLock<Mutex> lock(_broadcastMutex);
            _broadcast = data;
        }

        return;
    }

    if (line[0] == 'S')
    {
        if (strstr(line, "STOPPED"))
        {
            _controlStatus = STATUS_STOPPED;
        }
        else if (strstr(line, "MOVING"))
        {
            _controlStatus = STATUS_MOVING;
        }
        else if (strstr(line, "PAUSED"))
        {
            _controlStatus = STATUS_PAUSED;
        }
        else
        {
            _controlStatus = STATUS_UNKNOWN;
        }

        return;
    }
}

void CommandAsserv::flushRx()
{
    char c;

    while (_uart.read(&c, 1) == 1)
    {
    }
}

CommandAsserv::BroadcastData CommandAsserv::getBroadcast()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast;
}

int CommandAsserv::getX()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast.x;
}

int CommandAsserv::getY()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast.y;
}

int CommandAsserv::getAlpha()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast.alpha;
}

int CommandAsserv::getVX()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast.vx;
}

int CommandAsserv::getVY()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast.vy;
}

int CommandAsserv::getVAlpha()
{
    ScopedLock<Mutex> lock(_broadcastMutex);
    return _broadcast.valpha;
}

CommandAsserv::ControlStatus CommandAsserv::getStatus()
{
    return _controlStatus;
}