#ifndef TCS3472_I2C_H
#define TCS3472_I2C_H
#include "mbed.h"
 
//Defines 
#define SLAVE_ADDRESS           0x29
 
#define ENABLE                  0x00
#define ATIME                   0x01
#define WTIME                   0x03
#define AILTL                   0x04
#define AIHTL                   0x06
#define PERS                    0x0C
#define CONFIG                  0x0D
#define CONTROL                 0x0F
#define ID                      0x12
#define STATUS                  0x13
#define CDATA                   0x14
#define RDATA                   0x16
#define GDATA                   0x18
#define BDATA                   0x1A
 
/** TCS3472_I2C class.
 *  Used to read to and write from TCS3472 color sensor.
 */
class TCS3472_I2C {
public:
    /** Create TCS3472_I2C instance
     *
     * @param sda sda pin for I2C
     * @param scl scl pin for I2C
     */
    TCS3472_I2C( PinName sda, PinName scl );
    
    /** Destructor
     */
    ~TCS3472_I2C();
    
    /** Read red, green, blue and clear values into array
     *
     * @param readings Array of four integers to store the read data
     */
		void getAllColors( int* readings );
    
    /** Read clear data
     *
     * @returns
     *     Clear data reading
     */
		int getClearData();
    
    /** Read red data
     *
     * @returns
     *     Red data reading
     */
    int getRedData();
    
    /** Read green data
     *
     * @returns
     *     Green data reading
     */
    int getGreenData();
    
    /** Read blue data
     *
     * @returns
     *     Blue data reading
     */
    int getBlueData();
    
    /** Activates internal oscillator and two-channel ADC simultaneously (both necessary for standard operation).
     *
     * @returns
     *     1 if successful
     *     0 if otherwise
     */
    int enablePowerAndRGBC();
    
    /** Sets integration time.
     *
     * @param itime Integration time to set in milliseconds. Should be in the range 2.4 - 614.4ms.
     *
     * @returns
     *     1 if successful
     *     0 if otherwise
     */
		int setIntegrationTime( const float itime );
     
private:
    I2C i2c;
    
    int writeSingleRegister( char address, char data );
    char readSingleRegister( char address );
    int readMultipleRegisters( char address, char* output, int quantity );
    
    float roundTowardsZero( const float value );
};
 
#endif