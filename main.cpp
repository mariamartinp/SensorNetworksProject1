/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Application helpers
#include "DummySensor.h"
#include "trace_helper.h"
#include "lora_radio_helper.h"

//Libraries to use the sensors
#include "mbed.h"
#include "MMA8451Q.h"
#include "TCS3472_I2C.h"
#include "Si7021.h"
#include "MBed_Adafruit_GPS.h"
#include "RGBLed.h"

using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        10000

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
 * Dummy pin for dummy sensor
 */
#define PC_9                            0

/**
 * Dummy sensor class object
 */
DS1820  ds1820(PC_9);

/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;
static uint8_t DEV_EUI[] = { 0x7E, 0x39, 0x32, 0x35, 0x59, 0x37, 0x91, 0x94};
static uint8_t APP_EUI[] = { 0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0xfc, 0xda };
static uint8_t APP_KEY[] = { 0xf3,0x1c,0x2e,0x8b,0xc6,0x71,0x28,0x1d,0x51,0x16,0xf0,0x8f,0xf0,0xb7,0x92,0x8f };

/**
 * VARIABLES TO MONITOR THE PLANT
 */
//Sensors
AnalogIn input_light(PA_4);
AnalogIn soil(PA_0);
MMA8451Q acc(PB_9, PB_8, 0x1d<<1);
TCS3472_I2C rgb_sensor (PB_9, PB_8);
DigitalOut led_rgbsensor(PB_7);
int rgb_readings[4];
Si7021 tempHumSensor(PB_9, PB_8);
RGBLed rgbled(PH_0, PH_1, PB_13);
BufferedSerial *gps_Serial = new BufferedSerial(PA_9, PA_10,9600);
Adafruit_GPS myGPS(gps_Serial); 

//Threads
Thread threadAnalog(osPriorityNormal, 512);
Thread threadI2C(osPriorityNormal, 512);
Thread threadGPS(osPriorityNormal, 2048);
int wait_time = 2000000;

//Sensors values
signed short light_value;
float x_value, y_value, z_value, moisture_value;
int clear, red, green, blue;
float temp_value, hum_value;
float latitude, longitude;
int temp, hum, moisture, light, colour; //2byte

/**
 * Entry point for application
 */
 ///////////THREADS//////////////
void analogValues(void){
	while (1){
		//LIGHT (%)
		light_value = (input_light.read_u16() * 100)/4000; 
		if(light_value>=100){light_value = 100;} 
		light = (int)light_value;
		//SOIL MOISTURE
		moisture_value = soil * 100;
		if(moisture_value>=100){moisture_value = 100;}
		moisture = (int) moisture_value;
		wait_us(wait_time);
		
	}
}
void gpsRead(void){
	while(1){
		myGPS.read();
		if (myGPS.newNMEAreceived() ) {
			if (!myGPS.parse(myGPS.lastNMEA()) ) {
					continue;
			}
		}
	}
	wait_us(wait_time);
}
void i2cValues(void){
	while (1){
		//ACCELEROMETER
		x_value = acc.getAccX();
		y_value = acc.getAccY();
		z_value = - acc.getAccZ();
		
		//TEMPERATURE
		tempHumSensor.get_data();
		temp_value = tempHumSensor.get_temperature();
		temp_value = temp_value/1000;
		temp = (int)temp_value;
		
		//HUMIDITY (%)
		hum_value = tempHumSensor.get_humidity();
		hum_value = hum_value/1000;
		if(hum_value>=100){hum_value = 100;}
		hum = (int)hum_value;
		
		//RGB SENSOR
		rgb_sensor.setIntegrationTime( 100 ); 
		rgb_sensor.getAllColors( rgb_readings );
		red = rgb_readings[1];
		green = rgb_readings[2];
		blue = rgb_readings[3];
		clear = rgb_readings[0];
		
		wait_us(wait_time);
	}
}
////////////END THREADS////////////
void gpsInit(){
	myGPS.begin(9600);
	myGPS.sendCommand(PMTK_SET_NMEA_OUTPUT_GGA); 
	myGPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
	myGPS.sendCommand(PGCMD_ANTENNA);
	printf("Connection established at 9600 baud...\r\n");
	wait_us(1000000);
}
void dominantColour(){
	if((red>green) && (red>blue)){
		colour = 0;
	}
	else if( (green>red) && (green>blue)){ 
		colour = 1;
	}
	else{ 
		colour = 2;
	}
}
 
