/*
 * tads.h
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#ifndef TADS_H_
#define TADS_H_

	typedef struct Operation{
		char operationCode;
		char* key;
		char* value;
	}Operation;

	typedef struct OperationResponse {
		char coordinadorResponse;
		char esiStatus;
	} __attribute__((packed)) OperationResponse;


	typedef struct InstanciaConfiguration {
		int entriesAmount;
		char entrySize;
	} __attribute__((packed)) InstanciaConfiguration;

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <commons/log.h>
	#include "../messages/operation_codes.h"

	char* getOperationName(Operation* operation);
	void showOperation(Operation* operation, t_log* logger);
	char* getKeyStatusName(char keyStatus);
	void destroyOperation(Operation* operation);

#endif /* TADS_H_ */
