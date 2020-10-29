/*
    Edson Kazumi Yamamoto 081368
    Lauren Maria Ferreira 150017
    Eloa Camilo 150700
 */

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

/* Demo includes. */
#include "supporting_functions.h"
#include <semphr.h>

#define LONG_TIME 0xffff
#define TICKS_TO_WAIT    10

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

/* The task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
void vATask( void *pvParameters);

void vLiftAnalyzer(void* pvParameters);

const char* pcTextForTask1 = "Task 1 is running \r\n";
const char* pcTextForTask2 = "Task 2 is running \r\n";

typedef struct
{
	boolean PortaFechada;
	char Nome[6];  
	int	 Andar;
} Elevador; 

QueueHandle_t queueAnalyzer;//Objeto da queue
/*-----------------------------------------------------------*/

int main( void )
{
	Elevador* elevador1 = malloc(sizeof * elevador1);
	//char nome_cliente[] = "Fulano";

	elevador1->Nome[0] = 'f';
	elevador1->Nome[1] = 'u';
	elevador1->Nome[2] = 'l';
	elevador1->Nome[3] = 'a';
	elevador1->Nome[4] = 'n';
	elevador1->Nome[5] = 'o';
	elevador1->Andar = 5;
	elevador1->PortaFechada = false;

	/* Create one of the two tasks. */
	queueAnalyzer = xQueueCreate(10, sizeof(uint32_t));//Cria a queue *buffer* com 10 slots de 4 Bytes

	xTaskCreate(	vTask1,		/* Pointer to the function that implements the task. */
					"Task 1",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */ 
					1,			/* This task will run at priority 1. */
					NULL );		/* We are not using the task handle. */

	xTaskCreate(	vTask2,		/* Pointer to the function that implements the task. */
					"Task 2",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					(void*)elevador1,		/* We are not using the task parameter. */
					1,			/* This task will run at priority 1. */
					NULL);		/* We are not using the task handle. */

	xTaskCreate(	vATask,		/* Pointer to the function that implements the task. */
					"Task A",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					(void*)elevador1,		/* We are not using the task parameter. */
					1,			/* This task will run at priority 1. */
					NULL);		/* We are not using the task handle. */

	xTaskCreate(	vLiftAnalyzer,		/* Pointer to the function that implements the task. */
					"Task analyzer",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					(void*)elevador1,		/* We are not using the task parameter. */
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

	Elevador* elevador = (Elevador *)pvParameters;
	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		vPrintString(elevador->Nome);
		/* Print out the name of this task. */
		vPrintString( pcTaskName );
		vTaskDelay(1000, portTICK_PERIOD_MS);
	}
}

void vATask(void* pvParameters)
{
	const char* pcTaskName = "Task A is running\r\n";
	volatile uint32_t ul;
	Elevador* elevador = (Elevador*)pvParameters;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; )
	{

		xQueueSend(queueAnalyzer, &elevador, portMAX_DELAY);

		vTaskDelay(1500, portTICK_PERIOD_MS);
	}
}

void vLiftAnalyzer(void* pvParameters)
{
	Elevador* elevador;
	while (1)
	{
		if (xQueueReceive(queueAnalyzer, &elevador, portMAX_DELAY) == pdPASS) {

			vPrintString("\n--------------------------\n");
			vPrintString("Nome: ");
			vPrintString(elevador->Nome);
			vPrintString("\nAndar: ");
			char snum[5];
			itoa(elevador->Andar, snum, 10);
			vPrintString(snum);
			vPrintString("\n--------------------------\n");
		}
	}
}