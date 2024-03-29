/*
* define protocol
* define message structure for protocol
* feature: sendMess, recvMess
*/
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include "status.h"

#define BUFF_SIZE 2000
#define PAYLOAD_SIZE 1024
#define LENGTH_SIZE 2
#define TIME_OUT 1200
#define COMMAND_USER "USER"
#define COMMAND_PASSWORD "PASS"
#define COMMAND_LOGOUT "LOGOUT"
#define COMMAND_REGISTER "REGISTER"
#define MY_ZIP_EXTENSION "-tmpfileta2209.zip"

typedef enum {
	TYPE_AUTHENTICATE,
	TYPE_REQUEST_FILE,
	TYPE_REQUEST_DOWNLOAD,
	TYPE_REQUEST_DIRECTORY,
	TYPE_UPLOAD_FILE,
	TYPE_ERROR,
	TYPE_OK,
	TYPE_CANCEL,
	TYPE_CREATE_FOLDER,
	TYPE_DELETE_FOLDER,
	TYPE_DELETE_FILE,
	TYPE_RENAME,
    TYPE_MOVE,
    TYPE_COPY,
	TYPE_SHARE_FILE,
	TYPE_UPLOAD_FOLDER,
    TYPE_DOWNLOAD_FOLDER,
} MessageType;

typedef struct Message{
	MessageType type;
	int requestId;
	int length;
    char payload[PAYLOAD_SIZE];
} Message;

typedef struct Client {
	int requestId;
	char username[30];
	int connSock;
	int uploadSuccess;
} Client;

/*
* clone Message mess from temp
* @param Message* mess, Message temp
* @return 1 if clone success
*/
int copyMess(Message* mess, Message temp);

/*
* print Message content
* @param Message mess
* @return 1 if print success
*/
int printMess(Message mess);

/*
* send message
* @param int socket, Message msg
* @return size of message if valid
*/
int sendMessage(int socket, Message msg);

/*
* recv message
* @param int socket, Message msg
* @return size of message if valid 
*/
int receiveMessage(int socket, Message *msg);


char** str_split(char* a_str, const char a_delim);

char* getHeaderOfPayload(char* payload);

void sendWithCode(Message mess,StatusCode code, int sockfd);

#endif