int main(void)
{
		// setup tracing
    setup_trace();

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;
	
		//Previous code
		gpsInit();
	
		threadGPS.start(gpsRead);
		threadAnalog.start(analogValues);
		threadI2C.start(i2cValues);

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
            != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");
    lorawan_connect_t connect_params;
		connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
    connect_params.connection_u.otaa.dev_eui = DEV_EUI;
    connect_params.connection_u.otaa.app_eui = APP_EUI;
    connect_params.connection_u.otaa.app_key = APP_KEY;
    connect_params.connection_u.otaa.nb_trials = 3;
		
    retcode = lorawan.connect(connect_params);

    if (retcode == LORAWAN_STATUS_OK ||
            retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}

/**
 * Sends a message to the Network Server
 * (Including the sensor values)
 */
static void send_message()
{
    uint16_t packet_len;
    int16_t retcode;
    int32_t sensor_value;
		dominantColour();
	
		if(myGPS.fixquality == 0){
			latitude = 40.23;
			longitude = -3.37;
		}else{
			if(myGPS.lat == 'S'){
				latitude = - myGPS.latitude/100;
			}else if(myGPS.lat == 'N'){
				latitude = myGPS.latitude/100;
			}
			if(myGPS.lon == 'W'){
				longitude = - myGPS.longitude/100;
			}else if(myGPS.lon == 'E'){
				longitude = myGPS.longitude/100;
			}
		}
	
    /*if (ds1820.begin()) {
        ds1820.startConversion();
        sensor_value = ds1820.read();
        printf("\r\n Dummy Sensor Value = %d \r\n", sensor_value);
        ds1820.startConversion();
    } else {
        printf("\r\n No sensor found \r\n");
        return;
    }

    packet_len = sprintf((char *) tx_buffer, "%d",
                         sensor_value);
		*/
				
		memcpy(&tx_buffer[0],&latitude, sizeof(float));
		memcpy(&tx_buffer[4],&longitude, sizeof(float));
		memcpy(&tx_buffer[8],&temp, sizeof(int));
		memcpy(&tx_buffer[10],&hum, sizeof(int));
		memcpy(&tx_buffer[12],&light, sizeof(int));
		memcpy(&tx_buffer[14],&moisture, sizeof(int));
		memcpy(&tx_buffer[16],&x_value, sizeof(float));
		memcpy(&tx_buffer[20],&y_value, sizeof(float));
		memcpy(&tx_buffer[24],&z_value, sizeof(float));
		memcpy(&tx_buffer[28],&colour, sizeof(int));
		packet_len = 30;
		
		//To print the payload
		for (int i=0; i<30; i++){
			printf("%02x",tx_buffer[i]);
		}
		
		//To check the measured values and see them in the screen
		printf("\n");
		printf("quality: %i, lat: %f, long: %f\n", myGPS.fixquality, latitude, longitude);
		printf("temp: %i, hum: %i, light: %i, moisture: %i, colour: %i\n", temp, hum, light, moisture, colour);
		printf("xaxis: %f, yaxis: %f, zaxis: %f", x_value, y_value, z_value);
		
    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_UNCONFIRMED_FLAG);
		//retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
      //                     MSG_CONFIRMED_FLAG);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
        : printf("\r\n send() - Error code %d \r\n", retcode);

        if (retcode == LORAWAN_STATUS_WOULD_BLOCK) {
            //retry in 3 seconds
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                ev_queue.call_in(3000, send_message);
            }
        }
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
		for (uint8_t i = 0; i < retcode; i++) {
        printf("%02x ", rx_buffer[i]);
		}
		//To check the colour send from resiot to the platform
		if(strcmp((char *)rx_buffer,"GREEN")==0){
			rgbled.setColor(RGBLed::GREEN);
			printf("LED PRINTED AS GREEN\n");
		}else if(strcmp((char *)rx_buffer,"RED")==0){
			rgbled.setColor(RGBLed::RED);
			printf("LED PRINTED AS RED\n");
		}else if(strcmp((char *)rx_buffer,"OFF")==0){
			rgbled.setColor(RGBLed::BLACK);
			printf("LED OFF\n");
		}
    printf("\r\n");
    
    memset(rx_buffer, 0, sizeof(rx_buffer));
}



/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            } else {
                ev_queue.call_every(TX_TIMER, send_message);
            }

            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
