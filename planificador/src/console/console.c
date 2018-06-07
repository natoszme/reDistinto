/*
 * consola.c
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#include "console.h"

t_log* logger;

void openConsole() {
	char* line;
	char** parameters;
	printf("Listado de comandos\n1.Pausar\n2.Continuar\n3.Bloquear <clave> <id>\n4.Desbloquear <clave>\n5.Listar <recurso>\n6.Kill <ID>\n7.Status <clave>\n8.Deadlock\n");

	while(1) {
		line = readline("> ");
		string_to_lower(line);
		parameters = string_split(line," ");

		//Borrar estos printf cuando todo funcione
		printf("Parameter quantity = %d\n", parameterQuantity(parameters));
		for(int i = 0;i < parameterQuantity(parameters); i++) {
			printf("Parametro %d = %s\n", i+1, parameters[i]);
		}

		/*if(line) {
			add_history(line);
		}*/

		/*if(!strncmp(line, "exit", 4)) {
			free(line);
			break;
		}*/


		list_add(instruccionsByConsoleList,parameters);
		log_info(logger,"Command added to instruccionsByConsole list");




		free(line);
	}
}

void execute(char** parameters) {
	char* command = parameters[0];
	int commandNumber = getCommandNumber(command);
	// Borrar cuando todo funcione
	log_info(logger,"Execute command : %s\n", command);
	char* key = malloc(40);
	int esiID;
	t_queue* blockedEsis = malloc(sizeof(t_queue));
	Esi* esi = malloc(sizeof(Esi));
	switch(commandNumber) {
		case PAUSAR:
			pauseState = PAUSE;
			log_info(logger,"Execution paused by console");
			//sem_wait(&pauseStateSemaphore);

		break;
		case CONTINUAR:
			pauseState = CONTINUE;
			log_info(logger,"Execution continued by console");
			//sem_post(&pauseStateSemaphore);

		break;
		case BLOQUEAR:
		    key = parameters[1];
		    esiID = atoi(parameters[2]);
		    if(!isTakenResource(key))
		    	takeResource(key,CONSOLE_BLOCKED);
			blockEsi(key,esiID);

			printf("ESI %d was blocked in %s resource:\n", esiID,key);
		break;
		case DESBLOQUEAR:

		break;
		case LISTAR:
			 key = parameters[1];
			 blockedEsis = (t_queue*)dictionary_get(blockedEsiDic,key);
			 printf("LOGGG");
			/* if(queue_is_empty(blockedEsis))
				 printf("There are no blocked esis in key %s\n",key);*/
			 for(int i=0;i<queue_size(blockedEsis);i++){
				 esi = queue_pop(blockedEsis);
				 printEsi(esi);
				 queue_push(blockedEsis,esi);
			 }
			 printf("LOGGGGG 2");
		break;
		case KILL:

		break;
		case STATUS:

		break;
		case DEADLOCK:

		break;

		default:
			printf("%s: command not found\n", command);
		break;
	}
}

int parameterQuantity(char** parameters){
	int i=0;
	while(parameters[i]){
		i++;
	}
	return i;
}

int validCommand(char** parameters) {
	if(parameterQuantity(parameters)==0){
		printf("No command was written\n");
		return 0;
	}
	char* command = parameters[0];
	int commandNumber = getCommandNumber(command);
	int cantExtraParameters = parameterQuantity(parameters) - 1;

	char* key = malloc(40);
	int id;
	switch(commandNumber) {
		case PAUSAR:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 0);
		break;
		case CONTINUAR:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 0);
		break;
		case BLOQUEAR:
			if(parameterQuantityIsValid(cantExtraParameters, 2)){
				strcpy(key,parameters[1]);
				id = atoi(parameters[2]); //Falta validar que sea un numero
				if(validateBloquear(key,id)){
					free(key);
					return 1;
				}
			}
			free(key);
			return 0;
		break;
		case DESBLOQUEAR:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 1);
		break;
		case LISTAR:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 1)&&keyExists(parameters[1]);
		break;
		case KILL:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 1);
		break;
		case STATUS:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 1);
		break;
		case DEADLOCK:
			free(key);
			return parameterQuantityIsValid(cantExtraParameters, 0);
		break;

		default:
			printf("%s: command not found\n", command);
			free(key);
			return 0;
		break;
	}
}

int validateBloquear(char* key,int id){
	if(keyExists(key)&&(isReady(id)||isRunning(id))){
		return 1;
	}
	return 0;
}

int keyExists(char* key){
	if(dictionary_has_key(blockedEsiDic,key)){
		printf("key %s exists\n",key);
		return 1;

	}
	printf("key %s doesn't exists\n",key);
	return 0;

}
int isReady(int idEsi){
	if(list_is_empty(readyEsis)){
		printf("Ready list is empty\n");
		return 0;
	}
	for(int i = 0;i<list_size(readyEsis);i++){
		if(((Esi*)list_get(readyEsis,i))->id==idEsi){
			printf("Esi %d is  ready\n",idEsi);
			return 1;
		}
	}
	printf("Esi %d is not ready\n",idEsi);
	return 0;
}
int isRunning(int idEsi){
	if(runningEsi!=NULL && runningEsi->id==idEsi){
		printf("Esi %d is running\n",idEsi);
		return 1;
	}
	printf("Esi %d is not running\n",idEsi);
	return 0;
}

int getCommandNumber(char* command) {
	if(!strcmp(command, "pausar")) {
		return PAUSAR;
	} else if(!strcmp(command, "continuar")) {
		return CONTINUAR;
	} else if(!strcmp(command, "bloquear")) {
		return BLOQUEAR;
	} else if(!strcmp(command, "desbloquear")) {
		return DESBLOQUEAR;
	} else if(!strcmp(command, "listar")) {
		return LISTAR;
	} else if(!strcmp(command, "kill")) {
		return KILL;
	} else if(!strcmp(command, "status")) {
		return STATUS;
	} else if(!strcmp(command, "deadlock")) {
		return DEADLOCK;
	} else if(atoi(command) >= 1 && atoi(command) <= 8) {
		return atoi(command);
	} else {
		return INVALID_COMMAND;
	}
}

int parameterQuantityIsValid(int cantExtraParameters, int necessaryParameters) {
	if (cantExtraParameters != necessaryParameters) {
		printf("Invalid parameter quantity\n");
		return 0;
	}
	return 1;
}
