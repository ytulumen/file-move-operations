/*************************************
 * System Programming 244 FINAL
 * Yasin_Tulumen_121044020
 * Server.c
*************************************/ 



#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h> /* String fonksiyonlari kullanmak icin */
#include <sys/stat.h>
#include <errno.h> /* Hata bastirmak icin */
#include <unistd.h>
#include <signal.h>
#include "restart.h"

#define MAX_CHAR 1000
#define MAX_PR 200
#define XFILES "communicate"
#define TEMPFILE "temp"
#define BLKSIZE PIPE_BUF
#define MILLION 1000000L
#define D_MILLION 1000000.0
#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_EXCL)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)

static volatile int keepRunning = 1 ;
int globalSocket;

void error(char *msg);
void listServer(char *path, int newsockfd);
void intHandler(int dummy);
int main(int argc, char *argv[])
{
	FILE *filedes = fopen(XFILES, "w");
	fclose(filedes);
    int sockfd, newsockfd, portno, clilen, lsClPipe[2], sendPipe[2];
    char buffer[256], path[PATH_MAX+1];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    struct timeval startTime;
    pid_t childid;
    char lsClpid[MAX_CHAR], sendClient[MAX_CHAR];
    char clientPath[PATH_MAX];
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [directoryName]\n", argv[0]);
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    	error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    getcwd(path, sizeof(path));

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd,5);
	while(1){ 
	    clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		gettimeofday(&startTime, NULL);
		n = r_read(newsockfd, clientPath, PATH_MAX);
		if (n < 0) 
			error("ERROR reading from socket");
		if(fork()==0){
			close(sockfd);
			signal(SIGINT, intHandler);

			globalSocket = newsockfd;
		 	if (newsockfd < 0) 
		   		error("ERROR on accept");
			fprintf(stderr, "%d client connected %ld\n", getpid(), (startTime.tv_sec) * 1000 + (startTime.tv_usec) / 1000);
			bzero(lsClpid, MAX_CHAR);
	    	snprintf(lsClpid, MAX_CHAR, "%d", getpid());
			filedes = fopen(XFILES, "a");
			fprintf(filedes, "%s %s\n", lsClpid, clientPath);
			fclose(filedes);
			while(1){
				fprintf(stderr, "waiting request from client %ld\n", (long)getpid() );
	 			bzero(buffer,256);
	 			n = r_read(newsockfd,buffer,255);
	 			if (n < 0) 
	 				error("ERROR reading from socket");
	 			fprintf(stderr, "Here is the message from %s: %s\n", lsClpid, buffer);
				gettimeofday(&startTime, NULL);
				fprintf(stderr, "time is: %ld\n", (startTime.tv_sec) * 1000 + (startTime.tv_usec) / 1000);
	 			if(strncmp(buffer, "listServer", 10)==0){
	 				listServer(path, newsockfd);
					if( write(newsockfd, "end", 3) < 0)
						error("ERROR writing from socket");
	 			}
	 			else if(strncmp(buffer, "lsClients", 9)==0){
					bzero(lsClpid, MAX_CHAR);
					filedes = fopen(XFILES, "r");
					while (fscanf(filedes, "%s %s", buffer, clientPath) != EOF){
						strcat(sendClient, buffer);
						strcat(sendClient, "\n");
					}
		            n = write(newsockfd, sendClient, strlen(sendClient)+2);
			        if (n < 0) 
        				error("ERROR writing to socket");
	 				fprintf(stderr, "DONE\n" );
	 				fclose(filedes);
	 			}
	 			else if(strncmp(buffer, "sendFile", 7)==0){
	 				int i=0, boolean=0, pidInt, j=0;
	 				char fileName[50], clientID[5], sendPath[PATH_MAX+1], fileCounter='1';
	 				int fromfd, tofd;
	 				FILE* tofides;
	 				char fileID[5];

	 				while(buffer[i++] != 32);

	 				while(buffer[i] != 32){
	 					fileName[j] = buffer[i];
	 					++i;
	 					++j;
	 				}
	 				++i;
	 				j=0;
	 				filedes = fopen(XFILES, "r");
	 				while (fscanf(filedes, "%s %s", fileID, clientPath) != EOF){
	 					pidInt = atoi(fileID);
	 					if(pidInt == getpid()){
	 						strcat(clientPath, "/");
	 						strcat(clientPath, fileName);
 						    if ((fromfd = open(clientPath, READ_FLAGS)) == -1){
 						
					        	error("Failed to open input file");
						   }
		 				}
	 				}
	 				fclose(filedes);

	 				while(buffer[i] != '\0'){
	 					clientID[j] = buffer[i]; 
	 					++i;
	 					++j;
	 				}

	 				filedes = fopen(XFILES, "r");
	 				while (fscanf(filedes, "%s %s", fileID, clientPath) != EOF){
	 					pidInt = atoi(fileID);
	 					if(pidInt == atoi(clientID)){
	 						strcat(clientPath, "/");
	 						strcat(clientPath, fileName);
				            n = write(newsockfd, "File copying...", 16);
					        if (n < 0) 
		        				error("ERROR writing to socket");   
		 					boolean = 1;
		 				}

	 				}
	 				fclose(filedes);
	 				if(!boolean){
			            n = write(newsockfd, "Client not found.File copying server's local directory...", 60);
				        if (n < 0) 
	        				error("ERROR writing to socket");
						strcpy(clientPath, getcwd(path, PATH_MAX));
						strcat(clientPath, "/");
						strcat(clientPath, fileName);
	 				}
	 				while((tofides = fopen(clientPath, "r"))!= NULL){
 						fclose(tofides);
 						for (j = (strlen(clientPath) -1); clientPath[j] != '/'; --j){
 							if(clientPath[j]=='.'){
 								for(i=0 ; i<j-1 ; ++i){
 									sendPath[i] = clientPath[i];
 								}
 								sendPath[i] = fileCounter;
 								++fileCounter;
 								for(i=j ; i<(strlen(clientPath)-1) ; ++i){
 									sendPath[i] = clientPath[i];
 								}
 								for (i=0; i < (strlen(sendPath)-1); ++i){
 									clientPath[i] = sendPath[i];
 								}
 							}
 						}
					}	
					if((tofides = fopen(clientPath, "w")) ==NULL)
						error("copy file can not open");
					fclose(tofides);
				    if ((tofd = open(clientPath, 0666)) == -1)
		       			error("Failed to create output file");
		       		copyfile(fromfd, tofd);
		       		close(fromfd);
		       		close(tofd);
	 			}
		        else if(strncmp("die", buffer, 3)==0){
		        	fprintf(stderr, "Client %d disconnected\n", getpid() );
		        	FILE *temp = fopen(TEMPFILE, "w");
		        	filedes = fopen(XFILES, "r+");
					while (fscanf(filedes, "%s %s", buffer, clientPath) != EOF){
						if(getpid() != atoi(buffer))
							fprintf(temp, "%s %s\n", buffer, clientPath);
					}
					fclose(temp);
	 				fclose(filedes);
	 				filedes = fopen(XFILES, "w");
	 				temp = fopen(TEMPFILE, "r");
	 				while(fscanf(temp, "%s %s", buffer, clientPath) != EOF)
	 					fprintf(filedes, "%s %s\n", buffer, clientPath);
		        	fclose(temp);
	 				fclose(filedes);
		        	exit(EXIT_FAILURE);
		        }
 			}
		}
		close(newsockfd);
	}

	return 0; 
}
void error(char *msg)
{
    perror(msg);

    exit(1);
}
void listServer(char *path, int newsockfd)
{
	int n;
	struct dirent *direntp;
	DIR *dirp;
	char dirName[1000];
	int now=0;
	/* eger path file ise icindeki kelime sayisini bulmak icin
	 * numberofWord fonksiyonu cagirilacak*/
	if(isdirectory(path)==0){
		if( write(newsockfd, path, PATH_MAX) < 0)
			error("ERROR writing from socket");
	}
	else if(isdirectory(path)<0){
		error("ERROR is not a directory or a file!");
	}
	/*directory */
	if ((dirp = opendir(path)) == NULL || path[0]=='.')
		return;

	while ((direntp = readdir(dirp)) != NULL){
		if(direntp->d_name[0]=='.')
			continue;
		strcpy(dirName,path);
		strcat(dirName,"/");
		strcat(dirName,direntp->d_name);
		listServer(dirName, newsockfd);
	}
	while ((closedir(dirp) == -1) && (errno == EINTR));
	return;
}
int isdirectory(char *path)
{
	struct stat statbuf;

	/*verilen dosya yolu file veya directory degil ise*/
	/*-1 return edilecek*/
	/*aksi halde hangisi oldugu return edilecek*/
	if (stat(path, &statbuf) == -1)
		return -1;
	else
		return S_ISDIR(statbuf.st_mode);
}

void intHandler(int dummy){
	keepRunning=0;
	fprintf(stderr,"ctrl+c handled\n Program terminating...");
	int i;
	char path[PATH_MAX];
	FILE *x;
	x = fopen(XFILES, "r");
	while(fscanf(x, "%d %s", &i, path) != EOF){
    	r_write(globalSocket, "kill", 5);
		if(kill(i, SIGKILL)==-1)
			fprintf(stderr, "signal can not killed\n");
	}
	close(globalSocket);
	fclose(x);
	remove(XFILES);
	exit(EXIT_SUCCESS);
}















