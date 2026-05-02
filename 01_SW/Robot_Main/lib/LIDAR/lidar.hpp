#ifndef LIDAR_HPP
#define LIDAR_HPP

#include "mbed.h"

#define DATA_PACKET_SIZE 47
#define POINT_PER_PACK   12
#define HEADER           0x54
#define VERLEN           0x2C

typedef struct {
    uint16_t distance;
    uint8_t  intensity;
    uint16_t angle;
} LiDARPointTypeDef;

typedef struct {
    uint8_t  header;
    uint8_t  ver_len;
    uint16_t speed;
    uint16_t start_angle;
    LiDARPointTypeDef point[POINT_PER_PACK];
    uint16_t end_angle;
    uint16_t timestamp;
    uint8_t  crc8;
} LiDARFrameTypeDef;

class Lidar {
public:
    Lidar(PinName tx, PinName rx, int baud);
    LiDARFrameTypeDef GetPoints(void);

private:
    UnbufferedSerial serial;
    EventQueue queue;
    Thread lidar_thread;

    uint8_t buffer[DATA_PACKET_SIZE * 4];
    size_t buffer_pos;
    uint8_t packet_buffer[DATA_PACKET_SIZE] = {0};

    LiDARFrameTypeDef _dataPacket;

    void on_uart_rx();
    void processBuffer();
    void parsePacket(const uint8_t* DataPacket);

    // Thread Lidar_print_thread;
    // void routine_lidar(void);
};

#endif // LIDAR_HPP