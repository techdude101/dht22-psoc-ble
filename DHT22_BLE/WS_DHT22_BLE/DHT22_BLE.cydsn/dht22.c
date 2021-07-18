/* ========================================
 * Filename:        dht22.c
 * Description:     DHT22 Driver for PSoC source file
 * Author:          techdude101
 * Version:         0.1.0
 * ========================================
*/

#include "dht22.h"
#include <project.h>

/*******************************************************************************
* Function Name: DHT22_Reset
********************************************************************************
*
* Summary:
*  This routine resets a DHT22 device.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void DHT22_Reset(void)	   
{      	
    static uint8_t IState;
    IState = CyEnterCriticalSection();  
    DHT22_DQ_Write(0); 	// Pull down DQ
    CyDelay(20);    	// Pull down for at least 18ms
    DHT22_DQ_Write(1); 	// DQ = 1 
	CyDelayUs(30);     	// The host pulls 20~40us
    
    CyExitCriticalSection(IState); 
}

/*******************************************************************************
* Function Name: DHT22_Check
********************************************************************************
*
* Summary:
*  This routine resets a DHT22 device.
*
* Parameters:
*  None
*
* Return:
*  uint8_t error: 1 = error, 0 = no error
*
*******************************************************************************/
uint8_t DHT22_Check(void) 	   
{   
    static uint8_t IState;
    IState = CyEnterCriticalSection();  
    
	uint8_t retry = 100;
   	while ((!DHT22_DQ_Read()) && retry) // DHT22 will pull down 40~80us
	{
		retry--;
		CyDelayUs(1);
	};	 
	if (retry < 1) return 1;
	else retry = 100;
    while (DHT22_DQ_Read() && retry) // DHT22 will pull up 40~80us again after pulling low
	{
		retry--;
		CyDelayUs(1);
	};
    
    CyExitCriticalSection(IState); 
	
    if (retry < 1)
		return 1;	    
	return 0;
}

/*******************************************************************************
* Function Name: DHT22_Read_Bit
********************************************************************************
*
* Summary:
*  This routine reads one bit from a DHT22 device.
*
* Parameters:
*  None
*
* Return:
*  uint8_t bit: Bit status
*
*******************************************************************************/
uint8_t DHT22_Read_Bit(void) 			 
{
 	uint8_t dat;
 	uint8_t retry = 100;
    uint8_t IState = CyEnterCriticalSection();  
	
    while((!DHT22_DQ_Read()) && retry) // Wait to go high
	{
		retry--;
		CyDelayUs(1);
	}
	retry = 100;
	dat = 0;
	CyDelayUs(30); //wait for 30us
	
    if(DHT22_DQ_Read())	
		dat = 1;	   
	
    while(DHT22_DQ_Read() && retry) // Waiting to go low
	{
		retry--;
		CyDelayUs(1);
	}
    CyExitCriticalSection(IState);
	return dat;		
}

/*******************************************************************************
* Function Name: DHT22_Read_Byte
********************************************************************************
*
* Summary:
*  This routine reads one byte from a DHT22 device.
*
* Parameters:
*  None
*
* Return:
*  uint8_t read byte: Byte read from DHT22 device
*
*******************************************************************************/
uint8_t DHT22_Read_Byte(void)    
{        
    uint8_t dat;
    dat = 0;
	for (uint8_t i = 0; i < 8; i++) 
	{
   		dat <<= (1u); 
	    dat |= DHT22_Read_Bit();
    }						    
    return dat;
}

/*******************************************************************************
* Function Name: DHT22_Read_Data
********************************************************************************
*
* Summary:
*  This routine reads the sensor data from a DHT22 device.
*  Humidity[0-1], temperature[2-3] and checksum[4]
*
* Parameters:
*  uint8_t* data: Pointer to an array[5] to store the data read from the DHT22 device
*
* Return:
*  uint8_t error: 1 = error, 0 = no error
*
*******************************************************************************/
uint8_t DHT22_Read_Data(uint8_t *data)    
{        
 	volatile uint8_t buf[5];
	
    DHT22_Reset();
	
    if(DHT22_Check() == 0)
	{
		for(uint8_t i = 0; i < 5; i++) // Read 40-bit data
		{
			buf[i] = DHT22_Read_Byte();
		}
        
		// Checksum accumulation has a considerable probability that will exceed 8 bits (that is, the value is greater than 1 char, so do cast, only consider the lower 8)
		if((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])  
		{
			for(uint8_t i = 0; i < 4; i++)
			{
				*data = buf[i];
				data++;
			}
			return 0;	
		}
	}
	return 1;   
}

/*******************************************************************************
* Function Name: DHT22_Init
********************************************************************************
*
* Summary:
*  This routine initializes and checks for the presence of a DHT22 device.
*
* Parameters:
*  None
*
* Return:
*  uint8_t error: 1 = error, 0 = no error
*
*******************************************************************************/
uint8_t DHT22_Init(void)
{	
	DHT22_Reset();
	return DHT22_Check();
}

/*******************************************************************************
* Function Name: DHT22_getTemperatureX100
********************************************************************************
*
* Summary:
*  This routine extracts the temperature data from a data packet read from a DHT22 sensor
*
* Parameters:
*  uint8_t* data: DHT22 data array
*
* Return:
*  int16_t temperature: Temperature x 100 (to avoid using floats)
*
*******************************************************************************/
int16_t DHT22_getTemperatureX100(uint8_t* data) {
    
    return ((data[3] * 10) + data[2]);
}

/*******************************************************************************
* Function Name: DHT22_getHumidityX10
********************************************************************************
*
* Summary:
*  This routine extracts the humidity data from a data packet read from a DHT22 sensor
*
* Parameters:
*  uint8_t* data: DHT22 data array
*
* Return:
*  int16_t humidity: Humidity x 10 (to avoid using floats)
*
*******************************************************************************/
uint16_t DHT22_getHumidityX10(uint8_t* data) {
    return ((data[1] * 10) + data[0]);
}

/* [] END OF FILE */
