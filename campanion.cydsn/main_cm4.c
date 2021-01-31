/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description: This file main application code for the CE223727 EmWin Graphics
*				library EInk Display.
*
* Hardware Dependency: CY8CKIT-028-EPD E-Ink Display Shield
*					   CY8CKIT-062-BLE PSoC6 BLE Pioneer Kit
*
******************************************************************************* 
* Copyright (2019), Cypress Semiconductor Corporation. All rights reserved. 
******************************************************************************* 
* This software, including source code, documentation and related materials 
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its 
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent 
* protection (United States and foreign), United States copyright laws and 
* international treaty provisions. Therefore, you may use this Software only 
* as provided in the license agreement accompanying the software package from 
* which you obtained this Software (“EULA”). 
* 
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify, and compile the Software source 
* code solely for use in connection with Cypress’s integrated circuit products. 
* Any reproduction, modification, translation, compilation, or representation 
* of this Software except as specified above is prohibited without the express 
* written permission of Cypress. 
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
********************************************************************************/

#include "project.h"
#include "GUI.h"
#include "pervasive_eink_hardware_driver.h"
#include "cy_eink_library.h"
#include "LCDConf.h"
#include "Thermistor.h"
#include <stdio.h>
#include <stdlib.h>


#define Buzzer P12_5_PORT 
#define Buzzer1 P12_5_NUM

/* Image buffer cache */
uint8 imageBufferCache[CY_EINK_FRAME_SIZE] = {0};

/* Reference to the bitmap image for the startup screen */
extern GUI_CONST_STORAGE GUI_BITMAP bmCypressLogoFullColor_PNG_1bpp;


/* Function prototypes */
void TemperatureDisplay(void);
void GasSensor(void);
void EggTimer(void);
void CampingActivity(void);
/* Array of demo pages functions */
void (*demoPageArray[])(void) = {
    TemperatureDisplay,
    CampingActivity,
    EggTimer,
    GasSensor,
};

/* Number of demo pages */
#define NUMBER_OF_DEMO_PAGES    (sizeof(demoPageArray)/sizeof(demoPageArray[0]))

/*******************************************************************************
* Function Name: void UpdateDisplay(void)
********************************************************************************
*
* Summary: This function updates the display with the data in the display 
*			buffer.  The function first transfers the content of the EmWin
*			display buffer to the primary EInk display buffer.  Then it calls
*			the Cy_EINK_ShowFrame function to update the display, and then
*			it copies the EmWin display buffer to the Eink display cache buffer
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  It takes about a second to refresh the display.  This is a blocking function
*  and only returns after the display refresh
*
*******************************************************************************/
void UpdateDisplay(cy_eink_update_t updateMethod, bool powerCycle)
{
    cy_eink_frame_t* pEmwinBuffer;

    /* Get the pointer to Emwin's display buffer */
    pEmwinBuffer = (cy_eink_frame_t*)LCD_GetDisplayBuffer();

    /* Update the EInk display */
    Cy_EINK_ShowFrame(imageBufferCache, pEmwinBuffer, updateMethod, powerCycle);

    /* Copy the EmWin display buffer to the imageBuffer cache*/
    memcpy(imageBufferCache, pEmwinBuffer, CY_EINK_FRAME_SIZE);
}

/*******************************************************************************
* Function Name: void ShowStartupScreen(void)
********************************************************************************
*
* Summary: This function displays the startup screen with Cypress Logo and 
*			the demo description text
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowStartupScreen(void)
{
    /* Set foreground and background color and font size */
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();

    GUI_DrawBitmap(&bmCypressLogoFullColor_PNG_1bpp, 2, 2);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Campanion", 132, 105);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Your Camping Buddy", 132, 125);

  
    /* Send the display buffer data to display*/
    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
}


/*******************************************************************************
* Function Name: void ShowInstructionsScreen(void)
********************************************************************************
*
* Summary: This function shows screen with instructions to press SW2 to
*			scroll through various display pages
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowInstructionsScreen(void)
{
    /* Set font size, background color and text mode */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetColor(GUI_BLACK);	
    GUI_SetTextMode(GUI_TM_NORMAL);

    /* Clear the display */
    GUI_Clear();

    /* Display instructions text */
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("PRESS THE PUSH BUTTON", 132, 58);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("TO SCROLL THROUGH ", 132, 78);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("THE PAGES!", 132, 98);

    /* Send the display buffer data to display*/
    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
}


/*******************************************************************************
* Function Name: void ClearScreen(void)
********************************************************************************
*
* Summary: This function clears the screen
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ClearScreen(void)
{
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
}


/*******************************************************************************
* Function Name: void WaitforSwitchPressAndRelease(void)
********************************************************************************
*
* Summary: This implements a simple "Wait for button press and release"
*			function.  It first waits for the button to be pressed and then
*			waits for the button to be released.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This is a blocking function and exits only on a button press and release
*
*******************************************************************************/
void WaitforSwitchPressAndRelease(void)
{
    /* Wait for SW2 to be pressed */
    while(Status_SW2_Read() != 0);
    
    /* Wait for SW2 to be released */
    while(Status_SW2_Read() == 0);
}

