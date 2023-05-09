/***************************************************************************//**
  @file     main.c
  @brief    FW main
  @author   Nicolás Magliola
 ******************************************************************************/

#include "hardware.h"

void App_Init (void);
void App_Run (void);


int main (void)
{
	hw_Init(); //hardware init -> inicializa TODO (CLKs, etc);
    hw_DisableInterrupts(); //deshabilita las interrupciones para poder configurar todo lo que se necesita en app_init();
    App_Init(); /* Program-specific setup */
    hw_EnableInterrupts(); //y las vuelve a inicializar.

    __FOREVER__
        App_Run(); /* Program-specific loop  */
}
