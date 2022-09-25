/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"



/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
pinState_t B_Read;
TaskHandle_t Button_1_Monitor_Handler  = NULL;
TaskHandle_t Button_2_Monitor_Handler  = NULL;
TaskHandle_t Periodic_Transmitter_Handler  = NULL;
TaskHandle_t Uart_Receiver_Handler  = NULL;
TaskHandle_t Load_1_Simulation_Handler  = NULL;
TaskHandle_t Load_2_Simulation_Handler  = NULL;
static QueueHandle_t xQueue1;

int Load_1_in_time =0 , load_1_out_time =0,load_1_total_time =0 ;
int Load_2_in_time =0 , load_2_out_time =0,load_2_total_time =0 ;
int UART_in_time =0 , UART_out_time =0,UART_total_time =0 ;
int B_1_in_time =0 , B_1_out_time =0, B_1_total_time =0 ;
int B_2_in_time =0 , B_2_out_time =0,B_2_total_time =0 ;
int PT_in_time =0 , PT_out_time =0,PT_total_time =0 ;
int systimeTime =0;
float cpu_load =0;

void Uart_Receiver ( void * pvParameters )
{
			int i;
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)1 );	
		//	xSerialPortInitMinimal(ser115200);
	for (;;){
			GPIO_write(PORT_0,PIN0,PIN_IS_HIGH);

			for(i=0;i<5;i++){
			  char xReadQueue[10] ;
        xQueueReceive( xQueue1,
                           xReadQueue  ,
                         ( TickType_t ) 0 );
				
			vSerialPutString( xReadQueue ,10);
			//xSerialPutChar('\n');
			}
		GPIO_write(PORT_0,PIN0,PIN_IS_LOW);
		vTaskDelayUntil( &xLastWakeTime, 20 );
		
		}
}
void Button_1_Monitor ( void * pvParameters )
{
	TickType_t xLastWakeTime;
	pinState_t  newState=0;
	pinState_t oldState= 0;
	char	edge[10];
	oldState = GPIO_read(PORT_1,PIN0);
	vTaskSetApplicationTaskTag(NULL ,(void *)2 );	
	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	
	for (;;){
		GPIO_write(PORT_0,PIN1,PIN_IS_HIGH);
		newState =GPIO_read(PORT_1,PIN0);
		if(newState == 1 && oldState==0){
			strcpy(edge, "Rising....");
		}
		else if(newState == 0 && oldState==1){
			strcpy(edge, "Falling...");
		}
		else{
				strcpy(edge, "no event..");
		}
				
		xQueueSend( xQueue1,
							( void * ) &edge,
							( TickType_t ) 0    );
					
		oldState =newState;
		GPIO_write(PORT_0,PIN1,PIN_IS_LOW);
		vTaskDelayUntil( &xLastWakeTime, 50 );

		}
}
void Button_2_Monitor ( void * pvParameters )
{
	TickType_t xLastWakeTime;
	pinState_t  newState=0;
	pinState_t oldState= 0;
	char	edge[10];
	oldState = GPIO_read(PORT_1,PIN1);
	vTaskSetApplicationTaskTag(NULL ,(void *)3 );	
	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	
	for (;;){
		GPIO_write(PORT_0,PIN2,PIN_IS_HIGH);
		newState =GPIO_read(PORT_1,PIN1);
		if(newState == 1 && oldState==0){

			strncpy(edge, "Rising....", 10);
		}
		else if(newState == 0 && oldState==1){
			strcpy(edge, "Falling...");
		}
		else{
				strncpy(edge, "no event..",10);
		}
				
		xQueueSend( xQueue1,
							( void * ) &edge,
							( TickType_t ) 0    );
					
		oldState =newState;
		GPIO_write(PORT_0,PIN2,PIN_IS_LOW);
		vTaskDelayUntil( &xLastWakeTime, 50 );

		}
}
void Periodic_Transmitter ( void * pvParameters )
{
		char pString[10];
		TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)4 );
	for (;;){
		GPIO_write(PORT_0,PIN3,PIN_IS_HIGH);
		strcpy(pString, "perstring");
				xQueueSend( xQueue1,
							( void * ) &pString,
							( TickType_t ) 0    );
		GPIO_write(PORT_0,PIN3,PIN_IS_LOW);
		vTaskDelayUntil( &xLastWakeTime, 100 );
		
		}
}

void Load_1_Simulation ( void * pvParameters )
{
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)5 );
	for (;;){
			int i;
			GPIO_write(PORT_0,PIN4,PIN_IS_HIGH);
			for(i=0;i<33500 ;i++){
				i=i;
			}
			GPIO_write(PORT_0,PIN4,PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 10 );
			
		}
}
void Load_2_Simulation ( void * pvParameters )
{
			TickType_t xLastWakeTime;
     // Initialise the xLastWakeTime variable with the current time.
     xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL ,(void *)6 );
		for (;;){
			
			int i;
			GPIO_write(PORT_0,PIN5,PIN_IS_HIGH);
			for(i=0;i<80500;i++){

				i=i;
			}
			GPIO_write(PORT_0,PIN5,PIN_IS_LOW);
			vTaskDelayUntil( &xLastWakeTime, 100 );
		}
}
/*
void vApplicationIdleHook( void ){
	GPIO_write(PORT_0,PIN6,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN6,PIN_IS_LOW);
}*/
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */

int main( void )
  {
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
		xQueue1 = xQueueCreate( 5 , sizeof( char ) );
    /* Create Tasks here */

    /* Create the task, storing the handle. */
   
		xTaskCreatePeriodic(
                    Periodic_Transmitter,       /* Function that implements the task. */
                    "Periodic_Transmitter",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Periodic_Transmitter_Handler,
										100);
		xTaskCreatePeriodic(
										Load_2_Simulation,       /* Function that implements the task. */
										"Load_2_Simulation",          /* Text name for the task. */
										100,      /* Stack size in words, not bytes. */
										( void * ) 0,    /* Parameter passed into the task. */
										1,/* Priority at which the task is created. */
										&Load_2_Simulation_Handler,
										100);

		xTaskCreatePeriodic(
                    Button_1_Monitor,       /* Function that implements the task. */
                    "Button_1_Monitor",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button_1_Monitor_Handler,
										50);      /* Used to pass out the created task's handle. */

		xTaskCreatePeriodic(
                    Button_2_Monitor,       /* Function that implements the task. */
                    "Button_2_Monitor",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button_2_Monitor_Handler,
										50); 
		xTaskCreatePeriodic(
                    Uart_Receiver,       /* Function that implements the task. */
                    "Uart_Receiver",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Uart_Receiver_Handler,
										20);	

		
		xTaskCreatePeriodic(
										Load_1_Simulation,       /* Function that implements the task. */
										"Load_1_Simulation",          /* Text name for the task. */
										100,      /* Stack size in words, not bytes. */
										( void * ) 0,    /* Parameter passed into the task. */
										1,/* Priority at which the task is created. */
										&Load_1_Simulation_Handler,
											10);
			

	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo applicati	on projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
 void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


