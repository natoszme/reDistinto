/*
 * client.h
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#ifndef CLIENT_H_
#define CLIENT_H_

	#include "sockets.h"
	#include <string.h>
	#include <commons/log.h>

	int connectToServer(const char* serverIP, int serverPort, const char* serverName, const char* clientName, t_log* logger);
	int welcomeServer(const char* serverIp, int serverPort, const char* serverName, const char* clientName, char handshakeValue,
			int (*welcomeProcedure)(int serverSocket), t_log* logger);
	int sendMyIdToServer(int serverSocket, char clientId, const char* clientName, t_log* logger);

#endif /* CLIENT_H_ */
