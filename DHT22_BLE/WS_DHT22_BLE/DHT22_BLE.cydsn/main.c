/* ========================================
 * Filename:        main.c
 * Description:     Main program PSoC source file
 * Author:          techdude101
 * Version:         0.1.0
 * ========================================
*/

#include <project.h>
#include "dht22.h"

/***************************************
*        API Constants
***************************************/
#define LED_ON                                      (0u)
#define LED_OFF                                     (1u)

#define LOOP_DELAY                                  (1u)  /* How often would you like to update the ADV payload */

/* ADV payload dta structure */   
#define advPayload                                  (cyBle_discoveryModeInfo.advData->advData) /* DHT22 xx.xC xx%*/
#define TEMPERATURE_INDEX                           (11u) /* 11 - 14 */
#define HUMIDITY_INDEX                              (17u) /* 17 - 18 */

/***************************************
*        Function Prototypes
***************************************/
void InitializeSystem(void);
void StackEventHandler(uint32 event, void* eventParam);
void EnterLowPowerMode(void);
void DynamicADVPayloadUpdate(int16_t temperature, uint16_t humidity);

int main (void)
{
    InitializeSystem();
    
    /* Flash LED on startup */
    LED_R_Write(LED_ON);
    CyDelay(500);
    LED_R_Write(LED_OFF);
    
    for(;;)
    {
        /* Single API call to service all the BLE stack events. Must be
         * called at least once in a BLE connection interval */
        CyBle_ProcessEvents();
        
        uint8_t dht22_data[5] = { 0 };
        
        // Read the sensor and store in an array
        DHT22_Read_Data(dht22_data);
        
        // Extract the sensor data from the array
        int16_t temperatureX100 = DHT22_getTemperatureX100(dht22_data);
        uint16_t humidityX10 = DHT22_getHumidityX10(dht22_data);
        
        DynamicADVPayloadUpdate(temperatureX100, humidityX10);
        
        LED_R_Write(LED_OFF);
        
        /* Configure the system in lowest possible power modes during and between BLE ADV events */
        EnterLowPowerMode();
        
        LED_R_Write(LED_ON);
    }
}


/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
*
* Summary:
*  This routine initializes all the componnets and firmware state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void InitializeSystem(void)
{
    CYBLE_API_RESULT_T apiResult;

    CyGlobalIntEnable;
    
    //DHT22_Init();

    apiResult = CyBle_Start(StackEventHandler); /* Init the BLE stack and register an applicaiton callback */

    if(apiResult != CYBLE_ERROR_OK)
    {
        /* BLE stack initialization failed, check your configuration */
        CYASSERT(0);
    }
    
    /* Set XTAL divider to 3MHz mode */
    CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8); 
    
    /* ILO is no longer required, shut it down */
    CySysClkIloStop();
}

/*******************************************************************************
* Function Name: EnterLowPowerMode
********************************************************************************
*
* Summary:
*  This configures the BLESS and system in low power mode whenever possible.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void EnterLowPowerMode(void)
{
    CYBLE_BLESS_STATE_T blessState;
    uint8 intrStatus;
    
    /* Configure BLESS in Deep-Sleep mode */
    CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
    
    /* Prevent interrupts while entering system low power modes */
    intrStatus = CyEnterCriticalSection();
    
    /* Get the current state of BLESS block */
    blessState = CyBle_GetBleSsState();
    
    /* If BLESS is in Deep-Sleep mode or the XTAL oscillator is turning on, 
     * then PSoC 4 BLE can enter Deep-Sleep mode (1.3uA current consumption) */
    if(blessState == CYBLE_BLESS_STATE_ECO_ON || 
        blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
    {
        CySysPmDeepSleep();
    }
    else
    {
        /* Keep trying to enter either Sleep or Deep-Sleep mode */    
    }
    
    CyExitCriticalSection(intrStatus);
}


/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  uint8 event:       Event from the CYBLE component
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of event structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam)
{
    switch(event)
    {
        /* Mandatory events to be handled by Find Me Target design */
        case CYBLE_EVT_STACK_ON:
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_SLOW);
          break;

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
            {

            }
            break;

        default:
        break;
    }
}

/*******************************************************************************
* Function Name: DynamicADVPayloadUpdate
********************************************************************************
*
* Summary:
*  This routine dynamically updates the BLE advertisement packet
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void DynamicADVPayloadUpdate(int16_t temperature, uint16_t humidity)
{
    if(CyBle_GetBleSsState() == CYBLE_BLESS_STATE_EVENT_CLOSE)
    {   
        /* Once the system enters Sleep/Deepsleep mode during advertisement, the source of wake-up is the next  
         * advertisement interval which has a wakeup interval of 1 advertisement (ADV) interval (100ms). 
         * LOOP_DELAY * ADV interval is the interval after which ADV data is updated in this firmware.*/
        
        advPayload[TEMPERATURE_INDEX] = ('0' + (uint8_t)(temperature / 100));
        advPayload[TEMPERATURE_INDEX + 1] = ('0' + (uint8_t)((temperature / 10) % 10));
        advPayload[TEMPERATURE_INDEX + 3] = ('0' + (uint8_t)(temperature % 10));
        
        advPayload[HUMIDITY_INDEX] = ('0' + (uint8_t)((humidity / 100) % 10));
        advPayload[HUMIDITY_INDEX + 1] = ('0' + (uint8_t)((humidity / 10) % 10));
        
        CyBle_GapUpdateAdvData(cyBle_discoveryModeInfo.advData, cyBle_discoveryModeInfo.scanRspData);
    }
}

/* [] END OF FILE */
