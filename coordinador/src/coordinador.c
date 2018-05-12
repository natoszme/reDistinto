/*
 * coordinador.c
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#include "coordinador.h"
//TODO cambiar esta por la posta
#define CLAVE_PROVISORIA_ERROR_A_PLANIFICADOR '30'
#define CLAVE_PROVISORIA_CLABE_BLOQUEADA_DE_PLANIFICADOR '31'

t_log* logger;
t_log* operationsLogger;

int planificadorSocket;
void setDistributionAlgorithm(char* algorithm);
Instancia* (*distributionAlgorithm)(char* keyToBeBlocked);
t_list* instancias;
t_list* fallenInstancias;
int delay;
int lastInstanciaChosen = 0;
int firstAlreadyPass = 0;

int main(void) {
	logger = log_create("../coordinador.log", "tpSO", true, LOG_LEVEL_INFO);
	operationsLogger = log_create("../logOperaciones.log", "tpSO", true, LOG_LEVEL_INFO);

	int listeningPort;
	char* algorithm;
	int cantEntry;
	int entrySize;
	getConfig(&listeningPort, &algorithm, &cantEntry, &entrySize, &delay);

	printf("Puerto = %d\n", listeningPort);
	printf("Algoritmo = %s\n", algorithm);
	printf("Cant entry = %d\n", cantEntry);
	printf("Entry size= %d\n", entrySize);
	printf("delay= %d\n", delay);

	setDistributionAlgorithm(algorithm);

	instancias = list_create();
	fallenInstancias = list_create();

	welcomeClient(listeningPort, COORDINADOR, PLANIFICADOR, 10, &welcomePlanificador, logger);

	return 0;
}

void getConfig(int* listeningPort, char** algorithm, int* cantEntry, int* entrySize, int* delay){
	t_config* config;
	config = config_create(CFG_FILE);
	*listeningPort = config_get_int_value(config, "LISTENING_PORT");
	*algorithm = config_get_string_value(config, "ALGORITHM");
	*cantEntry = config_get_int_value(config, "CANT_ENTRY");
	*entrySize = config_get_int_value(config, "ENTRY_SIZE");
	*delay = config_get_int_value(config, "DELAY");
}

int instanciaIsNextToActual(Instancia* instancia){
	if(lastInstanciaChosen == 0 && firstAlreadyPass == 0){
		firstAlreadyPass = 1;
		return 1;
	}
	return instancia->id >= lastInstanciaChosen + 1;
}

Instancia* equitativeLoad(char* keyToBeBlocked){
	Instancia* instancia = list_find(instancias, (void*) &instanciaIsNextToActual);
	if(instancia == NULL){
		instancia = list_get(instancias, 0);
	}
	lastInstanciaChosen = instancia->id;
	return instancia;
}

Instancia* leastSpaceUsed(char* keyToBeBlocked){
	//TODO
	return list_get(instancias, 0);
}

Instancia* keyExplicit(char* keyToBeBlocked){
	//TODO
	return list_get(instancias, 0);
}

void setDistributionAlgorithm(char* algorithm){
	if(strcmp(algorithm, "EL") == 0){
		distributionAlgorithm = &equitativeLoad;
	}else if(strcmp(algorithm, "LSU") == 0){
		distributionAlgorithm = &leastSpaceUsed;
	}else if(strcmp(algorithm, "KE") == 0){
		distributionAlgorithm = &keyExplicit;
	}else{
		printf("Couldn't determine the distribution algorithm\n");
		//loggear el error
		//matar al coordinador
	}
}

Instancia* chooseInstancia(char* keyToBeBlocked){
	//TODO este if podria estar demas
	if(list_size(instancias) != 0){
		return (*distributionAlgorithm)(keyToBeBlocked);
	}
	return NULL;
}

int sendResponseToEsi(EsiRequest* esiRequest, int response, char** stringToLog){
	//TODO aca tambien hay que reintentar hasta que se mande todo?
	//TODO que pasa cuando se pasa una constante por parametro? vimos que hubo drama con eso
	if(send(esiRequest->socket, &response, sizeof(int), 0) < 0){
		sprintf(*stringToLog, "ESI %d hizo perdio conexion con el coordinador al intentar hacer %s", esiRequest->id, getOperationName(esiRequest->operation->operationCode));
		return -1;
	}
	return 0;
}

int getActualEsiID(){
	int esiId = 0;
	int recvResult = recv(planificadorSocket, &esiId, sizeof(int), 0);
	if(recvResult <= 0){
		if(recvResult == 0)
		log_error(logger, "Planificador disconnected from coordinador, quitting...");
		//TODO decidamos: de aca en mas hacemos exit si muere la conexion con el planificador?
		//Misma decision que en recieveKeyStatusFromPlanificador
		exit(-1);
	}

	return esiId;
}

int getActualEsiIDDummy(){
	return 0;
}

void logOperation(char* stringToLog){
	log_info(operationsLogger, stringToLog);
}

Instancia* lookForKey(char* key, t_list* instanciasList){
	int isLookedKey(char* actualKey){
		if(strcmp(actualKey, key)){
			return 0;
		}
		return 1;
	}

	int isKeyInInstancia(Instancia* instancia){
		if(list_any_satisfy(instancia->storedKeys, (void*) &isLookedKey)){
			return 1;
		}
		return 0;
	}

	//TODO hay que castear el return del find? Supongo que no por el tipo de retorno de esta func
	return list_find(instanciasList, (void*) &isKeyInInstancia);
}

int fallenInstanciaThatHasKey(char* key){
	return lookForKey(key, fallenInstancias);
}

Instancia* lookForOrChoseInstancia(char* key, int* keyExists){
	Instancia* chosenInstancia = lookForKey(key, instancias);

	if(chosenInstancia == NULL){
		chosenInstancia = chooseInstancia(key);
	}else{
		*keyExists = 1;
	}

	//TODO sacar este show
	showInstancia(chosenInstancia);

	return chosenInstancia;
}

int instanciaDoOperation(Instancia* instancia, Operation* operation){
	//TODO validar retorno
	sendOperation(operation, instancia->socket);
	//TODO ojo, que esto tiene un recv adentro y el hilo de la instancia tambien esta haciendo uno
	return waitForInstanciaResponse(instancia);
}

char* getOperationName(Operation* operation){
	//ver si se puede evitar repetir este switch
	switch(operation->operationCode){
		case OURSET:
			return "SET";
			break;
		case OURSTORE:
			return "STORE";
			break;
		case OURGET:
			return "GET";
			break;
		default:
			return "UNKNOWN OPERATION";
			break;
	}
}

int lookForKeyAndExecuteOperation(EsiRequest* esiRequest, char** stringToLog){
	Instancia* chosenInstancia = lookForKey(esiRequest->operation->key, instancias);
	if(chosenInstancia == NULL){
		chosenInstancia = fallenInstanciaThatHasKey(esiRequest->operation->key);
		if(chosenInstancia){
			sprintf(*stringToLog, "ESI %d intenta hacer %s sobre la clave %s. Clave inaccesible", esiRequest->id, getOperationName(esiRequest->operation), esiRequest->operation->key);
		}else{
			sprintf(*stringToLog, "ESI %d intenta hacer %s sobre la clave %s. Clave no identificada", esiRequest->id, getOperationName(esiRequest->operation), esiRequest->operation->key);
		}
		sendResponseToEsi(esiRequest, ABORT, stringToLog);
		return -1;
	}

	showInstancia(chosenInstancia);

	int response = instanciaDoOperation(chosenInstancia, esiRequest->operation);

	if(response < 0){
		sprintf(*stringToLog, "ESI %d no puede hacer %s sobre %s. Instancia %d se cayo", esiRequest->id, getOperationName(esiRequest->operation), esiRequest->operation->key, chosenInstancia->id);
		instanciaHasFallen(chosenInstancia, instancias, fallenInstancias);
		//TODO que pasa en este caso?
		//sendResponseToEsi(esiRequest->socket, BLOCK);

		//TODO ojo que no se va a matar al hilo del esi
		return 0;
	}

	sprintf(*stringToLog, "ESI %d hizo %s sobre la clave %s", esiRequest->id, getOperationName(esiRequest->operation), esiRequest->operation->key);

	return sendResponseToEsi(esiRequest, SUCCESS, stringToLog);
}

int doSet(EsiRequest* esiRequest, char keyStatus, char** stringToLog){

	if(keyStatus != LOCKED){
		sprintf(*stringToLog, "ESI %d no puede hacer SET sobre la clave %s. Clave no bloqueada por el", esiRequest->id, esiRequest->operation->key);
		return -1;
	}

	if(lookForKeyAndExecuteOperation(esiRequest, stringToLog) < 0){
		return -1;
	}

	return 0;
}

int doStore(EsiRequest* esiRequest, char keyStatus, char** stringToLog){

	if(keyStatus != LOCKED){
		sprintf(*stringToLog, "ESI %d no puede hacer STORE sobre la clave %s. Clave no bloqueada por el", esiRequest->id, esiRequest->operation->key);
		return -1;
	}

	if(lookForKeyAndExecuteOperation(esiRequest, stringToLog) < 0){
		return -1;
	}

	return 0;
}

//doget, set y store deben devolver 0 si salio ok y -1 si muere el hilo del esi
int doGet(EsiRequest* esiRequest, char keyStatus, char** stringToLog){
	if(keyStatus == BLOCKED){
		sprintf(*stringToLog, "ESI %d intenta hacer GET sobre la clave %s. Clave bloqueada", esiRequest->id, esiRequest->operation->key);
		//no se mata al esi en caso de clave inaccesible por si luego se recupera la clave
		return sendResponseToEsi(esiRequest, BLOCK, stringToLog);
	}

	//la clave esta tomada por el o libre, fijarse si la clave esta en instancia caida

	//TODO me parece que esta parte se valida siempre, no solo en caso de GET
	Instancia* chosenInstancia = fallenInstanciaThatHasKey(esiRequest->operation->key);
	if(chosenInstancia != NULL){

		sprintf(*stringToLog, "ESI %d intenta hacer GET sobre la clave %s. Clave inaccesible", esiRequest->id, esiRequest->operation->key);
		sendResponseToEsi(esiRequest, ABORT, stringToLog);
		return -1;
	}

	int keyExists = 0;
	chosenInstancia = lookForOrChoseInstancia(esiRequest->operation->key, &keyExists);

	if(chosenInstancia == NULL){
		sprintf(*stringToLog, "ESI %d es abortado por no existir la clave en ninguna instancia y no haber instancias disponibles", esiRequest->id);
		sendResponseToEsi(esiRequest, ABORT, stringToLog);
		return -1;
	}

	if(keyExists == 0){
		addKeyToInstanciaStruct(chosenInstancia, esiRequest->operation->key);
	}

	showInstancia(chosenInstancia);

	if(sendResponseToEsi(esiRequest, LOCK, stringToLog) < 0){
		//se cayo la conexion con el esi. muere su hilo. el planificador deberia enterarse por su select
		return -1;
	}

	if(keyStatus == LOCKED){
		sprintf(*stringToLog, "ESI %d hace GET sobre la clave %s, que ya tenia", esiRequest->id, esiRequest->operation->key);
		//TODO chequear con planificador si espera que en este caso tambien se le mande LOCK
	}else{
		sprintf(*stringToLog, "ESI %d hizo GET sobre la clave %s", esiRequest->id, esiRequest->operation->key);
	}
	return 0;
}

char checkKeyStatusFromPlanificador(int esiId, char* key){
	char response = 0;

	//TODO enviar la clave al planificador (no la operacion)

	int recvResult = recv(planificadorSocket, &response, sizeof(char), 0);
	if(recvResult <= 0){
		if(recvResult == 0){
			log_error(logger, "Planificador disconnected from coordinador, quitting...");
			//TODO decidamos: de aca en mas hacemos exit si muere la conexion con el planificador?
			exit(-1);
		}
	}
	return response;
}

char checkKeyStatusFromPlanificadorDummy(){
	return BLOCKED;
}

void recieveOperationDummy(Operation* operation){
	operation->key = "lio:messi";
	operation->value = "elMasCapo";
	operation->operationCode = OURSTORE;
}

void showOperation(Operation* operation){
	printf("Operation key = %s\n", operation->key);
}

int recieveStentenceToProcess(int esiSocket){
	int operationResult = 0;
	int esiId = 0;
	//esiId = getActualEsiID();
	esiId = getActualEsiIDDummy();

	//TODO chequear esto para evitar alocar demas. por otro lado, evitar alocar por cada sentencia...
	char* stringToLog = malloc(200);

	Operation* operation = malloc(sizeof(Operation));
	//TODO descomentar, esta asi para probar
	/*if(recieveOperation(operation, esiSocket) < 1){
		//TODO en este caso se mata al esi?
		informPlanificador(operation, FALLA);
	}*/
	recieveOperationDummy(operation);
	showOperation(operation);

	char keyStatus;
	//TODO descomentar
	//keyStatus = checkKeyStatusFromPlanificador(esiId, operation->key);
	keyStatus = checkKeyStatusFromPlanificadorDummy();

	EsiRequest esiRequest;
	esiRequest.id = esiId;
	esiRequest.operation = operation;
	esiRequest.socket = esiSocket;

	//TODO agregar estos casos a cada operacion
	/*if((operation->operationCode == OURGET && keyStatus == NOTBLOCKED) || (!(operation->operationCode == OURGET) && keyStatus == LOCK)){
		if(keyIsInFallenInstancia(operation->key)){
			//TODO logear clave en instancia caida
			removeKeyFromFallenInstancia(operation->key, NULL);
			return -1;
		}
	}*/

	//TODO validar los siguientes casos, los 3, por si es necesario matar al hilo
	//(y cada uno de ellos avisara al esi lo que corresponda)
	switch (esiRequest.operation->operationCode){
		case OURSET:
			if(doSet(&esiRequest, keyStatus, &stringToLog) < 0){
				operationResult = -1;
			}
			break;
		case OURSTORE:
			if(doStore(&esiRequest, keyStatus, &stringToLog)){
				operationResult = -1;
			}
			break;
		case OURGET:
			if (doGet(&esiRequest, keyStatus, &stringToLog) < 0){
				operationResult = -1;
			}
			break;
		default:
			//deberiamos matar al esi?
			//TODO se puede hacer informPlanificador con un error, pero hay que ver si esta esperando eso
			break;
	}

	logOperation(stringToLog);

	free(operation);
	free(stringToLog);
	sleep(delay);
	return operationResult == 0 ? 1 : -1;
}

