#ifndef UART_READER_H
#define UART_READER_H

#include <string>
#include <cstdint>

#define Prefix_code 0x27
#define __SIENTA__ 1
#define __URX__ 2
class UART_Reader {
public:
    UART_Reader();
    bool read_and_decode_data(const std::string& data, uint32_t& result);
    #if (VEHICLE_TYPE == __URX__)
    void get_seethrough_data(int &gear, float &speed,  float &steeringangle);
    #else
    void get_seethrough_data(int &gear, float &speed, float &steeringangle,int &door);
    #endif
private:
    uint8_t calculate_checksum(uint32_t data);
    void process_data(uint32_t data);
};

#endif // UART_READER_H
