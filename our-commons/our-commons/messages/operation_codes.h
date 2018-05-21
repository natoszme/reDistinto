/*
 * operation_codes.h
 *
 *  Created on: 27 abr. 2018
 *      Author: utnso
 */

#ifndef OPERATION_CODES_H_
#define OPERATION_CODES_H_

#define CUSTOM_SUCCESS 0
#define CUSTOM_FAILURE 1

#define SUCCESS '1'
#define FINISHED '2'
#define ABORT '3'
#define OURSET '4'
#define OURGET '5'
#define RUN '6'
#define COMPACT '7'
#define OURSTORE '8'
#define LOCK '9'
#define ERROR_CLAVE_SIZE 'a'
#define ERROR_CLAVE_NOT_IDENTIFIED 'q'
#define ERROR_CLAVE_NON_EXISTENT 'w'
#define ERROR_CLAVE_NOT_FOUND 'e'
#define BLOCK 'r'
#define NOTBLOCKED 'u'
#define FREE 't'
#define NOTFINISHED 'y'
#define BLOCKED 'i'
#define LOCKED 'o'

typedef struct OperationResponse {
	char coordinadorResponse;
	char esiStatus;
} __attribute__((packed)) OperationResponse;


typedef struct InstanciaConfiguration {
	int entriesAmount;
	char entrySize;
} __attribute__((packed)) InstanciaConfiguration;

#endif /* OPERATION_CODES_H_ */