int handleInstancia(int instanciaSocket){
	log_info(logger, "An instancia thread was created\n");
	//TODO hay que meter un semaforo para evitar conflictos de los diferentes hilos

	//TODO que pasa si una instancia se recupera? como la distinguimos?
	//si seguimos este camino, se va a crear una nueva y no queremos
	createNewInstancia(instanciaSocket, instancias);

	int recvResult = 0;
	while(1){
		int instanciaResponse = 0;
		recvResult = recv(instanciaSocket, &instanciaResponse, sizeof(int), 0);
		if(recvResult <= 0){
			if(recvResult == 0){
				printf("Instancia on socket %d has fallen\n", instanciaSocket);

				//handlear que pasa en este caso. podriamos guardar el id de la instancia caida, sacar la instancia de la lista...
				//de instancias. Despues, cuando se reincorpore, levantarla de ahi

				close(instanciaSocket);

				//si se cae la instancia, se cae su hilo
				break;
			}
		}else{
			//TODO handlear la respuesta normal de ejecucion en una instancia
		}
	}
	return 0;
}

int handleEsi(int esiSocket){
	log_info(logger,"An esi thread was created\n");
	while(1){
		if (recieveStentenceToProcess(esiSocket) < 0){
			//va a matar al hilo porque sale de este while!
			break;
		}
	}
	return 0;
}

