/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

/* Demo includes. */
#include "supporting_functions.h"
#include <semphr.h>

// Macros for button GPIO lines
#define SW1 (0xE000E000UL)
#define SW2 (0xE000E000UL)
#define SW3 (0xE000E000UL)
#define SW4 (0xE000E000UL)
#define SW5 (0xE000E000UL)
#define SW6 (0xE000E000UL)
#define SW7 (0xE000E000UL)
#define SW8 (0xE000E000UL)
#define SW9 (0xE000E000UL)
#define SW10 (0xE000E000UL)
#define SWOpen (0xE000E000UL)
#define SWClose (0xE000E000UL)

#define SwDelay 1000

#define LONG_TIME 0xffff
#define TICKS_TO_WAIT    10

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

/* The task functions. */
void vATask( void *pvParameters);

void vLiftAnalyzer(void* pvParameters);
void taskButtons(void* pvParameters);

void vLiftController(void* pvParameters);

bool CheckAndDebounceD(uint32_t swNum);
bool CheckAndDebounceC(uint32_t swNum);


bool mPORTDReadBits(uint32_t swNum);
bool mPORTCReadBits(uint32_t swNum);


void vLiftDoorCheck(void* pvParameters);
void vLiftMetor(void* pvParameters);

const char* pcTextForTask1 = "Task 1 is running \r\n";
const char* pcTextForTask2 = "Task 2 is running \r\n";

typedef struct
{
	boolean PortaFechada;
	char Nome[6];  
	int	Andar;
	int ProximosAndares[3];
	int MotorFuncionando;	/* 0 - andar parado
							   1 - subindo
							   -1 - descendo
							*/
} Elevador; 

typedef struct
{
	Elevador *elevador[3];
	int Proximos[10];
} ControleElevador;

Elevador* CreateLift();

QueueHandle_t queueAnalyzer;//Objeto da queue
QueueHandle_t queueLifter;//Objeto da queue
/*-----------------------------------------------------------*/

/* Global Variables */
xSemaphoreHandle xSemaphore;
int qtdLifters = 3; 

int main( void )
{
	vPrintString("Inicializando projeto de sistemas operacionais embarcados freertos");
	ControleElevador* controlador = malloc(sizeof * controlador);
	for(int i =0;i<qtdLifters;i++)
		controlador->elevador[i] = CreateLift();
	
	/* Create the Semaphore for synchronization between UART and LED task */
	vSemaphoreCreateBinary(xSemaphore)
	//xSemaphoreGive(xSemaphore);
	/* Create one of the two tasks. */
	queueAnalyzer = xQueueCreate(10, sizeof(uint32_t));
	queueLifter = xQueueCreate(10, sizeof(uint32_t));

	xTaskCreate(	taskButtons,	
					"Task Buttons",
					1000,	
					NULL,	
					1,	
					NULL);

	xTaskCreate(	vLiftAnalyzer,
					"Task analyzer",
					1000,	
					NULL,	
					1,		
					NULL);	

	xTaskCreate(	vLiftController,	
					"Task controller",	
					1000,	
					(void*)controlador,		
					1,	
					NULL);	
	vTaskStartScheduler();	

	for( ;; );
	return 0;
}

/*-----------------------------------------------------------*/

void vATask(void* pvParameters)
{
	const char* pcTaskName = "Task A is running\r\n";
	volatile uint32_t ul;
	Elevador* elevador = (Elevador*)pvParameters;

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
			vPrintString("\nporta: "); 
			vPrintString(elevador->PortaFechada);
			vPrintString("\'motor: ");
			vPrintString(elevador->MotorFuncionando);

			vPrintString("\n--------------------------\n");
		}
	}
}

/*
	Logica do controlador para manipular os elevadores.
*/
void vLiftController(void* pvParameters)
{
	ControleElevador* controlador = (ControleElevador*)pvParameters;

	char* comando;
	int andar = -1;
	while (1)
	{
		if (xQueueReceive(queueLifter, &comando, portMAX_DELAY) == pdPASS) {
			
			if (comando = "0") {
				andar = atoi(comando);
			}
			else if (comando = "1") {
				andar = atoi(comando);
			}
			else if (comando = "2") {
				andar = atoi(comando);
			}
			else if (comando = "3") {
				andar = atoi(comando);
			}
			else if (comando = "4") {
				andar = atoi(comando);
			}
			else if (comando = "5") {
				andar = atoi(comando);
			}
			else if (comando = "6") {
				andar = atoi(comando);
			}
			else if (comando = "7") {
				andar = atoi(comando);
			}
			else if (comando = "8") {
				andar = atoi(comando);
			}
			else if (comando = "9") {
				andar = atoi(comando);
			}
			if (andar >= 0 && andar <= 10)
				controlador->Proximos[0] = andar;

			int elevadorMaisProximo = 0;
			Elevador elevadorProx;
			for (int i = 0; i < qtdLifters; i++) {
				if (controlador->elevador[i]->PortaFechada == false) {
					int diferenca = controlador->elevador[i]->Andar - andar;
					if (diferenca < 0)diferenca *= -1;
					if (elevadorMaisProximo < diferenca) elevadorMaisProximo = controlador->elevador[i]->Andar;
					
				}
			}

			for (int i = 0; i < qtdLifters; i++) {
				if (controlador->elevador[i]->Andar == elevadorMaisProximo) {
					if(andar > controlador->elevador[i]->Andar)
					{
						for(int j = controlador->elevador[i]->Andar + 1; j <andar; j++)
						{
							controlador->elevador[i]->ProximosAndares[0] = j;
						}
					}

					else if(andar < controlador->elevador[i]->Andar)
					{
						for(int j = controlador->elevador[i]->Andar - 1; j > andar; j--)
						{
							controlador->elevador[i]->ProximosAndares[0] = j;
						}
					}

					xQueueSend(queueAnalyzer, &controlador->elevador[i], portMAX_DELAY);
					break;
				}
			}
		}
	}
}

