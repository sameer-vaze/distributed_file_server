#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAXBUF 200

FILE *fp;
char buf[MAXBUF], buffer[MAXBUF];
int sock;
int dfs;
int len;
struct sockaddr_in clientaddr;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

typedef struct ll 							//node for linked list to store DFS details
{
	char User[10], Pass[10];
	struct ll *next;
}node;

node *head = NULL;							//global variable to point to first item in linked list
node *current = NULL;						//global variable to traverse through the linked list

void insertToLL(char user[10], char pass[10])				//function that adds items to linked list
{
	int l;
	node *link = (node*) malloc(sizeof(node));
	for (l = 0; l < strlen(user); l++)
	{
		link->User[l] = user[l];				//insert name
	}
	for (l = 0; l < strlen(pass); l++)
	{
		link->Pass[l] = pass[l];				//insert port
	}
	if(head == NULL)							//if element added is first element
	{
		head = link;
		link->next = NULL;
		current = link;
	}
	else										//if element added is any subsequent element
	{
		current->next = link;
		link->next = NULL;
		current = current->next;
	}
}

void extractUserPass(char *DFSstring)
{
	char arr1[10], arr2[10];
	char *DFStok = strtok(DFSstring, "\t");
	printf("DFStok is %s\n", DFStok);
	int i = 0;
	while (DFStok != NULL)											
	{
		if(i == 0)
		{
			strcpy(arr1,DFStok);
			printf("arr1 is %s and size is %d\n", arr1, strlen(arr1));
		}
		else if(i==1)
		{
			DFStok = strtok (NULL, "\t");
			strcpy(arr2,DFStok);
			printf("arr2 is %s and size is %d\n", arr2, strlen(arr2));		
		}
		else
		{
			DFStok = NULL;	
		}
		i++;
	}
	insertToLL(arr1, arr2);
}

void displayLL()
{
	node *start;
	start = head;
	while(start != NULL)
	{
		printf("Values in list are: %s\t%s\n", start->User, start->Pass);
		start = start->next;
	}
}

void serverStart(int port)
{
	struct sockaddr_in sin;
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;							//set family as the internet family (AF_INET)
	sin.sin_addr.s_addr = INADDR_ANY;					//set any address as the address for the socket
	sin.sin_port = htons(port);							//set the port number to the socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)	//open socket  and check if error occurs
	{
		printf("Error in creating socket \n");
		exit(1);
	}
	int optval = 1;
  	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));		//prevents the "already in use" issue
	printf("Socket created\n");
	if ((bind(sock, (struct sockaddr *)&sin, sizeof(sin)))	 < 0)		//bind socket and check if error occurs
	{
		printf("%d\n", bind(sock, (struct sockaddr *)&sin, sizeof(sin)));
		//	printf("Error in binding socket\n");
		perror("Bind error: ");
		exit(1);
	}
	printf("Socket bound\n");	
	if(listen(sock, 4) != 0)							//listen for incoming requests on the socket we have created
	{
		printf("Error in listen\n");
		exit(1);
	}
	printf("Listening for connections\n");
}

char * receive()
{
	dfs = accept(sock, (struct sockaddr *)&clientaddr, &len);
	bzero(buf, MAXBUF);
	recv(dfs, buf, MAXBUF, 0);
	// printf("buf is %s\n", buf);
	shutdown(dfs, 2);
	return buf;
}

void sendToDFC(char * send_string, int size)
{
	dfs = accept(sock, (struct sockaddr *)&clientaddr, &len);
	write(dfs, send_string, size);
	shutdown(dfs, 2);
}

