/*
 * planificador.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_PLANIFICADOR_H_
#define SRC_PLANIFICADOR_H_

	#include <our-commons/sockets/client.h>
	#include <our-commons/sockets/server.h>
	#include <our-commons/modules/names.h>
	#include "console/console.h"
#include <commons/string.h>
	#include <commons/config.h>
	#include <commons/collections/dictionary.h>
	#include <commons/collections/queue.h>
	#include <pthread.h>

	#define  CFG_FILE "../planificador.cfg"
	#define USERBLOCKED -1


	void getConfig(int* listeningPort, char** algorithm,int* alphaEstimation, int* initialEstimation, char** ipCoordinador, int* portCoordinador, char*** blockedKeys);
	t_dictionary* blockedEsiDic;

	int listeningPort;
	char* algorithm;
	int initialEstimation;
	char* ipCoordinador;
	int portCoordinador;
	char** blockedKeys;

	void getConfig(int* listeningPort, char** algorithm, int* alphaEstimation,int* initialEstimation, char** ipCoordinador, int* portCoordinador, char*** blockedKeys);


	int welcomeEsi();
	int welcomeCoordinador();
	void addConfigurationBlockedKeys(char**);
#endif /* SRC_PLANIFICADOR_H_ */
