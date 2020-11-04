/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

/* Demo includes. */
#include "supporting_functions.h"
#include <semphr.h>
#include <stdlib.h>

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

void vDemo(void* pvParameters);
void vFuncionarElevador(void* pvParameters);

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
	char Nome[7];  
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
	int PonteiroProximos;
} ControleElevador;

Elevador* CreateLift();

QueueHandle_t queueAnalyzer;//Objeto da queue
QueueHandle_t queueLifter;//Objeto da queue
QueueHandle_t queueMotor;//Objeto da queue
/*-----------------------------------------------------------*/

/* Global Variables */
xSemaphoreHandle xSemaphore;
int qtdLifters = 3; 

int main( void )
{
	ControleElevador* controlador = malloc(sizeof * controlador);
	for (int i = 0; i < qtdLifters; i++)
	{
		controlador->elevador[i] = CreateLift();
	}
	controlador->elevador[0]->Nome[6] = '1';
	controlador->elevador[1]->Nome[6] = '2';
	controlador->elevador[2]->Nome[6] = '3';
	for (int i = 0; i < 10; i++)
		controlador->Proximos[i] = 0;
	controlador->PonteiroProximos = 0;
	
	
	/* Create the Semaphore for synchronization between UART and LED task */
	vSemaphoreCreateBinary(xSemaphore)
	//xSemaphoreGive(xSemaphore);
	/* Create one of the two tasks. */
	queueAnalyzer = xQueueCreate(10, sizeof(uint32_t));
	queueLifter = xQueueCreate(10, sizeof(uint32_t));
	queueMotor = xQueueCreate(10, sizeof(uint32_t));

	xTaskCreate(	vDemo,
					"Task Demo",
					1000,
					NULL,
					1,
					NULL);
	
	xTaskCreate(	taskButtons,	
					"Task Buttons",
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
	
	xTaskCreate(	vLiftAnalyzer,
					"Task analyzer",
					1000,	
					NULL,	
					1,		
					NULL);	
	
	//xTaskCreate(	vFuncionarElevador,
	//				"Task Elevador",
	//				1000,
	//				NULL,
	//				1,
	//				NULL);
		

	vTaskStartScheduler();	

	for( ;; );
	return 0;
}

/*
	Logica do controlador para manipular os elevadores.
*/
void vLiftController(void* pvParameters)
{
	const char* pcTaskName = "\r\nReceived command: ";
	ControleElevador* controlador = (ControleElevador*)pvParameters;

	char* comando;
	int andar = -1;
	while (1)
	{
		if (xQueueReceive(queueLifter, &comando, portMAX_DELAY) == pdPASS) {
			vPrintString(pcTaskName);
			vPrintString(comando);
			vPrintString("\r\n");
			
			if (*comando == *"0") {
				andar = atoi(comando);
			}
			else if (*comando == *"1") {
				andar = atoi(comando);
			}
			else if (*comando == *"2") {
				andar = atoi(comando);
			}
			else if (*comando == *"3") {
				andar = atoi(comando);
			}
			else if (*comando == *"4") {
				andar = atoi(comando);
			}
			else if (*comando == *"5") {
				andar = atoi(comando);
			}
			else if (*comando == *"6") {
				andar = atoi(comando);
			}
			else if (*comando == *"7") {
				andar = atoi(comando);
			}
			else if (*comando == *"8") {
				andar = atoi(comando);
			}
			else if (*comando == *"9") {
				andar = atoi(comando);
			}
			else {}

			/*
			Pega o elevador mais proximo
			*/
			int elevadorMaisProximo = 0;
			for (int i = 0; i < qtdLifters; i++) {
				if (controlador->elevador[i]->PortaFechada == false) {
					int diferenca = controlador->elevador[i]->Andar - andar;

					if (diferenca < 0)
						diferenca *= -1;

					if (elevadorMaisProximo < diferenca) 
						elevadorMaisProximo = controlador->elevador[i]->Andar;
				}
			}

			for (int i = 0; i < qtdLifters; i++) {
				if (controlador->elevador[i]->Andar == elevadorMaisProximo && controlador->elevador[i]->PortaFechada == false) {
					vPrintString("Enviar elevador: ");
					vPrintString(controlador->elevador[i]->Nome);

					if ( 0> andar - controlador->elevador[i]->Andar)
						controlador->elevador[i]->MotorFuncionando = -1;
					else
						controlador->elevador[i]->MotorFuncionando = 1;


					controlador->elevador[i]->PortaFechada = true;
					controlador->elevador[i]->MotorFuncionando = true;

					xQueueSend(queueAnalyzer, &controlador->elevador[i], portMAX_DELAY);
					xQueueSend(queueMotor, &controlador->elevador[i], portMAX_DELAY);
					
					break;
				}
			}

			if (controlador->elevador[0]->MotorFuncionando == true && controlador->elevador[1]->MotorFuncionando == true && controlador->elevador[2]->MotorFuncionando == true)
			{
				int ponteiro = controlador->PonteiroProximos;
				controlador->Proximos[ponteiro] = andar;
				controlador->PonteiroProximos++;
				vPrintString("Proximos: ");
				for (int x = 0; x < 10; x++) {
					vPrintString("[");
					char buffer[1];
					int buf = controlador->Proximos[x];
					itoa(buf, buffer, 10);
					vPrintString(&buffer);
					vPrintString("]");
				}
				//vPrintString(controlador->Proximos);
			}
		}
	}
}

/*-----------------------------------------------------------*/

void vDemo(void* pvParameters)
{
	const char* pcTaskName = "\r\nTask demo is running\r\n";
	for (;; )
	{
		vPrintString(pcTaskName);

		time_t t;
		/* Intializes random number generator */
		//srand((unsigned)time(&t));

		int escolhido = 0;
		escolhido = (rand() % 10) + 0;

		char numero[1];
		itoa(escolhido, numero, 10);

		char* pChar = &numero;
		vPrintString("Sending number: ");
		vPrintString(numero);

		xQueueSend(queueLifter, &pChar, portMAX_DELAY);

		vTaskDelay(400, portTICK_PERIOD_MS);
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
			if (elevador->PortaFechada) vPrintString("fechada");
			else vPrintString("aberta");
			
			vPrintString("\nmotor: ");
			if (elevador->MotorFuncionando>0) vPrintString("rodando");
			else if (elevador->MotorFuncionando == 0) vPrintString("parado");
			else vPrintString("desligado");

			vPrintString("\n--------------------------\n");
		}
	}
}


void vLiftDoorCheck(void* pvParameters)
{
	Elevador* elevador;
	while (100)
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


void vFuncionarElevador(void* pvParameters)
{

	Elevador* elevador;
	while (100)
	{
		vPrintString("teste");
		if (xQueueReceive(queueMotor, &elevador, portMAX_DELAY) == pdPASS) {
				vPrintString("teste");
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

	return elevador;
}