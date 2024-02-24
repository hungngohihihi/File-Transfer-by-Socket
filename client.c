#define _GNU_SOURCE
#include <stdio.h> /* These are the usual header files */
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/uio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <libgen.h>
#include "search.h"
#include "protocol.h"
#include "validate.h"
#include "status.h"
#include "stack.h"
#include "zipper.h"

char current_user[255];
char root[100];
int requestId;
int client_sock;
struct sockaddr_in server_addr; /* server's address information */
char choose;
Message *mess;
int isOnline = 0;
struct Stack *stack;
char *listCurrentDirec[1000];
char **listFolder;
char **listFile;
#define DIM(x) (sizeof(x) / sizeof(*(x)))

void openFolder(char *folder);

/*
 * count number param of command
 * @param temp
 * @return number param
 */
int numberElementsInArray(char **temp)
{
	int i;
	if (temp != NULL)
	{
		for (i = 0; *(temp + i); i++)
		{
			// count number elements in array
			// dont need to code in here
		}
		return i;
	}
	return 0;
}

int hasInList(char *element, char **temp)
{
	int i;
	if (temp != NULL)
	{
		for (i = 0; *(temp + i); i++)
		{
			if (strcmp(element, temp[i]) == 0)
				return 1;
		}
	}
	return 0;
}

void toNameOfFile(char *fileName, char *name)
{

	char **tokens;
	if (fileName[0] == '/')
	{
		char word2[strlen(fileName) - 1];
		strcpy(word2, &fileName[1]);
		tokens = str_split(word2, '/');
	}
	else
	{
		tokens = str_split(fileName, '/');
	}

	int i;
	for (i = 0; *(tokens + i); i++)
	{
		// count number elements in array
	}
	strcpy(name, *(tokens + i - 1));
	// printf("\nNAME: %s",name);
}

void removeFile(char *fileName)
{
	// remove file
	if (remove(fileName) != 0)
	{
		perror("Following error occurred\n");
	}
}
/*
 * get list directory from server
 * @return void
 */
void getDirectory()
{
	Message sendMsg, recvMsg1, recvMsg2;
	sendMsg.type = TYPE_REQUEST_DIRECTORY;
	sendMsg.requestId = requestId;
	sendMsg.length = 0;
	sendMessage(client_sock, sendMsg);

	// receive list folder path, list file path from server
	receiveMessage(client_sock, &recvMsg1);
	receiveMessage(client_sock, &recvMsg2);

	listFile = listFolder = NULL;

	if (recvMsg1.length > 0)
		listFolder = str_split(recvMsg1.payload, '\n');
	if (recvMsg2.length > 0)
		listFile = str_split(recvMsg2.payload, '\n');
}

/*
 * show list directory to client screen
 * @return void
 */
void showDirectory(char *root)
{
	printf("\n---------------- Your Directory ----------------\n");
	int i;
	int j = 0;
	printf("   %-15s%-30s%-6s\n", "Name", "Path", "Type");
	if (numberElementsInArray(listFolder) > 0)
	{
		for (i = 0; *(listFolder + i); i++)
		{
			/*The POSIX version of dirname and basename may modify the content of the argument.
			Hence, we need to strdup the local_file.*/
			char *temp = strdup(listFolder[i]);
			char *temp2 = strdup(listFolder[i]);
			if (strcmp(dirname(temp), root) == 0)
			{
				printf("%d. %-15s%-30sFolder\n", j + 1, basename(temp2), listFolder[i]);
				listCurrentDirec[j] = strdup(listFolder[i]);
				j++;
			}
			free(temp);
			free(temp2);
		}
	}
	if (numberElementsInArray(listFile) > 0)
	{
		for (i = 0; *(listFile + i); i++)
		{
			char *temp = strdup(listFile[i]);
			char *temp2 = strdup(listFile[i]);
			if (strcmp(dirname(temp), root) == 0)
			{
				printf("%d. %-15s%-30sFile\n", j + 1, basename(temp2), listFile[i]);
				listCurrentDirec[j] = strdup(listFile[i]);
				j++;
			}
			free(temp);
			free(temp2);
		}
	}
}

/*
 * handle upload file to server
 * @param message
 * @return void
 */