void vLiftDoorCheck(void* pvParameters)
{
	Elevador* elevador;
	while (1)
	{
		if (xQueueReceive(queueAnalyzer, &elevador, portMAX_DELAY) == pdPASS) {
			if (elevador->PortaFechada == true) {
				vLiftMetor(elevador);
			}
			else {
				vPrintString("porta aberta fecha isso");
			}
		}
	}
}

void vLiftMetor(Elevador* elevador)
{
	if (elevador->MotorFuncionando == 1) {
		vPrintString("motor subindo");
	}
	else if (elevador->MotorFuncionando == 0) {
		vPrintString("motor parado");
	}
	else if (elevador->MotorFuncionando == -1) {
		vPrintString("motor descendo");
	}
}

static bool CheckAndDebounceD(uint32_t swNum)
{
	bool sw_pressed = false;

	if (!mPORTDReadBits(swNum))
	{
		vTaskDelay(SwDelay);
		if (!mPORTDReadBits(swNum))
			sw_pressed = true;
	}

	return sw_pressed;
}

static bool CheckAndDebounceC(uint32_t swNum)
{
	bool sw_pressed = false;

	if (!mPORTCReadBits(swNum))
	{
		vTaskDelay(SwDelay);
		if (!mPORTCReadBits(swNum))
			sw_pressed = true;
	}

	return sw_pressed;
}

static const TickType_t pollDelay = 100 / portTICK_PERIOD_MS;

bool mPORTDReadBits(uint32_t swNum) {
	return true;
}
bool mPORTCReadBits(uint32_t swNum) {
	return true;
}


// Handle button presses and debouncing
void taskButtons(void* pvParameters)
{
	while (1)
	{
		if (CheckAndDebounceD(SW1))
		{
			vPrintString("1");
			xQueueSend(queueLifter, &"1", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW2))
		{
			vPrintString("2");
			xQueueSend(queueLifter, &"2", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW3))
		{
			vPrintString("3");
			xQueueSend(queueLifter, &"3", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW4))
		{
			vPrintString("4");
			xQueueSend(queueLifter, &"4", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW5))
		{
			vPrintString("5");
			xQueueSend(queueLifter, &"5", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW6))
		{
			vPrintString("6");
			xQueueSend(queueLifter, &"6", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW7))
		{
			vPrintString("7");
			xQueueSend(queueLifter, &"7", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW8))
		{
			vPrintString("8");
			xQueueSend(queueLifter, &"8", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW9))
		{
			vPrintString("9");
			xQueueSend(queueLifter, &"9", portMAX_DELAY);
		}
		if (CheckAndDebounceD(SW10))
		{
			vPrintString("10");
			xQueueSend(queueLifter, &"0", portMAX_DELAY);
		}

		/*
		if (CheckAndDebounceC(SWOpen))
		{
			vPrintString("10");
			xQueueSend(queueLifter, &"O", portMAX_DELAY);

		}

		if (CheckAndDebounceC(SWClose))
		{
			vPrintString("10");
			xQueueSend(queueLifter, &"C", portMAX_DELAY);

		}
		*/
		vTaskDelay(pollDelay);
	}
}


Elevador* CreateLift() {

	Elevador* elevador = malloc(sizeof * elevador);

	elevador->Nome[0] = 'f';
	elevador->Nome[1] = 'u';
	elevador->Nome[2] = 'l';
	elevador->Nome[3] = 'a';
	elevador->Nome[4] = 'n';
	elevador->Nome[5] = 'o';
	elevador->Andar = 0;
	elevador->PortaFechada = false;
	elevador->MotorFuncionando = 0;
	elevador->ProximosAndares[0] = 0;
	elevador->ProximosAndares[1] = 0;
	elevador->ProximosAndares[2] = 0;

	return &elevador;
}