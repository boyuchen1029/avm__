#include "UART_Reader.h"

#include <iostream>
#include <bitset>
#include <sstream>
#include <system.h>

#define VEHICLE_TYPE __SIENTA__
UART_Reader::UART_Reader() {}
int m_gear  = 0;
float m_speed = 0;
float m_steeringangle = 0;
int m_door = 0;
bool UART_Reader::read_and_decode_data(const std::string& data, uint32_t& result) {
    // if (data.length() != 8) {
    //     std::cerr << "Data length is incorrect. Expected 8 characters." << std::endl;
    //     return false;
    // }
#if (VEHICLE_TYPE == __URX__)
    std::string hex_string_gear  = data.substr(0, 2);
    std::string hex_string_speed = data.substr(2, 4);
    std::string hex_string_angle = data.substr(6, 4);
    std::string hex_string_crc   = data.substr(10, 2);
    //char termination_bit = data[8];

    // Convert the hex string to a uint32_t
    uint32_t _gear = 0;
    uint32_t _speed = 0;
    uint32_t _angle = 0;
    uint32_t _sum = 0;

    std::stringstream ss;
    ss << std::hex << hex_string_gear;
    ss >> _gear;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_speed;
    ss >> _speed;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_angle;
    ss >> _angle;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_crc;
    ss >> _sum;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    AVM_LOGI("speed %s\n",hex_string_speed.c_str());
    AVM_LOGI("U32angle %u\n",_speed);
    int low1  = (_speed)>>8;
    int High1 = (_speed & 0x00FF);
    AVM_LOGI("LowByte %d\n" ,low1);
    AVM_LOGI("HighByte %d\n",High1);
    int tmp_speed = High1 * 255 + low1;

    
    AVM_LOGI("angle %s\n",hex_string_angle.c_str());
    AVM_LOGI("U32angle %u\n",_angle);
    int low  = (_angle)>>8;
    int High = (_angle & 0x00FF);
    AVM_LOGI("LowByte %d\n" ,low);
    AVM_LOGI("HighByte %d\n",High);
    int tmp_angle = High * 255 + low;
    
    uint32_t cur_pake_crc = (uint32_t)(255 - (uint32_t)(_gear  + low + High + low1 + High1)/5 + 1);
    AVM_LOGI("cur_pake_crc %u\n",cur_pake_crc);
    AVM_LOGI("_sum %u\n",_sum);
    if(_sum != cur_pake_crc) 
    {
        AVM_LOGE("Uart CRC error\n");
        return false;
    }

    m_gear          =  _gear;
    m_speed         =  (float)tmp_speed/100.0f;
    m_steeringangle =  ((float)tmp_angle - 3000.0f)/100.0f;
    result          =  m_gear;
#else
    std::string hex_string_gear  = data.substr(4, 2);
    std::string hex_string_speed = data.substr(6, 4);
    std::string hex_string_angle = data.substr(10, 4);
    std::string hex_string_door   = data.substr(14, 2);
    std::string hex_string_crc   = data.substr(16, 2);
    uint32_t _gear = 0;
    uint32_t _speed = 0;
    uint32_t _angle = 0;
    uint32_t _door = 0;
    uint32_t _sum = 0;
    std::stringstream ss;
    ss << std::hex << hex_string_gear;
    ss >> _gear;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_speed;
    ss >> _speed;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_angle;
    ss >> _angle;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_door;
    ss >> _door;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    ss << std::hex << hex_string_crc;
    ss >> _sum;
    ss.clear();  // 清空状态
    ss.str("");  // 清空流内容

    //("speed %s\n",hex_string_speed.c_str());
    //AVM_LOGI("U32angle %u\n",_speed);
    int low1  = (_speed)>>8;
    int High1 = (_speed & 0x00FF);
    //AVM_LOGI("LowByte %d\n" ,low1);
    //AVM_LOGI("HighByte %d\n",High1);
    int tmp_speed = High1 * 255 + low1;

    
    // AVM_LOGI("angle %s\n",hex_string_angle.c_str());
    // AVM_LOGI("U32angle %u\n",_angle);
    int low  = (_angle)>>8;
    int High = (_angle & 0x00FF);
    // AVM_LOGI("LowByte %d\n" ,low);
    // AVM_LOGI("HighByte %d\n",High);
    int tmp_angle = High * 255 + low;
    
    uint32_t cur_pake_crc = (uint32_t)(255 - (uint32_t)(_gear  + low + High + low1 + High1 + _door)/6 + 1);
    // AVM_LOGI("cur_pake_crc %u\n",cur_pake_crc);
    // AVM_LOGI("_sum %u\n",_sum);
    if(_sum != cur_pake_crc) 
    {
        AVM_LOGE("Uart CRC error\n");
        return false;
    }

    m_gear          =  _gear;
    m_speed         =  (float)tmp_speed/100.0f;
    m_steeringangle =  ((float)tmp_angle - 4000.0f)/10.0f;
    m_door = _door;
    result          =  m_gear;
#endif
    return true;
}

uint8_t UART_Reader::calculate_checksum(uint32_t data) {
    uint8_t sum = 0;
    sum += (data >> 24) & 0xFF; 
    sum += (data >> 16) & 0xFF; 
    sum += (data >> 8) & 0xFF;         
    return ~sum + 1;
}

void UART_Reader::process_data(uint32_t data) {
    // uint8_t temp_gear  = (data >> 24) & 0xFF;  // 前 8 位
    // uint8_t temp_angle = (data >> 16) & 0xFF;  // 前 8 位
    // uint8_t temp_speed = (data >> 8) & 0xFF;        // 後 8 位
    // std::cout << "Gear (decimal): " << std::dec << static_cast<int>(temp_gear) << std::endl;
    // std::cout << "Speed (decimal): " << std::dec << static_cast<int>(temp_speed) << std::endl;
    // m_gear          = static_cast<int>(temp_gear);
    // m_steeringangle = static_cast<int>(temp_angle);
    // m_speed         = static_cast<int>(temp_speed);
}

#if (VEHICLE_TYPE == __URX__)
void UART_Reader::get_seethrough_data(int &gear, float &speed, float &steeringangle)
{
    gear = m_gear;
    speed = m_speed;
    steeringangle = m_steeringangle;
}
#else
void UART_Reader::get_seethrough_data(int &gear, float &speed, float &steeringangle,int &door)
{
    gear  = m_gear;
    speed = m_speed;
    steeringangle = m_steeringangle;
    door = m_door;
}
#endif