/*******************************************************************************
* Function Name: void TemperatureDisplay(void)
********************************************************************************
*
* Summary: This function shows screen current temperature from the onboard
*    	   thermistor on the e-ink display
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/


void TemperatureDisplay(void)
{

    UART_1_Start(); 
    ADC_1_Start(); //start the Analog to digital convertor
    Cy_SAR_StartConvert(SAR,CY_SAR_START_CONVERT_CONTINUOUS);
    
    float v1,v2;
    int16_t countThermistor, countReference;

    /* Read the ADC count values */
    countReference  = Cy_SAR_GetResult16(SAR,0);
    countThermistor = Cy_SAR_GetResult16(SAR,1);

    v1 = Cy_SAR_CountsTo_Volts(SAR,0,countReference);
    v2 = Cy_SAR_CountsTo_Volts(SAR,1,countThermistor);
	/* use the formula to calculate the temperature */
    uint32 resT = Thermistor_GetResistance(countReference, countThermistor);
    float temperature = (float)Thermistor_GetTemperature(resT) / 100.0 ;
	
    printf("V1 = %fv V2=%fv Vtot =%fv T=%fC T=%fF\r\n",v1,v2,v1+v2,temperature,9.0/5.0*temperature + 32.0); //output the temperature to the terminal console
     
     char c[50]; //size of the number
     sprintf(c, "%g", temperature); // convert the number to a string
    

    
    /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);
    

    /* Clear the screen */
    GUI_Clear();
    
        /* Display page title */
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Current temperature", 132, 5);
    

    /* Display labels */	
    GUI_SetFont(GUI_FONT_D64);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt(c, 132, 53); //display the current temperature

    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("degrees", 132, 150);
    
    
    
    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
    

    
}  

/*******************************************************************************
* Function Name: void GasSensor(void)
********************************************************************************
*
* Summary: This function makes a buzzer sound if the gas reading is reported as
*          to high
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void GasSensor(void)
{
    UART_1_Start();
    ADC_1_Start();
    Cy_SAR_StartConvert(SAR,CY_SAR_START_CONVERT_CONTINUOUS);
    
    float value;

    /* Read the ADC count values */
    
    

    

    float reading, calc;
    for (;;) {
        /* Read the ADC count values */
        value  = Cy_SAR_GetResult16(SAR,2);
        reading  = Cy_SAR_GetResult16(SAR,2);
        calc = Cy_SAR_CountsTo_Volts(SAR,2,reading);
        char c[50]; //size of the number
        sprintf(c, "%g", calc); // convert the number into a string

        
        /* Set font size, foreground and background colors */
        GUI_SetColor(GUI_BLACK);
        GUI_SetBkColor(GUI_WHITE);
        GUI_SetTextMode(GUI_TM_NORMAL);
        GUI_SetTextStyle(GUI_TS_NORMAL);
        

        /* Clear the screen */
        GUI_Clear();
        
            /* Display page title */
        GUI_SetFont(GUI_FONT_20_1);
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt("Gas reading", 132, 5);
        

        /* Display labels */	
        GUI_SetFont(GUI_FONT_D32);
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt(c, 132, 53); //display the current gas reading

        GUI_SetFont(GUI_FONT_20_1);
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt("", 132, 150);
        
    
        UpdateDisplay(CY_EINK_FULL_4STAGE, true);
        
        
        if (calc > 0.3) { // If the reading is greater than a number, start the buzzer and keep it on for 10 seconds
            PWM_Start(); //Start the buzzer
               CyDelay(10000); //Wait 10 seconds
               PWM_Disable();  //Stop the buzzer
        }
        else {
            PWM_Disable();
        }
       
        CyDelay(2000); //Wait 2 seconds inbetween every recording
         
        
      }  

}

