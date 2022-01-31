#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions
#include <time.h>   // for number generator
#include "ece198.h"

// this bool function checks if the winning condition has been met
bool checkWin(int cRed, int cBlue, int pRed, int pBlue){
    if(cRed == pRed && cBlue == pBlue){
        return true;
    } else {
        return false;
    }
}

int main(void)
{
    HAL_Init(); // initialize the Hardware Abstraction Layer

    // Peripherals (including GPIOs) are disabled by default to save power, so we
    // use the Reset and Clock Control registers to enable the GPIO peripherals that we're using.

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)

    // initialize the pins to be input, output, alternate function, etc...

    InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // on-board LED

    // note: the on-board pushbutton is fine with the default values (no internal pull-up resistor
    // is required, since there's one on the board)

    // set up for serial communication to the host computer
    // (anything we write to the serial port will appear in the terminal (i.e. serial monitor) in VSCode)

    SerialSetup(9600);

    // necessary values for pulse width modulation
    uint32_t period = 100, prescale = 0;
    
    // enabling Timer 3
    __TIM3_CLK_ENABLE();
    
    // initializing an instance of the timer
    TIM_HandleTypeDef pwmTimerInstance3;
    InitializePWMTimer(&pwmTimerInstance3, TIM3, period, prescale);
    
    // initializing the 4 channels of the timer, each associated with their respective red/blue pin
    InitializePWMChannel(&pwmTimerInstance3, TIM_CHANNEL_1);       //red1
    InitializePWMChannel(&pwmTimerInstance3, TIM_CHANNEL_2);       //blue1
    InitializePWMChannel(&pwmTimerInstance3, TIM_CHANNEL_3);       //red2
    InitializePWMChannel(&pwmTimerInstance3, TIM_CHANNEL_4);       //blue2

    // initializing the pins that will go to their respective red/blue pin
    InitializePin(GPIOA, GPIO_PIN_6, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM3); // compRed
    InitializePin(GPIOA, GPIO_PIN_7, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM3); // compBlue
    InitializePin(GPIOB, GPIO_PIN_0, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM3); // playerRED
    InitializePin(GPIOB, GPIO_PIN_1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF2_TIM3); // playerBLUE

    InitializePin(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // red plus button
    InitializePin(GPIOB, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // red minus button
    InitializePin(GPIOA, GPIO_PIN_9, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // blue plus button
    InitializePin(GPIOB, GPIO_PIN_6, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // blue minus button

    InitializePin(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0); // green LED

    srand(time(NULL)); // ensures the generation of different red/blue values every round

    // this loop repeats itself for every round of the game
    while (true)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, false); // turns green LED off
        
        // default values for player and computer colour variables
        int computerRed = (int) ((rand()%100)/10)*10;
        int computerBlue = (int) ((rand()%100)/10)*10;
        int playerRed = 0;
        int playerBlue = 0;

        SetPWMDutyCycle(&pwmTimerInstance3, TIM_CHANNEL_1, computerRed); // computer red
        SetPWMDutyCycle(&pwmTimerInstance3, TIM_CHANNEL_2, computerBlue); // computer blue 

        /* this loop checks for button presses, updates the player's LED after each press,
        turns on green LED to signify a win if met
        */
        while(true){
            if(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) && playerRed < 100){ // button press pRed+
                playerRed += 10;
                HAL_Delay(300);
            }

            if(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) && playerRed > 0){ // button press pRed-
                playerRed -= 10;
                HAL_Delay(300);
            }

            if(!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) && playerBlue < 100){ // button press pBlue+
                playerBlue += 10;
                HAL_Delay(300);
            }

            if(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) && playerBlue > 0){ // button press pBlue-
                playerBlue -= 10;
                HAL_Delay(300);
            }
            
            SetPWMDutyCycle(&pwmTimerInstance3, TIM_CHANNEL_3, playerRed); // player red
            SetPWMDutyCycle(&pwmTimerInstance3, TIM_CHANNEL_4, playerBlue); // player blue

            // checks if the game has been won
            if(checkWin(computerRed, computerBlue, playerRed, playerBlue)){
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, true); // turn on the green LED
              HAL_Delay(3000); // leave the green LED on for 3 seconds before starting the next round
              break;
            }
        }
    }

    return 0;

}


// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}