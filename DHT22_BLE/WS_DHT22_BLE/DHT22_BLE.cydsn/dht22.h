/* ========================================
 * Filename:        dht22.h
 * Description:     DHT22 Driver for PSoC header file
 * Author:          techdude101
 * Version:         0.1.0
 * ========================================
*/
#include <stdint.h>

#ifndef __DHT22_H
#define __DHT22_H 
    int     DHTread(void);
    uint8_t DHT22_Init(void);			                // Initialize DHT22
    uint8_t DHT22_Read_Data(uint8_t *temp);	            // Read temperature and humidity
    uint8_t DHT22_Read_Byte(void);		                // Read a byte
    uint8_t DHT22_Read_Bit(void);		                // Read a bit
    uint8_t DHT22_Check(void);			                // Check if there is DHT22
    void    DHT22_Reset(void);			                // Reset DHT22  
    int16_t DHT22_getTemperatureX100(uint8_t* data);
    uint16_t DHT22_getHumidityX10(uint8_t* data);
#endif



/* [] END OF FILE */
