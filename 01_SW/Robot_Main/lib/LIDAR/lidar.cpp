#include "lidar.hpp"

Lidar::Lidar(PinName tx, PinName rx, int baud)
    : serial(tx, rx, baud), buffer_pos(0)
{
    serial.attach(callback(this, &Lidar::on_uart_rx), SerialBase::RxIrq);
    lidar_thread.start(callback(&queue, &EventQueue::dispatch_forever));
    //Lidar_print_thread.start(callback(this, &Lidar::routine_lidar));
}

void Lidar::on_uart_rx()
{
    // Lecture des données dispo
    while (serial.readable()) {
        uint8_t c;
        if (serial.read(&c, 1) == 1) {
            if (buffer_pos < sizeof(buffer)) {
                buffer[buffer_pos++] = c;
            }
            
            if (buffer_pos >= DATA_PACKET_SIZE) {
                queue.call(callback(this, &Lidar::processBuffer));
            }
        }
    }
}

void Lidar::processBuffer()
{
    const size_t max_scan = buffer_pos > DATA_PACKET_SIZE ? buffer_pos - DATA_PACKET_SIZE : 0;

    for (size_t i = 0; i < max_scan; ++i)
    {
        // Cherche début de paquet
        if (buffer[i] == HEADER && buffer[i + 1] == VERLEN)
        {
            // Vérifie qu'on a assez de données pour un paquet complet + début du suivant
            if (i + DATA_PACKET_SIZE + 2 > buffer_pos)
                continue;

            // Vérifie que la trame suivante commence bien avec HEADER + VERLEN
            if (buffer[i + DATA_PACKET_SIZE] != HEADER || buffer[i + DATA_PACKET_SIZE + 1] != VERLEN)
                continue;

            // Vérifie qu'il n'y a PAS d'autre HEADER+VERLEN dans la trame actuelle
            bool valid = true;
            for (size_t j = i + 2; j < i + DATA_PACKET_SIZE - 1; ++j)
            {
                if (buffer[j] == HEADER && buffer[j + 1] == VERLEN)
                {
                    valid = false;
                    break;
                }
            }

            if (!valid)
                continue;

            // Paquet OK, on copie et traite
            memcpy(packet_buffer, &buffer[i], DATA_PACKET_SIZE);
            memmove(buffer, &buffer[i + DATA_PACKET_SIZE], buffer_pos - (i + DATA_PACKET_SIZE));
            buffer_pos -= (i + DATA_PACKET_SIZE);

            parsePacket(packet_buffer);
            return;
        }
    }

    // Sécurité : si buffer trop rempli sans valid packet
    if (buffer_pos > sizeof(buffer) - DATA_PACKET_SIZE) {
        buffer_pos = 0;
    }
}
void Lidar::parsePacket(const uint8_t* DataPacket)
{
    _dataPacket.header      = DataPacket[0];
    _dataPacket.ver_len     = DataPacket[1];
    _dataPacket.speed       = ((uint16_t)DataPacket[3] << 8) | DataPacket[2];
    _dataPacket.start_angle = ((uint16_t)DataPacket[5] << 8) | DataPacket[4];
    _dataPacket.end_angle   = ((uint16_t)DataPacket[43] << 8) | DataPacket[42];
    _dataPacket.timestamp   = ((uint16_t)DataPacket[45] << 8) | DataPacket[44];
    _dataPacket.crc8        = DataPacket[46];
    // printf("\nDATA_PACKET: ");
    // for (int i = 0; i < POINT_PER_PACK; i++) {
    //   printf("%02X",DataPacket[i]);
    // }
    //  printf("\r\n");


    _dataPacket.start_angle           = _dataPacket.start_angle % 36000; //modulo 360°
    _dataPacket.end_angle             = _dataPacket.end_angle   % 36000; //modulo 360°
    float step = 0.0f;
    int delta = (int)_dataPacket.end_angle - (int)_dataPacket.start_angle;

    if (delta > 0) {
      step = float(delta) / float(POINT_PER_PACK - 1);
    } 
    else {
      step = float((36000 + delta)) / float(POINT_PER_PACK - 1);
    }
    // if(step >100){
    //    printf("step=%f,end_angle=%5d start_angle=%5d, \r\n",step,_dataPacket.end_angle/100,_dataPacket.start_angle/100);
    // }
   
    if(step <=100){
      for (uint8_t i = 0; i < POINT_PER_PACK; ++i) {
          uint8_t offset = 6 + 3 * i;
          _dataPacket.point[i].distance  = ((uint16_t)DataPacket[offset + 1] << 8) | DataPacket[offset];
          _dataPacket.point[i].intensity = DataPacket[offset + 2];

          float angle = _dataPacket.start_angle + (step * i);
          _dataPacket.point[i].angle = int(angle) % 36000;

          //printf("Step=%f Angle=%f\n",step,_dataPacket.point[i].angle/100.0f); 
      }
    }
}

LiDARFrameTypeDef Lidar::GetPoints(void)
{
    return _dataPacket;
}

// void Lidar::routine_lidar(void)
// {
//   while (true) {
//     // Récupérer la dernière trame
//     LiDARFrameTypeDef frame = Lidar::GetPoints();

//     // Affichage simple des distances
//     //printf("Start angle: %.2f deg\n", frame.start_angle / 100.0f);
//     for (int i = 0; i < POINT_PER_PACK; i++) {
//         printf("Point %d: angle=%.2f deg, distance=%d mm, intensity=%d\n",
//                 i,
//                 frame.point[i].angle / 100.0f,
//                 frame.point[i].distance,
//                 frame.point[i].intensity);

//         //printf("%.2f;%5d\r\n",frame.point[i].angle/100.0f,frame.point[i].distance);
//     }
//     printf("-----\n");

    

//     ThisThread::sleep_for(1ms); 
//   }   
// }