void pthreadInitialize(void* clientSocket){
	int castedClientSocket = *((int*) clientSocket);
	int id = recieveClientId(castedClientSocket, COORDINADOR, logger);

	if (id == 11){
		handleInstancia(castedClientSocket);
	}else if(id == 12){
		handleEsi(castedClientSocket);
	}else{
		log_info(logger,"I received a strange\n");
	}

	free(clientSocket);
}

int clientHandler(int clientSocket){
	pthread_t clientThread;
	int* clientSocketPointer = malloc(sizeof(int));
	*clientSocketPointer = clientSocket;
	if(pthread_create(&clientThread, NULL, (void*) &pthreadInitialize, clientSocketPointer)){
		log_error(logger, "Error creating thread\n");
		return -1;
	}

	if(pthread_detach(clientThread) != 0){
		log_error(logger,"Couldn't detach thread\n");
		return -1;
	}

	return 0;
}

int welcomePlanificador(int coordinadorSocket, int newPlanificadorSocket){
	planificadorSocket = newPlanificadorSocket;
	log_info(logger, "%s recieved %s, so it'll now start listening esi/instancia connections\n", COORDINADOR, PLANIFICADOR);
	while(1){
		int clientSocket = acceptUnknownClient(coordinadorSocket, COORDINADOR, logger);
		//TODO validar el retorno
		clientHandler(clientSocket);
	}

	return 0;
}
