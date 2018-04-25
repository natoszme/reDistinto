/*
 * coordinador.c
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#include "planificador.h"

<<<<<<< HEAD



t_dictionary* blockedEsiDic;
t_list* readyEsis;
t_list* finishedEsis;
Esi* runningEsi;
=======
>>>>>>> e8458a6c6caf6b17f22d8c8ad70fba153b758e65
int listeningPort;
char* algorithm;
int alphaEstimation;
int initialEstimation;
char* ipCoordinador;
int portCoordinador;
char** blockedKeys;

int welcomeNewClients();

int main(void) {

	pthread_t threadConsole;

	getConfig(&listeningPort, &algorithm,&alphaEstimation, &initialEstimation, &ipCoordinador, &portCoordinador, &blockedKeys);

	blockedEsiDic = dictionary_create();
	addConfigurationBlockedKeys(blockedKeys);
	readyEsis = list_create();
	finishedEsis = list_create();
	printf("Genero los esi para testear\n");
	generateTestEsi();
	int welcomeResponse = welcomeServer(ipCoordinador, portCoordinador, COORDINADOR, PLANIFICADOR, 10, &welcomeNewClients);
	if (welcomeResponse < 0){
		//reintentar?
	}

	/*
	 * Planificador console
	 * */
	pthread_create(&threadConsole, NULL, (void *) openConsole, NULL);
	pthread_join(threadConsole, NULL);
	/*
	 *  Planificador console
	 * */

	return 0;
}

void executeEsi(int esiID){
	//Obtengo el socket del ESI con ID = esiID

	//Le mando un mensaje al socket

	//Lanzo un hilo para esperar la respuesta del ESI

	//Puedo obtener que se ejecuto correctamente, que se ejecuto correctamente Y FINALIZO o un FALLO en la operacion
}

void generateTestEsi(){
	Esi* esi1 = createEsi(1,initialEstimation,1);
	Esi* esi2 = createEsi(2,initialEstimation,2);
	Esi* esi3 = createEsi(3,initialEstimation,3);
	list_add(readyEsis,(void*)esi1);
	printf("Agregue el esi con id=%d\n",esi1->id);
	list_add(readyEsis,(void*)esi2);
	printf("Agregue el esi con id=%d\n",esi2->id);
	runningEsi = esi3;
}

void addConfigurationBlockedKeys(char** blockedKeys){
	int i = 0;
	t_queue* newBlockedKey = queue_create();
	while(blockedKeys[i]){
		dictionary_put(blockedEsiDic,blockedKeys[i],(void*)newBlockedKey);
		i++;
	}
}

void getConfig(int* listeningPort, char** algorithm,int* alphaEstimation, int* initialEstimation, char** ipCoordinador, int* portCoordinador, char*** blockedKeys){

	t_config* config;
	config = config_create(CFG_FILE);
	*listeningPort = config_get_int_value(config, "LISTENING_PORT");
	*algorithm = config_get_string_value(config, "ALGORITHM");
	*alphaEstimation = config_get_int_value(config, "ALPHA_ESTIMATION");
	*initialEstimation = config_get_int_value(config, "ESTIMATION");
	*ipCoordinador = config_get_string_value(config, "IP_COORDINADOR");
	*portCoordinador = config_get_int_value(config, "PORT_COORDINADOR");
	*blockedKeys = config_get_array_value(config, "BLOCKED_KEYS");
}

int welcomeEsi(){
	printf("I received an esi\n");

	return 0;
}

int clientHandler(int* clientSocket){
	int clientId = *((int*) clientSocket);

	if (clientId == 13){
		welcomeEsi();
	}else{
		printf("I received a strange\n");
	}

	return 0;
}

int welcomeNewClients(){
	/*int welcomeEsiResult = welcomeClient(8082, COORDINADOR, ESI, 12, &welcomeEsi);
	if(welcomeEsiResult < 0){
		//TODO: que pasa en este caso?
		return -1;
	}*/

	handleConcurrence(listeningPort, &clientHandler, PLANIFICADOR);

	return 0;
}
