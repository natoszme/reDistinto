/*
 * coordinador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_COORDINADOR_H_
#define SRC_COORDINADOR_H_

	#include <our-commons/sockets/server.h>
	#include <our-commons/modules/names.h>
	#include <our-commons/messages/operation_codes.h>
	#include <our-commons/messages/serialization.h>
	#include <our-commons/tads/tads.h>
	//#include <commons/string.h>
	#include <commons/config.h>
	#include <commons/collections/list.h>
	#include <commons/log.h>
	#include <pthread.h>
	#include "submodules/instancia/instanciaFunctions.h"

	#define  CFG_FILE "../coordinador.cfg"

	//TODO revisar que hace esto aca
	#define EXECUTION_ERROR -1

	typedef struct EsiRequest{
		int id;
		int socket;
		Operation* operation;
	}EsiRequest;

	char* getOperationName(Operation* operation);
	void showConfig(int listeningPort);
	void getConfig(int* listeningPort);

	int welcomePlanificador(int coordinadorSocket, int planificadorSocket);
	void freeResources();
	void planificadorFell();

	t_list* instancias;
	char instanciaResponseStatus;

#endif /* SRC_COORDINADOR_H_ */
