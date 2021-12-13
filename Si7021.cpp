#include "Si7021.h"
 
Si7021::Si7021(PinName sda, PinName scl):i2c(sda, scl)
{
    i2c.frequency(FREQ);
}
    
int32_t Si7021::get_temperature()
{
    return tempData;
}
 
 
uint32_t Si7021::get_humidity()
{
    return humData;
}
 
bool Si7021::get_data()
{
    buff_tx[0] = READ_HUM  ;
    if(i2c.write(ADDR, (char*)buff_tx, 1) != 0) return 0;
    if(i2c.read(ADDR, (char*)buff_rx, 2) != 0) return 0;
    
    humData = ((uint32_t)buff_rx[0] << 8) + (buff_rx[1] & 0xFC);
    humData = (((humData) * 15625L) >> 13) - 6000;
    
    buff_tx[0] = READ_TEMP;
    if(i2c.write(ADDR, (char*)buff_tx, 1) != 0) return 0;
    if(i2c.read(ADDR, (char*)buff_rx, 2) != 0) return 0;
    
    tempData = ((uint32_t)buff_rx[0] << 8) + (buff_rx[1] & 0xFC);
    tempData = (((tempData) * 21965L) >> 13) - 46850;
    
    return 1;
}
 
bool Si7021::check()
{
    buff_tx[0] = READ_ID2_1;
    buff_tx[1] = READ_ID2_2;
    if(i2c.write(ADDR, (char*)buff_tx, 2) != 0) return 0;
    if(i2c.read(ADDR, (char*)buff_rx, 8) != 0) return 0;
    
    if(buff_rx[0] == DEVICE_ID)
        return true;
    else return 0;
}