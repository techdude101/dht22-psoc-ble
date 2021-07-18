/*******************************************************************************
* File Name: DHT22_DQ.c  
* Version 2.20
*
* Description:
*  This file contains APIs to set up the Pins component for low power modes.
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "DHT22_DQ.h"

static DHT22_DQ_BACKUP_STRUCT  DHT22_DQ_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: DHT22_DQ_Sleep
****************************************************************************//**
*
* \brief Stores the pin configuration and prepares the pin for entering chip 
*  deep-sleep/hibernate modes. This function applies only to SIO and USBIO pins.
*  It should not be called for GPIO or GPIO_OVT pins.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None 
*  
* \sideeffect
*  For SIO pins, this function configures the pin input threshold to CMOS and
*  drive level to Vddio. This is needed for SIO pins when in device 
*  deep-sleep/hibernate modes.
*
* \funcusage
*  \snippet DHT22_DQ_SUT.c usage_DHT22_DQ_Sleep_Wakeup
*******************************************************************************/
void DHT22_DQ_Sleep(void)
{
    #if defined(DHT22_DQ__PC)
        DHT22_DQ_backup.pcState = DHT22_DQ_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            DHT22_DQ_backup.usbState = DHT22_DQ_CR1_REG;
            DHT22_DQ_USB_POWER_REG |= DHT22_DQ_USBIO_ENTER_SLEEP;
            DHT22_DQ_CR1_REG &= DHT22_DQ_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(DHT22_DQ__SIO)
        DHT22_DQ_backup.sioState = DHT22_DQ_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        DHT22_DQ_SIO_REG &= (uint32)(~DHT22_DQ_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: DHT22_DQ_Wakeup
****************************************************************************//**
*
* \brief Restores the pin configuration that was saved during Pin_Sleep(). This 
* function applies only to SIO and USBIO pins. It should not be called for
* GPIO or GPIO_OVT pins.
*
* For USBIO pins, the wakeup is only triggered for falling edge interrupts.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None
*  
* \funcusage
*  Refer to DHT22_DQ_Sleep() for an example usage.
*******************************************************************************/
void DHT22_DQ_Wakeup(void)
{
    #if defined(DHT22_DQ__PC)
        DHT22_DQ_PC = DHT22_DQ_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            DHT22_DQ_USB_POWER_REG &= DHT22_DQ_USBIO_EXIT_SLEEP_PH1;
            DHT22_DQ_CR1_REG = DHT22_DQ_backup.usbState;
            DHT22_DQ_USB_POWER_REG &= DHT22_DQ_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(DHT22_DQ__SIO)
        DHT22_DQ_SIO_REG = DHT22_DQ_backup.sioState;
    #endif
}


/* [] END OF FILE */
