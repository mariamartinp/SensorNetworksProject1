#ifndef SI7021_H
#define SI7021_H
 
#include "mbed.h"
 
 
#define READ_TEMP        0xE0   // Read Temperature Command
#define READ_HUM         0xE5   //  Read RH Command
 
/***** Si7012 Read ID *****/
#define READ_ID1_1       0xFA
#define READ_ID1_2       0x0F
#define READ_ID2_1       0xFC
#define READ_ID2_2       0xC9
 
#define ADDR    0x80            //Device address
#define FREQ    100000          //Device frequency
#define DEVICE_ID 0x15          //Device id value
 
class Si7021
{
    public:
    
        Si7021(PinName sda, PinName scl);
        int32_t get_temperature();  // Get last measured temperature data (ºC)
        uint32_t get_humidity();    // Get last measured relative humidity data (%)
        bool get_data();             // Perform measurement, return: 0 if successful
        bool check();               // Check if the sensor is active
        
    private:
    
        I2C i2c;
        uint8_t  buff_rx[8];
        uint8_t  buff_tx[2];
        uint32_t humData;
        int32_t  tempData;
};
 
#endif