void uploadFile()
{
	char fileName[30];
	char fullPath[100];
	int i = 0;
	printf("\n------------------ Upload File ------------------\n");
	printf("Please choose folder you want to upload into it :\n");
	printf("1. %-15s./%-28sFolder\n", current_user, current_user);
	if (numberElementsInArray(listFolder) > 0)
	{
		for (i = 0; *(listFolder + i); i++)
		{
			char *temp = strdup(listFolder[i]);
			printf("%d. %-15s%-30sFolder\n", i + 2, basename(temp), listFolder[i]);
			free(temp);
		}
	}
	char choose[10];
	int option;
	while (1)
	{
		printf("\nChoose (Press 0 to cancel): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i + 1))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}
	if (option == 0)
		return;

	FILE *fptr;
	while (1)
	{
		printf("Please input the path of file you want to upload:");
		scanf("%[^\n]s", fullPath);
		while (getchar() != '\n')
			;

		if ((fptr = fopen(fullPath, "rb+")) == NULL)
		{
			printf("\nError: File not found\n");
		}
		else
			break;
	}

	Message msg, sendMsg, recvMsg;
	toNameOfFile(fullPath, fileName);
	// printf("FILE NAME: %s",fileName);
	if (option == 1)
	{
		snprintf(msg.payload, sizeof(msg.payload), "./%s/%s", current_user, fileName);
	}
	else
	{
		snprintf(msg.payload, sizeof(msg.payload), "%s/%s", listFolder[option - 2], fileName);
	}

	msg.type = TYPE_UPLOAD_FILE;
	msg.length = strlen(msg.payload);
	msg.requestId = requestId;
	sendMessage(client_sock, msg);
	receiveMessage(client_sock, &recvMsg);

	if (recvMsg.type == TYPE_ERROR)
	{
		printf("%s\n", recvMsg.payload);
		fclose(fptr);
	}
	else
	{
		long filelen;
		fseek(fptr, 0, SEEK_END); // Jump to the end of the file
		filelen = ftell(fptr);	  // Get the current byte offset in the file
		rewind(fptr);			  // pointer to start of file
		int sumByte = 0;
		while (!feof(fptr))
		{
			int numberByteSend = PAYLOAD_SIZE;
			if ((sumByte + PAYLOAD_SIZE) > filelen)
			{ // if over file size
				numberByteSend = filelen - sumByte;
			}
			char *buffer = (char *)malloc((numberByteSend) * sizeof(char));
			fread(buffer, numberByteSend, 1, fptr); // read buffer with size
			memcpy(sendMsg.payload, buffer, numberByteSend);
			sendMsg.type = TYPE_UPLOAD_FILE;
			sendMsg.requestId = requestId;
			sendMsg.length = numberByteSend;
			sumByte += numberByteSend; // increase byte send
			if (sendMessage(client_sock, sendMsg) <= 0)
			{
				printf("Connection closed!\n");
				break;
			}
			free(buffer);
			if (sumByte >= filelen)
			{
				break;
			}
		}
		sendMsg.length = 0;
		sendMessage(client_sock, sendMsg);
		receiveMessage(client_sock, &recvMsg);
		printf("\n%s", recvMsg.payload);
	}
}

void computeLPSArray(char *pat, int M, int *lps)
{
	int len = 0;
	int i = 1;
	lps[0] = 0;

	while (i < M)
	{
		if (pat[i] == pat[len])
		{
			len++;
			lps[i] = len;
			i++;
		}
		else
		{
			if (len != 0)
			{
				len = lps[len - 1];
			}
			else
			{
				lps[i] = 0;
				i++;
			}
		}
	}
}

int KMPSearch(char *pat, char *txt)
{
	int M = strlen(pat);
	int N = strlen(txt);

	int lps[M];
	computeLPSArray(pat, M, lps);

	int i = 0; // index for txt[]
	int j = 0; // index for pat[]

	while (i < N)
	{
		if (pat[j] == txt[i])
		{
			j++;
			i++;
		}

		if (j == M)
		{
			return i - j; // pattern found at index i - j
		}
		else if (i < N && pat[j] != txt[i])
		{
			if (j != 0)
			{
				j = lps[j - 1];
			}
			else
			{
				i++;
			}
		}
	}

	return -1; // pattern not found
}

void handleSearchFile(char *fileName, char *listResult)
{
	int i;
	if (numberElementsInArray(listFile) > 0)
	{
		for (i = 0; *(listFile + i); i++)
		{
			char *temp = strdup(listFile[i]);
			char *baseName = basename(temp);
			if (strstr(baseName, fileName) != NULL)
			{
				strcat(listResult, listFile[i]); // Store the full path
				strcat(listResult, "\n");
			}
			free(temp);
		}
	}
}

/*
 * show list directory to client screen
 * @return void
 */

/*
 * show list directory to client screen
 * @return void
 */

/*
 * receive file from server and save
 * @param filename, path
 * @return void
 */
int handleSelectDownloadFile(char *selectLink)
{
	char fileName[100];
	char listResult[1000];
	memset(listResult, '\0', sizeof(listResult));
	printf("\n------------------ Download File ------------------\n");
	printf("Please Input Download File Name: ");
	scanf("%[^\n]s", fileName);
	handleSearchFile(fileName, listResult);
	if (strlen(listResult) <= 0)
		return -1;

	char **tmp = str_split(listResult, '\n');
	int i;
	printf("   %-15s%-30s\n", "Name", "Path");
	for (i = 0; *(tmp + i); i++)
	{
		printf("%d. %-15s%-30s\n", i + 1, basename(tmp[i]), tmp[i]);
	}

	char choose[10];
	int option;
	while (1)
	{
		printf("\nPlease select to download (Press 0 to cancel): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}

	if (option == 0)
	{
		return -1;
	}
	else
	{
		strcpy(selectLink, tmp[option - 1]);
		return 1;
	}
}

int download(char *link)
{
	Message sendMsg, recvMsg;
	FILE *fptr;
	char saveFolder[20];
	char savePath[50];
	char temp[50];

	strcpy(temp, link);
	sendMsg.type = TYPE_REQUEST_DOWNLOAD;
	sendMsg.requestId = requestId;
	strcpy(sendMsg.payload, link);
	sendMsg.length = strlen(sendMsg.payload);

	sendMessage(client_sock, sendMsg);
	receiveMessage(client_sock, &recvMsg);

	if (recvMsg.type != TYPE_ERROR)
	{
		printf("Please Input Saved Path in Local: ");
		scanf("%[^\n]s", saveFolder);
		sprintf(savePath, "%s/%s", saveFolder, basename(temp));
		if (fopen(savePath, "r+") != NULL)
		{
			char choose;
			printf("Warning: File name already exists!!! Do you want to replace? Y/N\n");
			while (1)
			{
				scanf(" %c", &choose);
				while (getchar() != '\n')
					;
				if ((choose == 'Y') || choose == 'y' || choose == 'N' || choose == 'n')
				{
					break;
				}
				else
				{
					printf("Please press Y or N\n");
				}
			}
			if (choose == 'N' || choose == 'n')
			{
				sendMsg.type = TYPE_CANCEL;
				sendMessage(client_sock, sendMsg);
				return -1;
			}
		}
		sendMsg.type = TYPE_OK;
		sendMessage(client_sock, sendMsg);
		printf("----------------------Downloading-----------------------\n");
		fptr = fopen(savePath, "w+");
		while (1)
		{
			receiveMessage(client_sock, &recvMsg);
			if (recvMsg.type == TYPE_ERROR)
			{
				fclose(fptr);
				removeFile(savePath);
				return -1;
			}
			if (recvMsg.length > 0)
			{
				fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
			}
			else
			{
				break;
			}
		}
		fclose(fptr);
		return 1;
	}
	else
	{
		char temp[PAYLOAD_SIZE];
		strcpy(temp, recvMsg.payload);
		printf("\n%s", temp);
		return -1;
	}
}
/*
 * method download
 * @param filename, path
 * @return void
 */
void downloadFile()
{
	char selectLink[50];
	if (handleSelectDownloadFile(selectLink) == 1)
	{
		printf("...............................................\n");
		if (download(selectLink) == -1)
		{
			printf("Having trouble, stop downloading the file!!\n");
			return;
		}
		printf("Download Successful!!!");
	}
	else
	{
		printf("No results were found\n");
		return;
	}
}

void open(char *pre_folder, char *cur_folder)
{
	push(stack, pre_folder);
	openFolder(cur_folder);
}

void createNewFolder()
{
	int i = 0;
	printf("\n------------------ Create Folder ------------------\n");
	printf("Please choose folder you want to create into it :\n");
	printf("1. %-15s./%-28sFolder\n", current_user, current_user);
	if (numberElementsInArray(listFolder) > 0)
	{
		for (i = 0; *(listFolder + i); i++)
		{
			char *temp = strdup(listFolder[i]);
			printf("%d. %-15s%-30sFolder\n", i + 2, basename(temp), listFolder[i]);
			free(temp);
		}
	}
	char choose[10];
	int option;
	while (1)
	{
		printf("\nChoose (Press 0 to cancel): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i + 1))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}
	if (option == 0)
		return;
	char newFolder[20];
	Message sendMsg;
	printf("Input new folder name: ");
	scanf("%[^\n]s", newFolder);
	if (option == 1)
	{
		strcpy(sendMsg.payload, root);
	}
	else
	{
		strcpy(sendMsg.payload, listFolder[option - 2]);
	}
	strcat(sendMsg.payload, "/");
	strcat(sendMsg.payload, newFolder);
	sendMsg.length = strlen(sendMsg.payload);
	sendMsg.requestId = requestId;
	sendMsg.type = TYPE_CREATE_FOLDER;
	sendMessage(client_sock, sendMsg);
}

void deleteFile(char *cur_file)
{
	Message sendMsg;
	strcpy(sendMsg.payload, cur_file);
	sendMsg.length = strlen(sendMsg.payload);
	sendMsg.requestId = requestId;
	sendMsg.type = TYPE_DELETE_FILE;
	sendMessage(client_sock, sendMsg);
}

void renameFile(char *current_folder, char *file_name)
{
	char new_name[255];

	printf("Enter the new name: ");
	scanf(" %[^\n]s", new_name);
	while (getchar() != '\n')
		;

	char old_path[255];
	sprintf(old_path, "%s/%s", current_folder, file_name);

	char new_path[600];
	sprintf(new_path, "%s/%s", current_folder, new_name);

	Message sendMsg;
	sendMsg.type = TYPE_RENAME;
	sendMsg.requestId = requestId;
	snprintf(sendMsg.payload, sizeof(sendMsg.payload), "OLD %s\nNEW %s", old_path, new_path);
	sendMsg.length = strlen(sendMsg.payload);
	sendMessage(client_sock, sendMsg);
}

void moveFile(char *current_folder, char *file_name)
{
	char new_folder[255];

	printf("Enter the folder you want to move: ");
	scanf(" %[^\n]s", new_folder);
	while (getchar() != '\n')
		;

	char old_path[255];
	sprintf(old_path, "%s/%s", current_folder, file_name);

	char new_path[600];
	sprintf(new_path, "%s/%s", new_folder, file_name);

	Message sendMsg;
	sendMsg.type = TYPE_MOVE;
	sendMsg.requestId = requestId;
	snprintf(sendMsg.payload, sizeof(sendMsg.payload), "OLD %s\nNEW %s", old_path, new_path);
	sendMsg.length = strlen(sendMsg.payload);
	sendMessage(client_sock, sendMsg);
}

void copyFile(char *current_folder, char *file_name)
{
	char new_folder[255];

	printf("Enter the folder you want to copy to: ");
	scanf(" %[^\n]s", new_folder);
	while (getchar() != '\n')
		;

	char old_path[255];
	sprintf(old_path, "%s/%s", current_folder, file_name);

	char new_path[600];
	sprintf(new_path, "%s/%s", new_folder, file_name);

	Message sendMsg;
	sendMsg.type = TYPE_COPY;
	sendMsg.requestId = requestId;
	snprintf(sendMsg.payload, sizeof(sendMsg.payload), "OLD %s\nNEW %s", old_path, new_path);
	sendMsg.length = strlen(sendMsg.payload);
	sendMessage(client_sock, sendMsg);
	printf("Copy successful.\n");
}

void shareFile(char *current_folder, char *file_name)
{
	char new_folder[255] = "./";
	char username[255];

	printf("Enter the user name you want to share file to: ");
	scanf(" %[^\n]s", username);

	// Gán giá trị của biến username vào đằng sau giá trị của biến new_folder
	strcat(new_folder, username);
	strcat(new_folder, "/");

	// In ra giá trị của biến new_folder để kiểm tra

	while (getchar() != '\n')
		;

	char old_path[255];
	sprintf(old_path, "%s/%s", current_folder, file_name);

	char new_path[600];
	sprintf(new_path, "%s/%s", new_folder, file_name);

	Message sendMsg;
	sendMsg.type = TYPE_COPY;
	sendMsg.requestId = requestId;
	snprintf(sendMsg.payload, sizeof(sendMsg.payload), "OLD %s\nNEW %s", old_path, new_path);
	sendMsg.length = strlen(sendMsg.payload);
	sendMessage(client_sock, sendMsg);
	printf("Share file successful.\n");
}

void renameFolder(char *current_folder, char *new_name)
{
	char new_path[255];

	// Remove the trailing slash from current_folder if it exists
	if (current_folder[strlen(current_folder) - 1] == '/')
	{
		current_folder[strlen(current_folder) - 1] = '\0';
	}

	// Find the last occurrence of the old folder name in current_folder
	char *old_folder_position = strrchr(current_folder, '/');

	// If the old folder name is found, replace it with the new name
	if (old_folder_position != NULL)
	{
		int index = old_folder_position - current_folder;
		strncpy(new_path, current_folder, index + 1); // Copy the part before the old folder name
		new_path[index + 1] = '\0';					  // Null-terminate the string
		strcat(new_path, new_name);					  // Concatenate the new folder name
	}
	else
	{
		// If the old folder name is not found, simply concatenate the new name to the current folder
		sprintf(new_path, "%s%s", current_folder, new_name);
	}

	Message sendMsg;
	sendMsg.type = TYPE_RENAME;
	sendMsg.requestId = requestId;
	snprintf(sendMsg.payload, sizeof(sendMsg.payload), "OLD %s\nNEW %s", current_folder, new_path);
	sendMsg.length = strlen(sendMsg.payload);
	sendMessage(client_sock, sendMsg);
}

void menuFileProcess()
{
	printf("\n------------------File Process------------------\n");
	printf("\n1 - Delete File");
	printf("\n2 - Download File");
	printf("\n3 - Rename File"); // Thêm lựa chọn để đổi tên file
	printf("\n4 - Move File");
	printf("\n5 - Copy File");
	printf("\n6 - Share File");
	printf("\n7 - Cancel");
	printf("\nChoose: ");
}

void fileProcess(char *cur_file)
{
	printf("%s", cur_file);
	menuFileProcess();
	scanf(" %c", &choose);
	char current_folder[256];
	char file_name[256];

	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		deleteFile(cur_file);
		getDirectory();
		break;
	case '2':
		if (download(cur_file) == -1)
		{
			printf("Having trouble, stop downloading the file!!\n");
			return;
		}
		printf("Download Successful!!!");
		break;
	case '3':
		// Using sscanf to extract the substrings
		sscanf(cur_file, "./%[^/]/%s", current_folder, file_name);
		renameFile(current_folder, file_name);
		getDirectory();
		break;
	case '4':

		// Using sscanf to extract the substrings
		sscanf(cur_file, "./%[^/]/%s", current_folder, file_name);
		moveFile(current_folder, file_name);
		getDirectory();
		break;
	case '5':
		// Using sscanf to extract the substrings
		sscanf(cur_file, "./%[^/]/%s", current_folder, file_name);
		copyFile(current_folder, file_name);
		getDirectory();
		break;

	case '6':
		// Using sscanf to extract the substrings
		sscanf(cur_file, "./%[^/]/%s", current_folder, file_name);
		shareFile(current_folder, file_name);
		getDirectory();
		break;
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}


void uploadFolder() {
    char fullPath[100];
    int i = 0;
    printf("\n------------------ Upload Folder ------------------\n");
    printf("Please choose folder you want to upload into it :\n");
    printf("1. %-15s./%-28sFolder\n", current_user, current_user);
    if (numberElementsInArray(listFolder) > 0) {
        for (i = 0; *(listFolder + i); i++) {
            char* temp = strdup(listFolder[i]);
            printf("%d. %-15s%-30sFolder\n", i + 2, basename(temp), listFolder[i]);
            free(temp);
        }
    }
    char choose[10];
    int option;
    while (1) {
        printf("\nChoose (Press 0 to cancel): ");
        scanf(" %s", choose);
        while (getchar() != '\n');
        option = atoi(choose);
        if ((option >= 0) && (option <= i + 1)) {
            break;
        } else {
            printf("Please Select Valid Options!!\n");
        }
    }
    if (option == 0) return;

    while (1) {
        printf("Please input the path of file you want to upload:");
        scanf("%[^\n]s", fullPath);
        while (getchar() != '\n');

        if (is_folder_empty(fullPath) == 1) {
            printf("\nError: Folder not found or empty\n");
        } else break;
    }

    char zipFile[256];
    snprintf(zipFile, sizeof(zipFile), "%s%s", fullPath, MY_ZIP_EXTENSION);

    create_zip(zipFile, fullPath);

    char zipName[256];
    strcpy(zipName, zipFile);

    Message msg, sendMsg, recvMsg;
    printf("FILE NAME: %s, %s\n", zipFile, basename(zipName));
    if (option == 1) {
        snprintf(msg.payload, sizeof(msg.payload), "./%s/%s", current_user, basename(zipName));
    } else {
        snprintf(msg.payload, sizeof(msg.payload), "%s/%s", listFolder[option - 2], basename(zipName));
    }

    FILE* fptr;
    if ((fptr = fopen(zipFile, "rb+")) == NULL) {
        printf("\nAn error occurred!\n");
        return;
    }

    printf("Sending %s\n", msg.payload);

    msg.type = TYPE_UPLOAD_FOLDER;
    msg.length = strlen(msg.payload);
    msg.requestId = requestId;
    sendMessage(client_sock, msg);
    receiveMessage(client_sock, &recvMsg);

    if (recvMsg.type == TYPE_ERROR) {
        printf("%s\n", recvMsg.payload);
    } else {
        long filelen;
        fseek(fptr, 0, SEEK_END);
        filelen = ftell(fptr);
        rewind(fptr);
        int sumByte = 0;
        while (!feof(fptr)) {
            int numberByteSend = PAYLOAD_SIZE;
            if ((sumByte + PAYLOAD_SIZE) > filelen) {
                numberByteSend = filelen - sumByte;
            }
            char* buffer = (char*) malloc((numberByteSend) * sizeof(char));
            fread(buffer, numberByteSend, 1, fptr);
            memcpy(sendMsg.payload, buffer, numberByteSend);
            sendMsg.type = TYPE_UPLOAD_FILE;
            sendMsg.requestId = requestId;
            sendMsg.length = numberByteSend;
            sumByte += numberByteSend;
            if (sendMessage(client_sock, sendMsg) <= 0) {
                printf("Connection closed!\n");
                break;
            }
            free(buffer);
            if (sumByte >= filelen) {
                break;
            }
        }
        sendMsg.length = 0;
        sendMessage(client_sock, sendMsg);
        receiveMessage(client_sock, &recvMsg);
        printf("\n%s", recvMsg.payload);
    }

    fclose(fptr);

    remove(zipFile);
    printf("\nUpload folder %s successfully\n", fullPath);
}

void downloadFolder(char* link) {
    printf("Download folder %s\n", link);
    Message sendMsg, recvMsg;
    FILE* fptr;
    char saveFolder[100];
    char savePath[100];
    char saveFilePath[150];
    char temp[100];

    saveFolder[0] = 0;
    savePath[0] = 0;
    saveFilePath[0] = 0;

    strcpy(temp, link);
    sendMsg.type = TYPE_DOWNLOAD_FOLDER;
    sendMsg.requestId = requestId;
    strcpy(sendMsg.payload, link);
    sendMsg.length = strlen(sendMsg.payload);

    sendMessage(client_sock, sendMsg);
    receiveMessage(client_sock, &recvMsg);

    if (recvMsg.type != TYPE_ERROR) {
        printf("Please Input Saved Path in Local: ");
        scanf("%[^\n]s", saveFolder);
        sprintf(savePath, "%s/%s", saveFolder, basename(temp));
        sprintf(saveFilePath, "%s%s", savePath, MY_ZIP_EXTENSION);
        printf("%s\n%s\n", savePath, saveFilePath);

        if (fopen(saveFilePath, "r+") != NULL) {
            if (remove(saveFilePath) != 0) {
                sendMsg.type = TYPE_CANCEL;
                sendMessage(client_sock, sendMsg);
                printf("An error occurred\n");
                return;
            }
        }

        sendMsg.type = TYPE_OK;
        sendMessage(client_sock, sendMsg);
        printf("----------------------Downloading-----------------------\n");
        fptr = fopen(saveFilePath, "w+");
        while (1) {
            receiveMessage(client_sock, &recvMsg);
            if (recvMsg.type == TYPE_ERROR) {
                fclose(fptr);
                removeFile(saveFilePath);
                return;
            }
            if (recvMsg.length > 0) {
                fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
            } else {
                break;
            }
        }
        fclose(fptr);

        if (extract_zip(saveFilePath, savePath) == 0) {
            printf("An error occurred\n");
        } else {
            printf("Download successful\n");
        }
        remove(saveFilePath);
    } else {
        char tmp[PAYLOAD_SIZE];
        strcpy(tmp, recvMsg.payload);
        printf("\n%s\n", tmp);
    }
}


void deleteFolder(char *cur_folder)
{
	Message sendMsg;
	strcpy(sendMsg.payload, cur_folder);
	sendMsg.length = strlen(sendMsg.payload);
	sendMsg.requestId = requestId;
	sendMsg.type = TYPE_DELETE_FOLDER;
	sendMessage(client_sock, sendMsg);
}

void menuFolderProcess()
{
	printf("\n------------------Folder Process------------------\n");
	printf("\n1 - Open");
	printf("\n2 - Download Folder");
	printf("\n3 - Delete Folder");
	printf("\n4 - Rename");
	printf("\n5 - Cancel");
	printf("\nChoose: ");
}

void folderProcess(char *pre_folder, char *cur_folder)
{
	menuFolderProcess();
	char new_name[255];
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		open(pre_folder, cur_folder);
	case '2':
		downloadFolder(cur_folder);
		break;
	case '3':
		deleteFolder(cur_folder);
		getDirectory();
		// openFolder(cur_folder);
		break;
	case '4':
		printf("Enter the new name: ");
		scanf(" %[^\n]s", new_name);
		while (getchar() != '\n')
			;
		renameFolder(cur_folder, new_name);
		getDirectory();
		break;
	case '5':
		break;
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}
/*
 * open subfolder
 * @param
 * @return void
 */
void openFolder(char *folder)
{
	showDirectory(folder);
	// for(int u=0;u<=stack->top;u++)
	// printf("++++++Top Stack++++++: %s\n",stack->array[u]);
	printf("\n------------------------------------------------\n");
	char choose[10];
	int option;
	int i = numberElementsInArray(listCurrentDirec);
	while (1)
	{
		printf("\nSelect to File/Folder (1,2...n)(Press 0 to Back): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}
	if (option == 0)
	{
		if (!isEmpty(stack))
			openFolder(pop(stack));
		else
			return;
	}
	else
	{
		if (hasInList(listCurrentDirec[option - 1], listFolder))
		{
			folderProcess(folder, listCurrentDirec[option - 1]);
		}
		else
		{
			fileProcess(listCurrentDirec[option - 1]);
		}
	}
}

/*
 * print waiting message to screen when waiting download
 * @param
 * @return void
 */
void printWaitingMsg()
{
	printf("\n..................Please waiting................\n");
}

// get username and password from keyboard to login
void getLoginInfo(char *str)
{
	char username[255];
	char password[255];
	printf("Enter username: ");
	scanf("%[^\n]s", username);
	while (getchar() != '\n')
		;
	printf("Enter password: ");
	scanf("%[^\n]s", password);
	while (getchar() != '\n')
		;
	sprintf(mess->payload, "LOGIN\nUSER %s\nPASS %s", username, password);
	strcpy(str, username);
}

/*
 * get login info and login
 * @param current user
 * @return void
 */
void loginFunc(char *current_user)
{
	char username[255];
	mess->type = TYPE_AUTHENTICATE;
	getLoginInfo(username);
	mess->length = strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock, mess);
	if (mess->type != TYPE_ERROR)
	{
		isOnline = 1;
		strcpy(current_user, username);
		strcpy(root, "./");
		strcat(root, username);
		requestId = mess->requestId;
		getDirectory();
		stack = createStack(30);
	}
	else
	{
		char temp[PAYLOAD_SIZE];
		strcpy(temp, mess->payload);
		printf("\n%s", temp);
		return;
	}
	// printf("%s\n", mess->payload);
	printf("LOGIN SUCCESSFUL!!!\n");
}

/*
* get register info
* @param user
* @return void

*/
int getRegisterInfo(char *user)
{
	char username[255], password[255], confirmPass[255];
	printf("Username: ");
	scanf("%[^\n]s", username);
	printf("Password: ");
	while (getchar() != '\n')
		;
	scanf("%[^\n]s", password);
	printf("Confirm password: ");
	while (getchar() != '\n')
		;
	scanf("%[^\n]s", confirmPass);
	while (getchar() != '\n')
		;
	if (!strcmp(password, confirmPass))
	{
		sprintf(mess->payload, "REGISTER\nUSER %s\nPASS %s", username, password);
		strcpy(user, username);
		return 1;
	}
	else
	{
		printf("Confirm password invalid!\n");
		return 0;
	}
}

/*
 * get register info and login
 * @param current user
 * @return void
 */
void registerFunc(char *current_user)
{
	char username[255];
	if (getRegisterInfo(username))
	{
		mess->type = TYPE_AUTHENTICATE;
		mess->length = strlen(mess->payload);
		sendMessage(client_sock, *mess);
		receiveMessage(client_sock, mess);
		if (mess->type != TYPE_ERROR)
		{
			isOnline = 1;
			strcpy(current_user, username);
			strcpy(root, "./");
			strcat(root, username);
			requestId = mess->requestId;
			getDirectory();
			stack = createStack(30);
		}
		else
		{
			char temp[PAYLOAD_SIZE];
			strcpy(temp, mess->payload);
			printf("\n%s", temp);
			return;
		}
		// printf("%s\n", mess->payload);
		printf("REGISTER SUCCESSFUL!!!\n");
	}
}

/*
 * logout from system
 * @param current user
 * @return void
 */
void logoutFunc(char *current_user)
{
	mess->type = TYPE_AUTHENTICATE;
	sprintf(mess->payload, "LOGOUT\n%s", current_user);
	mess->length = strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock, mess);
	if (mess->type != TYPE_ERROR)
	{
		isOnline = 0;
		current_user[0] = '\0';
		requestId--;
	}
	else
	{
		char temp[PAYLOAD_SIZE];
		strcpy(temp, mess->payload);
		printf("\n%s", temp);
		return;
	}
	// printf("%s\n", mess->payload);
	printf("LOGGED OUT SUCCESSFULLY!\n");
}

/*
 * get show first menu of application
 * @param
 * @return void
 */
void menuAuthenticate()
{
	printf("\n------------------Storage System------------------\n");
	printf("\n1 - Login");
	printf("\n2 - Register");
	printf("\n3 - Exit");
	printf("\nChoose: ");
}

/*
 * get show main menu of application
 * @param
 * @return void
 */
void mainMenu()
{
	printf("\n------------------Menu------------------\n");
	printf("\n1 - Upload file");
	printf("\n2 - Upload folder");
	printf("\n3 - Download File");
	printf("\n4 - Open folder");
	printf("\n5 - Create folder");
	printf("\n6 - Logout");
	printf("\nPlease choose: ");
}

void authenticateFunc()
{
	menuAuthenticate();
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		loginFunc(current_user);
		break;
	case '2':
		registerFunc(current_user);
		break;
	case '3':
		exit(0);
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}

void requestFileFunc()
{
	mainMenu();
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		uploadFile();
		getDirectory();
		break;
	case '2':
		uploadFolder();
		getDirectory();
		break;
	case '3':
		downloadFile();
		break;
	case '4':
		getDirectory();
		openFolder(root);
		break;
	case '5':
		createNewFolder();
		getDirectory();
		break;
	case '6':
		logoutFunc(current_user);
		break;
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}

int initSock()
{
	int newsock = socket(AF_INET, SOCK_STREAM, 0);
	if (newsock == -1)
	{
		perror("\nError: ");
		exit(0);
	}
	return newsock;
}

void bindClient(int port, char *serverAddr)
{
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(serverAddr);
}

void connectToServer()
{
	if (connect(client_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately!\n");
		exit(0);
	}
}

void communicateWithUser()
{
	while (1)
	{
		if (!isOnline)
		{
			authenticateFunc();
		}
		else
		{
			requestFileFunc();
		}
	}
}

int main(int argc, char const *argv[])
{
	// check valid of IP and port number
	if (argc != 3)
	{
		printf("Error!\nPlease enter two parameter as IPAddress and port number!\n");
		exit(0);
	}

	char *serAddr = malloc(sizeof(argv[1]) * strlen(argv[1]));
	strcpy(serAddr, argv[1]);
	int port = atoi(argv[2]);
	mess = (Message *)malloc(sizeof(Message));
	mess->requestId = 0;

	if (!validPortNumber(port))
	{
		perror("Invalid Port Number!\n");
		exit(0);
	}

	if (!checkIP(serAddr))
	{
		printf("Invalid Ip Address!\n"); // Check valid Ip Address
		exit(0);
	}

	strcpy(serAddr, argv[1]);
	/*if(!hasIPAddress(serAddr)) {
		printf("Not found information Of IP Address [%s]\n", serAddr); // Find Ip Address
		exit(0);
	}*/

	// Step 1: Construct socket
	client_sock = initSock();
	// Step 2: Specify server address

	bindClient(port, serAddr);

	// Step 3: Request to connect server
	connectToServer();

	// Step 4: Communicate with server
	communicateWithUser();

	close(client_sock);
	return 0;
}

/*

TODO :check inet_adrr and htonl(INADDR_ANY);
*/