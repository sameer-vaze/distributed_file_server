#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFF 200

int soc[4];
//struct hostent *serv[4];
struct sockaddr_in addr_server;
char buf[BUFF], buffer[BUFF];
FILE *fp;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

typedef struct ll 							//node for linked list to store DFS details
{
	char DFSname[5];
	int DFSport;
	struct ll *next;
}node;

node *head = NULL;							//global variable to point to first item in linked list
node *current = NULL;						//global variable to traverse through the linked list

// typedef struct userpassll 							//node for linked list to store DFS details
// {
// 	char username[10];
// 	int password[10];
// 	struct userpassll *next;
// }upnode;

// node *uphead = NULL;							//global variable to point to first item in linked list
// node *upcurrent = NULL;						//global variable to traverse through the linked list

void insertToLL(char name[5], char port[6])				//function that adds items to linked list
{
	int l;
	node *link = (node*) malloc(sizeof(node));
	for (l = 0; l < strlen(name); l++)
	{
		link->DFSname[l] = name[l];				//insert name
	}

	link->DFSport = atoi(port);				//insert port
	
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

void displayLL()
{
	node *start;
	start = head;
	while(start != NULL)
	{
		printf("Values in list are: %s\t%d\n", start->DFSname, start->DFSport);
		start = start->next;
	}
}

// void insertTouserpassLL(char name[10], char pass[10])				//function that adds items to linked list
// {
// 	int l;
// 	upnode *link = (upnode*) malloc(sizeof(upnode));
// 	for (l = 0; l < strlen(name); l++)
// 	{
// 		link->username[l] = name[l];				//insert name
// 	}

// 	for (l = 0; l < strlen(pass); l++)
// 	{
// 		link->password[l] = pass[l];				//insert name
// 	}
	
// 	if(uphead == NULL)							//if element added is first element
// 	{
// 		uphead = link;
// 		link->next = NULL;
// 		upcurrent = link;
// 	}
// 	else										//if element added is any subsequent element
// 	{
// 		upcurrent->next = link;
// 		link->next = NULL;
// 		upcurrent = current->next;
// 	}
// }

void extractDFS(char *DFS_string)		//get file formats(extensions) supported by the web server as per the ws.conf file
{
	char arr1[5];
	char arr2[6];
	char *DFStok = strtok(DFS_string, "\t");
	int i = 0;
	while (DFStok != NULL)											
	{
		if(i == 0)
		{
			DFStok = strtok (NULL, "\t");
			strcpy(arr1,DFStok);
			//printf("arr1 is %s\n", arr1);
		}
		else if(i==1)
		{
			DFStok = strtok (NULL, ":");

		}
		else if(i==2)
		{
			DFStok = strtok (NULL, "\n");
			strcpy(arr2,DFStok);
			//printf("arr2 is %s\n", arr2);
		}
		else
		{
			DFStok = NULL;	
		}
		i++;
	}
	insertToLL(arr1, arr2);								//function call to add extension and description to linked list
}

char *extractUsername(char *name_string)			//get file root address for file from ws.conf
{
	strtok(name_string, "\t");
	char *usertok = strtok (NULL, "\0");
	return usertok;
}

char *extractPassword(char *pass_string)			//get file root address for file from ws.conf
{
	strtok(pass_string, "\t");
	char *passtok = strtok (NULL, "\0");
	return passtok;
}

void connectToServer(int sel)
{
	node *start;
	start = head;
	int x = 0;
	if(sel == 5)
	{
		while(start != NULL)
		{
			if((soc[x] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				error("Error opening socket");
			}
			bzero((char *) &addr_server, sizeof(addr_server));
			addr_server.sin_family = AF_INET;
			//printf("port is: %d\n", start->DFSport);
			addr_server.sin_port = htons(start->DFSport);
			addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
			if (connect(soc[x], &addr_server, sizeof(addr_server)) < 0)
			{
				error("Error connecting");
			}
			start = start->next;
			x++;
		}
	}
	else
	{
		while(x != sel)
		{
			start = start->next;
			x++;
		}
		if((soc[x] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			error("Error opening socket");
		}
		bzero((char *) &addr_server, sizeof(addr_server));
		addr_server.sin_family = AF_INET;
		//printf("port is: %d\n", start->DFSport);
		addr_server.sin_port = htons(start->DFSport);
		addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
		if (connect(soc[x], &addr_server, sizeof(addr_server)) < 0)
		{
			error("Error connecting");
		}
	}
}

void sendToDFS(char *send_string, int size, int serv)
{
	if(serv == 5)
	{
		connectToServer(serv);
		for(int serv_no = 0; serv_no < 4; serv_no++)
		{
			write(soc[serv_no], send_string, size);
			close(soc[serv_no]);		
		}
	}
	else
	{
		connectToServer(serv);
		write(soc[serv], send_string, size);
		close(soc[serv]);		
	}
}

char * recvFromDFS(int serv)
{
	connectToServer(serv);
	bzero(buf,BUFF);
	recv(soc[serv], buf, BUFF, 0);
	close(soc[serv]);
	return buf;
}

char * concat_string(char * a, char * b)
{
	char * result = (char *)malloc(strlen(a)+strlen(b));
	sprintf(result,"%s%s",a,b);
	return result;
}

void main(int argc, char *argv[])
{
	char * client_conf;
	char * data;
	char u[10], p[10];
	char message[50];
	char buff[100];
	char user[10];
	char pass[10];
	char * rcvfilesize = (char *)malloc(10);
	char * getbuffer = (char *)malloc(BUFF);
	char command[20], tem[20], subdir[20];
	char comm[4], file[10], chunksize[10], chunk_no[10];
	int dist[4][4] = {{12,23,34,41},{41,12,23,34},{34,41,12,23},{23,34,41,12}};
	struct stat st;
	int file_size, chunk, index, indx;
	int pc1,pc2, bytes, c_size, offset, last_size;
	char * result_1;
	char * result_2;
	char * result_3;
	char * result_4;
	static char a1[10000];
	static char a2[10000];
	static char a3[10000];
	static char a4[10000];

	if(argc != 2)
	{
		printf("The number of command line arguments is incorrect\n");
		exit(1);
	}

	client_conf = argv[1];

	if ((fp = fopen(client_conf,"r")) == NULL)					//check if dfc.conf file exists
	{
		printf("Configuration file dfc.conf not found.\nExiting\n");
		exit(1);
	}
	else
	{
		while(!feof(fp))
		{
			fgets(buff, BUFF, fp);
			//printf("Buffer is %s\n", buff);
			if(strncmp(buff, "Server", 6) == 0)
			{
				extractDFS(buff);
			}
			else if(strncmp(buff, "Username:", 9) == 0)
			{
				data = extractUsername(buff);
				strcpy(user, data);
			}
			else if(strncmp(buff, "Password:", 9) == 0)
			{
				data = extractPassword(buff);
				strcpy(pass, data);
			}
		}
		fclose(fp);
		displayLL();
	}
	printf("Enter Username:\t");
	scanf("%s",u);
	printf("Enter Password:\t");
	scanf("%s",p);
	sendToDFS(u,strlen(u), 5);
	sendToDFS(p,strlen(p), 5);
	// checkValidUser(name);
	while(1)
	{
		if((strncmp(u, user, strlen(u)) == 0) && (strncmp(p, pass, strlen(p)) == 0))
		{
			while(1)
			{
				printf("Available commands are:\n\t1. list(subdirectory)\n\t2. put(subdirectory, filename)\n\t3. get(subdirectory, filename)\n\t4. exit\n\n\tFor no subdir enter none\n");
				scanf("%s", command);
				printf("command is %s\n", command);

				strcpy(tem, command);
				int token = 0;
				char *tok = strtok(tem, "()");						//tokenize string to extract command (get,put,ls or exit) and filename
				while (tok != NULL)
				{
					if(token == 0)
					{
						bzero(comm, strlen(comm));
						strcpy(comm, tok);
						tok = strtok (NULL, ",)");
						printf("token is %d and comm is %s\n", token, comm);
					}
					else if(token == 1)
					{
						bzero(subdir, strlen(subdir));
						strcpy(subdir, tok);
						printf("token is %d and subdir is %s\n", token, subdir);
						tok = strtok (NULL, "()");
					}
					else
					{
						bzero(file, strlen(file));
						strcpy(file, tok);
						printf("token is %d and file is %s\n", token, file);
						tok = NULL;
					}
					token++;
				}
				if(strncmp(command, "list", 4) == 0)
				{
					printf("this is list\n");
					sendToDFS(command, strlen(command), 5);
					//list stuff
					getbuffer = recvFromDFS(0);
					printf("%s\n", getbuffer);
					getbuffer = recvFromDFS(1);
					printf("%s\n", getbuffer);
					getbuffer = recvFromDFS(2);
					printf("%s\n", getbuffer);
					getbuffer = recvFromDFS(4);
					printf("%s\n", getbuffer);

				}
				else if(strncmp(command, "get", 3) == 0)
				{
					sendToDFS(command, strlen(command), 5);
					printf("this is get\n");
					int parts_rcvd[4] = {0,0,0,0};
					
							
					while(parts_rcvd[0] != 1 && parts_rcvd[1] != 1 && parts_rcvd[2] != 1 && parts_rcvd[3] != 1)
					{
						int m = 0;
						for(int x = 0;x < 2;x++)
						{
							rcvfilesize = recvFromDFS(m);
							sscanf(rcvfilesize, "%s %s", chunksize, chunk_no);
							printf("%s and %s\n", chunksize, chunk_no);
							int chunk_size = atoi(chunksize);
							int chunk_num = atoi(chunk_no);
							if(x == 0)
							{
								offset = chunk_size;
							}
							if(chunk_num == 4)
							{
								last_size = chunk_size;
							}
							int bytes_received = 0;

							if(chunk_num == 1)
							{
								result_1 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								//fseek(fp, 0, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										//fwrite(getbuffer, BUFF, 1, fp);
										//result_1 = concat_string(result_1, getbuffer);

										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_1[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										//result_1 = concat_string(result_1, getbuffer);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_1[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_1);
								printf("size is %d\n", strlen(result_1));
								// char a1[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a1[h] = result_1[h];
								}

							}
							else if(chunk_num == 2)
							{
								result_2 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								// fseek(fp, offset, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										//result_2 = concat_string(result_2, getbuffer);
										// fwrite(getbuffer, BUFF, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_2[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// result_2 = concat_string(result_2, getbuffer);
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_2[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_2);	
								printf("size is %d\n", strlen(result_2));

								// char a2[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a2[h] = result_2[h];
								}
							}
							else if(chunk_num == 3)
							{
								result_3 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								// fseek(fp, 2*offset, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										//result_3 = concat_string(result_3, getbuffer);
										// fwrite(getbuffer, BUFF, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_3[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// result_3 = concat_string(result_3, getbuffer);
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_3[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_3);
								printf("size is %d\n", strlen(result_3));
								// char a3[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a3[h] = result_3[h];
								}
							}
							else
							{
								result_4 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								// fseek(fp, 3*offset, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										//result_4 = concat_string(result_4, getbuffer);
										// fwrite(getbuffer, BUFF, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_4[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// result_4 = concat_string(result_4, getbuffer);
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_4[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_4);
								printf("size is %d\n", strlen(result_4));
								// char a4[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a4[h] = result_4[h];
								}
							}
							parts_rcvd[chunk_num] = 1;
						}

						m = 2;
						for(int x = 0;x < 2;x++)
						{
							// char * result_1 = (char *)malloc(1);
							// char * result_2 = (char *)malloc(1);
							// char * result_3 = (char *)malloc(1);
							// char * result_4 = (char *)malloc(1);
							rcvfilesize = recvFromDFS(m);
							sscanf(rcvfilesize, "%s %s", chunksize, chunk_no);
							printf("%s and %s\n", chunksize, chunk_no);
							int chunk_size = atoi(chunksize);
							int chunk_num = atoi(chunk_no);
							if(x == 0)
							{
								offset = chunk_size;
							}
							if(chunk_num == 4)
							{
								last_size = chunk_size;
							}
							int bytes_received = 0;
			
							if(chunk_num == 1)
							{
								result_1 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								//fseek(fp, 0, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										//fwrite(getbuffer, BUFF, 1, fp);
										// result_1 = concat_string(result_1, getbuffer);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_1[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										// result_1 = concat_string(result_1, getbuffer);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_1[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_1);
								printf("size is %d\n", strlen(result_1));
								// char a1[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a1[h] = result_1[h];
								}
							}
							else if(chunk_num == 2)
							{
								result_2 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								// fseek(fp, offset, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										// result_2 = concat_string(result_2, getbuffer);
										// fwrite(getbuffer, BUFF, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_2[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// result_2 = concat_string(result_2, getbuffer);
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_2[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_2);
								printf("size is %d\n", strlen(result_2));
								// char a2[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a2[h] = result_2[h];
								}
							}
							else if(chunk_num == 3)
							{
								result_3 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								// fseek(fp, 2*offset, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										// result_3 = concat_string(result_3, getbuffer);
										// fwrite(getbuffer, BUFF, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_3[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// result_3 = concat_string(result_3, getbuffer);
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_3);
								printf("size is %d\n", strlen(result_3));
								// char a3[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a3[h] = result_3[h];
								}
							}
							else
							{
								result_4 = (char *)malloc(chunk_size);
								printf("I AM HERE\n");
								// fseek(fp, 3*offset, SEEK_SET);
								printf("I AM HERE\n");
								while(bytes_received < chunk_size)
								{
									getbuffer = recvFromDFS(m);
									if(chunk_size - bytes_received > BUFF)
									{
										// result_4 = concat_string(result_4, getbuffer);
										// fwrite(getbuffer, BUFF, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + BUFF;index++)
										{
											result_4[index] = getbuffer[indx];
											indx++;
										}
									}
									else
									{
										// result_4 = concat_string(result_4, getbuffer);
										// fwrite(getbuffer, chunk_size - bytes_received, 1, fp);
										indx = 0;
										for(index = bytes_received;index < bytes_received + chunk_size - bytes_received;index++)
										{
											result_4[index] = getbuffer[indx];
											indx++;
										}
									}
									bytes_received = bytes_received + strlen(getbuffer);
								}
								printf("%s\n", result_4);
								printf("size is %d\n", strlen(result_4));
								// char a4[chunk_size];
								for(int h = 0;h < chunk_size;h++)
								{
									a4[h] = result_4[h];
								}
							}
							parts_rcvd[chunk_num] = 1;
						}
					}

					FILE *ff;
					if((ff = fopen("results", "wb")) != NULL)
					{
						// fwrite("hi", 2, 1, ff);
						fwrite(a1, offset, 1, ff);
						fwrite(a2, offset, 1, ff);
						fwrite(a3, offset, 1, ff);
						fwrite(a4, last_size, 1, ff);
						fclose(ff);
					}

					//get stuff
				}
				else if (strncmp(command, "put", 3) == 0)
				{
					sendToDFS(command, strlen(command), 5);
					printf("this is put\n");
					char md5_string[] = "md5sum";
					sprintf(md5_string, "md5sum %s >md5value.txt", file);
					printf("md5_string is %s\n", md5_string);
					system(md5_string);
					if((fp = fopen("md5value.txt", "rb")) == NULL)
					{
						printf("File %s does not exist in current location\n", file);
					}
					else
					{
						fgets(buff, BUFF, fp);
						char md5[32], md, md1[2];
						int choice;
						strncpy(md5, buff, 32);
						md = md5[31];
						printf("md is %c\n", md);
						// sprintf(md1, "%x", md);	
						// printf("md1 is %s\n", md1);
						if(md == '0' || md == '1' || md == '2' || md == '3' || md == '4' || md == '5' || md == '6' || md == '7' || md == '8' || md == '9')
						{
							choice = atoi(md);
						}
						else
						{
							sprintf(md1, "%x", md);
							choice = atoi(md1) - 51;
						}
						choice = choice%4;
						
						if((fp = fopen(file,"rb")) != NULL)
						{
							stat(file, &st);												//read file size
							file_size = st.st_size;
							fclose(fp);
						}
						chunk = (file_size/4) + 1;
						sprintf(message, "%d %d", file_size, chunk);
						printf("message is %s\n", message);
						sendToDFS(message,strlen(message), 5);
						if((fp = fopen(file, "rb")) != NULL)
						{
							for(int m = 0; m<4 ;m++)
							{
								pc1 = dist[choice][m]/10;
								sprintf(message, "%d", pc1);
								sendToDFS(message,strlen(message), m);
							
								if(pc1 == 1)
								{
									c_size = chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, 0, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer1[bytes];
											fread(new_buffer1, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer1));
											sendToDFS(new_buffer1,strlen(new_buffer1), m);
											bytes = bytes - strlen(new_buffer1);
										}								  
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
									}
								}
								else if(pc1 == 2)
								{
									c_size = chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, chunk, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer2[bytes];
											fread(new_buffer2, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer2));
											sendToDFS(new_buffer2,strlen(new_buffer2), m);
											bytes = bytes - strlen(new_buffer2);
										}
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
										// sendToDFS(buffer,strlen(buffer), m);
									}
								}
								else if(pc1 == 3)
								{
									c_size = chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, 2*chunk, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer3[bytes];
											fread(new_buffer3, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer3));
											sendToDFS(new_buffer3,strlen(new_buffer3), m);
											bytes = bytes - strlen(new_buffer3);
										}								  
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
										// sendToDFS(buffer,strlen(buffer), m);
									}
								}
								else
								{
									c_size = file_size - 3*chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, 3*chunk, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer4[bytes];
											fread(new_buffer4, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer4));
											sendToDFS(new_buffer4,strlen(new_buffer4), m);
											bytes = bytes - strlen(new_buffer4);
										}								  
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
										// sendToDFS(buffer,strlen(buffer), m);
									}
								}


								pc2 = dist[choice][m]%10;
								sprintf(message, "%d", pc2);
								sendToDFS(message,strlen(message), m);
							
								if(pc2 == 1)
								{
									c_size = chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, 0, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer1[bytes];
											fread(new_buffer1, bytes, 1, fp);
											////printf("%d\n", strlen(new_buffer1));
											sendToDFS(new_buffer1,strlen(new_buffer1), m);
											bytes = bytes - strlen(new_buffer1);
										}								  
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
									}
								}
								else if(pc2 == 2)
								{
									c_size = chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, chunk, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer2[bytes];
											fread(new_buffer2, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer2));
											sendToDFS(new_buffer2,strlen(new_buffer2), m);
											bytes = bytes - strlen(new_buffer2);
										}
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
										// sendToDFS(buffer,strlen(buffer), m);
									}
								}
								else if(pc2 == 3)
								{
									c_size = chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, 2*chunk, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer3[bytes];
											fread(new_buffer3, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer3));
											sendToDFS(new_buffer3,strlen(new_buffer3), m);
											bytes = bytes - strlen(new_buffer3);
										}								  
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
										// sendToDFS(buffer,strlen(buffer), m);
									}
								}
								else
								{
									c_size = file_size - 3*chunk;
									printf("c_size is %d\n", c_size);
									fseek(fp, 3*chunk, SEEK_SET);
									printf("Is here\n");
									bytes = c_size;
									while(bytes > 0)
									{
										printf("here\n");
										if(bytes > BUFF)
										{
											fread(buffer, BUFF, 1, fp);
											//printf("%d\n", strlen(buffer));
											sendToDFS(buffer,strlen(buffer), m);
											bytes = bytes - strlen(buffer);	
										}
										else //if(bytes < BUFF && bytes >= 0)
										{
											char new_buffer4[bytes];
											fread(new_buffer4, bytes, 1, fp);
											//printf("%d\n", strlen(new_buffer4));
											sendToDFS(new_buffer4,strlen(new_buffer4), m);
											bytes = bytes - strlen(new_buffer4);
										}								  
										printf("%d\n", bytes);
										// printf("%s\n", buffer);
										// sendToDFS(buffer,strlen(buffer), m);
									}
								}
							}
							fclose(fp);
						}
					}
				}
				else if (strncmp(command, "exit", 4) == 0)
				{
					sendToDFS(command, strlen(command), 5);
					printf("this is exit\n");
					exit(1);
				}
				else
				{
					printf("Command %s is not recognized\n", command);
				}
			}
		}
		else
		{
			printf("Invalid Username or Password\n");
		}
	}
}