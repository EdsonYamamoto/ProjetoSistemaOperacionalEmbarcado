/*
    Edson Kazumi Yamamoto 081368
    Lauren Maria Ferreira 150017
    Eloa Camilo 150700
 */

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo includes. */
#include "supporting_functions.h"
#include <semphr.h>

#define LONG_TIME 0xffff
#define TICKS_TO_WAIT    10

SemaphoreHandle_t xSemaphore = NULL;


/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

/* The task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

void vTaskFunction(void* pvParameters);

const char* pcTextForTask1 = "Task 1 is running \r\n";
const char* pcTextForTask2 = "Task 2 is running \r\n";

/*-----------------------------------------------------------*/

int main( void )
{
	/* Create one of the two tasks. */

	xTaskCreate(	vTask1,		/* Pointer to the function that implements the task. */
					"Task 1",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */ 
					1,			/* This task will run at priority 1. */
					NULL );		/* We are not using the task handle. */

	xTaskCreate(	vTask2,		/* Pointer to the function that implements the task. */
					"Task 2",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					1,			/* This task will run at priority 1. */
					NULL);		/* We are not using the task handle. */

	/* Start the scheduler to start the tasks executing. */
	vTaskStartScheduler();	

	/* The following line should never be reached because vTaskStartScheduler() 
	will only return if there was not enough FreeRTOS heap memory available to
	create the Idle and (if configured) Timer tasks.  Heap management, and
	techniques for trapping heap exhaustion, are described in the book text. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
const char *pcTaskName = "Task 1 is running\r\n";
volatile uint32_t ul;

	for( ;; )
	{
        vPrintString(pcTaskName);
        vTaskDelay(500, portTICK_PERIOD_MS);
	}
}
/*-----------------------------------------------------------*/

void vTask2( void *pvParameters )
{
const char *pcTaskName = "Task 2 is running\r\n";
volatile uint32_t ul;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task. */
		vPrintString( pcTaskName );

		vTaskDelay(1000, portTICK_PERIOD_MS);
	}
}




/* Repetitive task. */
void vATask(void* pvParameters)
{
    /* We are using the semaphore for synchronisation so we create a binary
    semaphore rather than a mutex.  We must make sure that the interrupt
    does not attempt to use the semaphore before it is created! */
    xSemaphore = xSemaphoreCreateBinary();

    for (;; )
    {
        /* We want this task to run every 10 ticks of a timer.  The semaphore
        was created before this task was started.

        Block waiting for the semaphore to become available. */
        if (xSemaphoreTake(xSemaphore, LONG_TIME) == pdTRUE)
        {
            /* It is time to execute. */

                /* We have finished our task.  Return to the top of the loop where
                we will block on the semaphore until it is time to execute
                again.  Note when using the semaphore for synchronisation with an
                ISR in this manner there is no need to 'give' the semaphore
                back. */
        }
    }
}

/* Timer ISR */
void vTimerISR(void* pvParameters)
{
    static char ucLocalTickCount = 0;
    static BaseType_t xHigherPriorityTaskWoken;

        /* Is it time for vATask() to run? */
    xHigherPriorityTaskWoken = pdFALSE;
    ucLocalTickCount++;
    if (ucLocalTickCount >= TICKS_TO_WAIT)
    {
        /* Unblock the task by releasing the semaphore. */
        xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

        /* Reset the count so we release the semaphore again in 10 ticks
        time. */
        ucLocalTickCount = 0;
    }

    /* If xHigherPriorityTaskWoken was set to true you
    we should yield.  The actual macro used here is
    port specific. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}