/*******************************************************************************
* Function Name: void EggTimer(void)
********************************************************************************
*
* Summary: This function acts as a timer for how long to cook a boiled egg, when
*          the timer is up, a buzzer will beep
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void EggTimer(void) {
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);
    

    /* Clear the screen */
    GUI_Clear();
    
    /* Display page title */
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Egg Timer", 132, 5);
    
    GUI_DispStringAt("Timer started now", 132, 25);
    
    int i;
    int time_left = 420; //set the time in seconds for an egg to cook
    
    while (time_left >= 0) { //check if the timer has finished
        
        time_left = time_left - 10; //if not take 10 seconds off as you have just waited 10 seconds
        
        if (time_left <= 0) {
            break; //exit the loop if the timer is up
        }
        
        
        GUI_SetColor(GUI_BLACK);
        GUI_SetBkColor(GUI_WHITE);
        GUI_SetTextMode(GUI_TM_NORMAL);
        GUI_SetTextStyle(GUI_TS_NORMAL);
        
        /* Clear the screen */
        GUI_Clear();
        /* Display page title */
        GUI_SetFont(GUI_FONT_20_1);
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt("Egg Timer", 130, 5);
    
        GUI_DispStringAt("Timer started now", 60, 25);
        
        char c[20];
        sprintf(c, "%d", time_left);
        
            
        GUI_SetFont(GUI_FONT_D64);
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt(c, 132, 70);
        
        GUI_SetFont(GUI_FONT_20_1);
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt("Seconds Left", 135, 150);
        
        
        
        UpdateDisplay(CY_EINK_FULL_4STAGE, true); //Update the display
        
        CyDelay(10000); //then wait 10 seconds and then update to check if the timer has run out again
        
        
        
    
    
    }
    
    
    GUI_Clear();
        /* Display 0 seconds left on the timer */
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Egg Timer", 130, 5);

    GUI_DispStringAt("Timer started now", 60, 25);
            
    GUI_SetFont(GUI_FONT_D64);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("0", 132, 70);
    
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Seconds Left", 135, 150);

    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
    
    
    int x;
    for (x = 0; x < 50; x = x + 1) { //beep the buzzer
    PWM_Start();
    CyDelay(100);
    PWM_Disable();
    CyDelay(100);
    }
    
        
}

/*******************************************************************************
* Function Name: void CampingActivity(void)
********************************************************************************
*
* Summary: Give the user a random camping activity to do on the display
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void CampingActivity(void){
     UART_1_Start();
    __enable_irq();
    /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);
    /*Set up the various camping activities for program to pick one of */
    const char *activities[] = {"Card Game", "Eat a boiled egg", "scavenger hunt", "Brave the campsite showers", "Take a walk", "Play i spy"};
    
    int choice = rand() % 7; //Pick a random choice from the list
    
    
    /* Clear the screen */
    GUI_Clear();
    
        /* Display page title */
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Random camping activity", 132, 5);
    
    char c[20];
    sprintf(c, "%s", activities[choice]);

    /* Display labels */	
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt(c, 132, 83); //Output the random camping activity to the display
    GUI_SetFont(GUI_FONT_20_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("Have fun!", 132, 150);
    
    
    
    UpdateDisplay(CY_EINK_FULL_4STAGE, true); //update the display

    
}
/*******************************************************************************
* Function Name: int main(void)
********************************************************************************
*
* Summary: This is the main function.  Following functions are performed
*			1. Initialize the EmWin library
*			2. Initialize the thermistor
*			3. Display the startup screen for 3 seconds
*			4. Display the instruction screen 
*			5. Display the startup screen again and wait for key press and release
*			6. Inside a while loop scroll through the 6 demo pages on every
*				key press and release
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main(void)
{
    
    uint8 pageNumber = 0;
    
    __enable_irq(); /* Enable global interrupts. */
    
    /* Initialize emWin Graphics */
    GUI_Init();

    /* Start the eInk display interface and turn on the display power */
    Cy_EINK_Start(20);
    Cy_EINK_Power(1);
    
    UART_1_Start();
    ADC_1_Start();
    
    
    /*This starts the thermistor and prevents it from gettin a rediculous value such as -158 degrees */
    Cy_SAR_StartConvert(SAR,CY_SAR_START_CONVERT_CONTINUOUS);
    
    float v1,v2;
    int16_t countThermistor, countReference;

    /* Read the ADC count values */
    countReference  = Cy_SAR_GetResult16(SAR,0);
    countThermistor = Cy_SAR_GetResult16(SAR,1);

    v1 = Cy_SAR_CountsTo_Volts(SAR,0,countReference);
    v2 = Cy_SAR_CountsTo_Volts(SAR,1,countThermistor);

    uint32 resT = Thermistor_GetResistance(countReference, countThermistor);
    float temperature = (float)Thermistor_GetTemperature(resT) / 100.0 ;

    /* Show the startup screen */
    ShowStartupScreen();
    CyDelay(2000);

    /* Show the instructions screen */
    ShowInstructionsScreen();
    CyDelay(4000);
    ShowStartupScreen();
    WaitforSwitchPressAndRelease();
    
    for(;;)
    {
        /* Turn on the Green LED before page update */
        Cy_GPIO_Write(GreenLED_PORT, GreenLED_NUM, 0);
        

        (*demoPageArray[pageNumber])();
        
        /* Turn off the Greed LED after page update */
        Cy_GPIO_Write(GreenLED_PORT, GreenLED_NUM, 1);
        
        /* Wait for a switch press event */
        WaitforSwitchPressAndRelease();
        
        /* Cycle through page numbers */
        pageNumber = (pageNumber+1) % NUMBER_OF_DEMO_PAGES;
    }
    
    
    
}

/* [] END OF FILE */
