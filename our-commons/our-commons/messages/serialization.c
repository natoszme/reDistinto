/*
 * serialization.c
 *
 *  Created on: 1 may. 2018
 *      Author: utnso
 */

#include "serialization.h"

t_log* logger;

void initSerializationLogger(t_log* _logger) {
	logger = _logger;
}

int send_all(int socket, void* package, int length)
{
    char *auxPointer = (char*) package;
    while (length > 0)
    {
        int i = send(socket, auxPointer, length, MSG_NOSIGNAL);
        if (i < 1) {
        	if (i < 0) log_error(logger, "Error while sending all package: %s", strerror(errno));
        	return CUSTOM_FAILURE;
        }
        auxPointer += i;
        length -= i;
    }
    return CUSTOM_SUCCESS;
}

int recv_all(int socket, void* package, int length)
{
    char *auxPointer = (char*) package;
    while (length > 0)
    {
        int i = recv(socket, auxPointer, length, 0);
        if (i < 1) {
        	if (i < 0) log_error(logger, "Error while receiving all package: %s", strerror(errno));
        	return CUSTOM_FAILURE;
        }
        auxPointer += i;
        length -= i;
    }
    return CUSTOM_SUCCESS;
}

int addToPackageGeneric(void** package, void* value, int size, int* offset) {
	*package = realloc(*package, *offset + size);
	memcpy(*package + *offset, value, size);
	*offset += size;
	return 1;
}

int sendInt(int value, int sendSocket) {
	void* package = NULL;
	int offset = 0;

	int addToPackage(void* value, int size) {
		return addToPackageGeneric(&package, value, size, &offset);
	}

	addToPackage(&value, sizeof(value));

	int result = send_all(sendSocket, package, offset);
	free(package);
	return result;
}

int recieveInt(int* value, int recvSocket) {
	return recv_all(recvSocket, value, sizeof(int));
}


int sendString(char* value, int sendSocket) {
	void* package = NULL;
	int offset = 0;

	int addToPackage(void* value, int size) {
		return addToPackageGeneric(&package, value, size, &offset);
	}

	int sizeValue = value != NULL ? strlen(value) + 1 : 0;

	addToPackage(&sizeValue, sizeof(sizeValue));
	if (sizeValue != 0) addToPackage(value, sizeValue);

	int result = send_all(sendSocket, package, offset);

	free(package);
	return result;
}

int recieveStringBySize(char** stringRef, int sizeString, int recvSocket) {
	char* string = malloc(sizeString);
	*stringRef = string;
	return recv_all(recvSocket, string, sizeString);
}

int recieveString(char** stringRef, int recvSocket) {
	*stringRef = NULL;
	int sizeString;
	return
		recieveInt(&sizeString, recvSocket) &&
		((sizeString != 0) ? recieveStringBySize(stringRef, sizeString, recvSocket): 1);
}

void* generateOperationPackage(Operation* operation, int* offset) {
	void* package = NULL;

	int addToPackage(void* value, int size) {
		return addToPackageGeneric(&package, value, size, offset);
	}

	int sizeKey = strlen(operation->key) + 1;
	int sizeValue = operation->value != NULL ? strlen(operation->value) + 1 : 0;

	addToPackage(&operation->operationCode, sizeof(operation->operationCode));
	addToPackage(&sizeKey, sizeof(sizeKey));
	addToPackage(&sizeValue, sizeof(sizeValue));
	addToPackage(operation->key, sizeKey);
	if (sizeValue != 0) {addToPackage(operation->value, sizeValue);}

	return package;
}

int sendOperation(Operation* operation, int sendSocket) {
	int offset = 0;
	void* package = generateOperationPackage(operation, &offset);

	int result = send_all(sendSocket, package, offset);

	free(package);
	return result;
}

int recieveOperation(Operation** operationRef, int recvSocket) {
	int sizeKey;
	int sizeValue;

	Operation* operation = calloc(1, sizeof(Operation));
	*operationRef = operation;
	operation->value = NULL;

	return
		recv_all(recvSocket, &operation->operationCode, sizeof(char)) &&
		recv_all(recvSocket, &sizeKey, sizeof(int)) &&
		recv_all(recvSocket, &sizeValue, sizeof(int)) &&
		recieveStringBySize(&operation->key, sizeKey, recvSocket) &&
		((sizeValue != 0) ? recieveStringBySize(&operation->value, sizeValue, recvSocket) : 1);
}

int sendStingList(t_list* strings, int sendSocket) {
	int offset = 0;
	void* package = NULL;

	int addToPackage(void* value, int size) {
		return addToPackageGeneric(&package, value, size, &offset);
	}

	void addStringToPackage(char* string) {
		int sizeString = strlen(string) + 1;
		addToPackage(&sizeString, sizeof(sizeString));
		addToPackage(string, sizeString);
	}

	int listSize = list_size(strings);

	addToPackage(&listSize, sizeof(listSize));

	list_iterate(strings, (void*) addStringToPackage);

	int result = send_all(sendSocket, package, offset);

	free(package);
	return result;
}