void main(int argc, char *argv[])
{
	char * p, * p1, currentpass[10], tem[20];
	char comm[4], file[10], subdir[20], file_size1[10], chunk1[10], chunk_name[10], message[50];
	int p_no;
	char * command = (char *)malloc(20), * rcvd = (char *)malloc(MAXBUF), * currentuser = (char *)malloc(20);
	int x = 0;
	int file_size, chunk, pc1, pc2, bytes, c_size, openfile_size;
	struct stat st;
	
	if(argc != 3)
	{
		printf("The number of command line arguments is incorrect\n");
		exit(1);
	}

	p = argv[1];
	p_no = atoi(argv[2]);
	p1 = (char *)malloc(4);
	strncpy(p1, p+1, 4);
	chdir(p1);
	system("ls -a");

	if ((fp = fopen("dfs.conf","r")) == NULL)					//check if dfs.conf file exists
	{
		printf("Configuration file dfs.conf not found.\nExiting\n");
		exit(1);
	}
	else
	{
		while(!feof(fp))
		{
			fgets(buf, MAXBUF, fp);
			printf("Buffer is %s\n", buf);
			extractUserPass(buf);
		}
		displayLL();
	}

	serverStart(p_no);
	len = sizeof(clientaddr);
	currentuser = receive();
	printf("currentuser is %s\n", currentuser);
	int i = mkdir(currentuser, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	chdir(currentuser);
	system("ls -a");
	//printf("%d\n", i);
	
	while(1)
	{		
		command = receive();
		printf("command is %s \n", command);
		
		strcpy(tem, command);
		int token = 0;
		char *tok = strtok(tem, "()");						//tokenize string to extract command (get,put,ls or exit) and filename
		while (tok != NULL)
		{
			if(token == 0)
			{
				strcpy(comm, tok);
				tok = strtok (NULL, ",)");
				printf("token is %d and comm is %s\n", token, comm);
			}
			else if(token == 1)
			{
				strcpy(subdir, tok);
				printf("token is %d and subdir is %s\n", token, subdir);
				tok = strtok (NULL, "()");
			}
			else
			{
				strcpy(file, tok);
				printf("token is %d and file is %s\n", token, file);
				tok = NULL;
			}
			token++;
		}
		if(strncmp(command, "list", 4) == 0)
		{
			if(strncmp(subdir, "none",4) != 0)
			{
				x = chdir(subdir);
			}
			printf("x is %d\n", x);
			if(x == -1)
			{
				mkdir(subdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				chdir(subdir);
			}
			printf("this is list\n");
			system("ls -a > lsvalue.txt");

			fp = fopen("lsvalue.txt","rb+");
			fread(buffer, MAXBUF, 1, fp);
			sendToDFC(buffer,strlen(buffer));
			fclose(fp);

			if(strncmp(subdir, "none",4) != 0)
			{
				chdir("..");
			}
		}
		else if(strncmp(command, "get", 3) == 0)
		{
			if(strncmp(subdir, "none",4) != 0)
			{
				x = chdir(subdir);
			}
			printf("x is %d\n", x);
			if(x == -1)
			{
				mkdir(subdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				chdir(subdir);
			}
			
			printf("this is get\n");
			if(p_no == 10001 || p_no == 10003)
			{
				for(int l = 1;l <= 4;l++)
				{
					char * openfile = (char *)malloc(20);
					sprintf(openfile, ".%s.%d", file, l);
					if((fp = fopen(openfile, "rb")) != NULL)
					{
						stat(openfile, &st);												//read file size
						openfile_size = st.st_size;
						sprintf(message, "%d %d", openfile_size, l);
						sendToDFC(message, strlen(message));
						rewind(fp);
						int bytes_sent = 0;
						while(bytes_sent != openfile_size)
						{
							if(openfile_size - bytes_sent > MAXBUF)
							{
								fread(buffer, MAXBUF, 1, fp);
								sendToDFC(buffer,strlen(buffer));
								bytes_sent = bytes_sent + strlen(buffer);
							}
							else
							{
								fread(buffer, openfile_size - bytes_sent, 1, fp);
								sendToDFC(buffer,openfile_size - bytes_sent);
								bytes_sent = bytes_sent + openfile_size - bytes_sent;
							}
							printf("bytes_sent is %d\n", bytes_sent);
						}
						fclose(fp);
					}
					else
					{
						printf("piece %s not here\n", openfile);
					}
					free(openfile);
				}
			}
			if(strncmp(subdir, "none",4) != 0)
			{
				chdir("..");
			}
		}
		else if (strncmp(command, "put", 3) == 0)
		{
			if(strncmp(subdir, "none",4) != 0)
			{
				x = chdir(subdir);
			}
			printf("x is %d\n", x);
			if(x == -1)
			{
				mkdir(subdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				chdir(subdir);
			}
			printf("this is put\n");
			rcvd = receive();
			printf("rcvd is %s\n", rcvd);
			sscanf(rcvd, "%s %s", file_size1, chunk1);
			printf("%s and %s\n", file_size1, chunk1);
			file_size = atoi(file_size1);
			chunk = atoi(chunk1);
			
			rcvd = receive();
			pc1 = atoi(rcvd);
			bytes = 0;
			if(pc1 == 4)
			{
				c_size = file_size - 3*chunk;
				printf("c_size %d\n", c_size);
			}
			else
			{
				c_size = chunk;
				printf("c_size %d\n", c_size);
			}
			sprintf(chunk_name, ".%s.%d", file, pc1);
			printf("%s\n", chunk_name);
			if((fp = fopen(chunk_name, "wb+")) != NULL)
			{
				while(bytes < c_size)
				{
					rcvd = receive();
					if(c_size - bytes > MAXBUF)
					{
						fwrite(rcvd, MAXBUF, 1, fp);
						bytes = bytes + MAXBUF;
					}
					else
					{
						fwrite(rcvd, c_size - bytes, 1, fp);
						bytes = bytes + c_size - bytes;
					}
					printf("%d\n", strlen(rcvd));
					printf("bytes is %d\n", bytes);
				}
				fclose(fp);
			}


			rcvd = receive();
			pc2 = atoi(rcvd);
			bytes = 0;
			if(pc2 == 4)
			{
				c_size = file_size - 3*chunk;
				printf("c_size %d\n", c_size);
			}
			else
			{
				c_size = chunk;
				printf("c_size %d\n", c_size);
			}
			sprintf(chunk_name, ".%s.%d", file, pc2);
			printf("%s\n", chunk_name);
			if((fp = fopen(chunk_name, "wb+")) != NULL)
			{
				while(bytes < c_size)
				{
					rcvd = receive();
					if(c_size - bytes > MAXBUF)
					{
						fwrite(rcvd, MAXBUF, 1, fp);
						bytes = bytes + MAXBUF;
					}
					else
					{
						fwrite(rcvd, c_size - bytes, 1, fp);
						bytes = bytes + c_size - bytes;
					}
					printf("%d\n", strlen(rcvd));
					printf("bytes is %d\n", bytes);
				}
				fclose(fp);
			}

			if(strncmp(subdir, "none",4) != 0)
			{
				chdir("..");
			}
		}
		else if (strncmp(command, "exit", 4) == 0)
		{
			printf("this is exit\n");
			exit(1);
		}
	}
}