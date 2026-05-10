#ifndef COMMAND_ASSERV_HPP
#define COMMAND_ASSERV_HPP

#include "mbed.h"

/******************************************************************************
 * CommandAsserv UART Protocol
 *
 * Commands format:
 * ------------------------------------------------------------
 * <ID> <Param0> <Param1> ... <ParamN>\n
 *
 * Example:
 * ------------------------------------------------------------
 * G 1000 500\n
 *
 * All commands are ASCII based and terminated by '\n'
 *
 ******************************************************************************/

/******************************************************************************
 * Commands table
 * ---------------------------------------------------------------------------
 * Identifier | Name            | Description
 * ---------------------------------------------------------------------------
 * S          | Stop            | Stop the control loop at current position
 *            |                 | and clear current waypoint.
 *
 * P          | Pause           | Pause current action and hold position.
 *
 * R          | Resume          | Resume a paused action.
 *
 * G X Y      | GoTo            | Add an absolute waypoint.
 *
 * M X Y      | Move            | Add a relative waypoint.
 *
 * A Alpha    | Rotate          | Rotate robot to a fixed angle.
 *
 * E          | Status          | Request current control loop status.
 *
 * B          | Broadcast       | Enable / Disable position broadcasting.
 *
 * C X Y A    | Set Position    | Set current robot position.
 ******************************************************************************/

/******************************************************************************
 * Status table
 * ---------------------------------------------------------------------------
 * Identifier | Name            | Description
 * ---------------------------------------------------------------------------
 * O          | Ok              | Previous command executed successfully.
 *
 * F          | Fail            | Previous command failed.
 *
 * A          | Arrived         | Waypoint completed.
 *
 * S          | Status          | Current control loop status:
 *            |                 | STOPPED / MOVING / PAUSED
 *
 * B          | Broadcast       | Current position and velocity packet.
 *
 ******************************************************************************/

/******************************************************************************
 * Broadcast packet format
 * ---------------------------------------------------------------------------
 * B X Y Alpha VX VY VAlpha\n
 *
 * Example:
 * ---------------------------------------------------------------------------
 * B 1200 450 90 100 0 5\n
 *
 ******************************************************************************/

class CommandAsserv
{
public:
    enum Result
    {
        RESULT_NONE,
        RESULT_OK,
        RESULT_FAIL,
        RESULT_ARRIVED,
        RESULT_TIMEOUT,
        RESULT_STOPPED
    };

    enum ControlStatus
    {
        STATUS_UNKNOWN = 0,
        STATUS_STOPPED,
        STATUS_MOVING,
        STATUS_PAUSED
    };

    struct BroadcastData
    {
        int x;
        int y;
        int alpha;

        int vx;
        int vy;
        int valpha;

        bool valid;
    };

    CommandAsserv(PinName tx, PinName rx, int baudrate = 115200);

    bool stop();
    bool pause();
    bool resume();

    bool goTo(int x, int y);
    bool move(int x, int y);
    bool rotate(int alpha);

    bool setPosition(int x, int y, int alpha);

    bool enableBroadcast();
    bool requestStatus();

    BroadcastData getBroadcast();

    int getX();
    int getY();
    int getAlpha();

    int getVX();
    int getVY();
    int getVAlpha();

    ControlStatus getStatus();

private:
    BufferedSerial _uart;
    Thread _rxThread;

    Mutex _uartMutex;
    Mutex _broadcastMutex;

    volatile bool _stopWaitArrived;
    volatile Result _lastResponse;
    volatile ControlStatus _controlStatus;

    BroadcastData _broadcast;

private:
    bool sendAndWaitAck(const char* cmd);
    bool sendAndWaitAckThenArrived(const char* cmd);

    bool waitArrived();

    void rxTask();
    void parseLine(const char* line);

    void flushRx();
};

#endif