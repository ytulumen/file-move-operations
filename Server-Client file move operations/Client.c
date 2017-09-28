/*************************************
 * System Programming 244 FINAL
 * Yasin_Tulumen_121044020
 * Client.c
*************************************/ 

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

static volatile int keepRunning = 1 ;
int globalSocket;

void listLocal(char *path);
int isdirectory(char *path);
void error(char *msg);
void intHandler(int dummy);

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256], *res, path[PATH_MAX+1];
    if (argc < 3) {
       	fprintf(stderr,"usage %s hostname port\n", argv[0]);
       	exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    	error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


	signal(SIGINT, intHandler);
	globalSocket = sockfd;
    getcwd(path, sizeof(path));
    n = write(sockfd,path,strlen(path)+1);
    if (n < 0) 
    	error("ERROR writing to socket");

    while(1){
    	printf("Please enter the command: ");
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        if(strncmp("listLocal", buffer, 9)==0){
    	    getcwd(path, sizeof(path));
        	listLocal(path);
            n = write(sockfd,buffer,strlen(buffer));
	        if (n < 0) 
	        	error("ERROR writing to socket");
        }
        else if(strncmp("listServer", buffer, 10)==0){
            n = write(sockfd,buffer,strlen(buffer));
	        if (n < 0) 
	        	error("ERROR writing to socket");
            do{
            	fprintf(stderr, "%s\n", path);
	 			bzero(path,PATH_MAX);

	            n = read(sockfd,path, PATH_MAX);
	        	if (n < 0) 
	            	error("ERROR reading from socket");
		        if(strncmp("kill", path, 4)==0){
		        	fprintf(stderr, "server said that i will gone die\n" );
		        	exit(EXIT_FAILURE);
		        }
		        else if(strncmp("end", path, 3)==0)
		        	n=-1;
            }while(n>0);
        }
        else if(strncmp("lsClients", buffer, 9)==0){
            n = write(sockfd,buffer,strlen(buffer));
	        if (n < 0) 
	        	error("ERROR writing to socket");
	        fprintf(stderr, "Connected clients:\n" );
 			bzero(buffer, 256);
            n = read(sockfd, buffer, 255);
        	if (n < 0) 
            	error("ERROR reading from socket");
	        if(strncmp("kill", buffer, 4)==0){
	        	fprintf(stderr, "server said that i will gone die\n" );
	        	exit(EXIT_FAILURE);
	        }
        	fprintf(stderr, "%s", buffer);

        }
        else if(strncmp("sendFile", buffer, 8)==0){
            n = write(sockfd,buffer,strlen(buffer));
	        if (n < 0) 
	        	error("ERROR writing to socket");
            n = read(sockfd, buffer, 255);
        	if (n < 0) 
            	error("ERROR reading from socket");
        	fprintf(stderr, "%s\n", buffer);
        	fprintf(stderr, "File copied.\n");
        }
        else{
            n = read(sockfd, buffer, 255);
        	if (n < 0) 
            	error("ERROR reading from socket");
	        if(strncmp("kill", buffer, 4)==0){
	        	fprintf(stderr, "server said that i will gone die\n" );
	        	exit(EXIT_FAILURE);
	        }
        }
        bzero(buffer,256);
    }
    close(sockfd);
    return 0;
}
void error(char *msg)
{
    perror(msg);
    exit(0);
}
void listLocal(char *path)
{
	struct dirent *direntp;
	DIR *dirp;
	char dirName[1000], fileName[256];
	int now=0, childpid;
	/* eger path file ise icindeki kelime sayisini bulmak icin
	 * numberofWord fonksiyonu cagirilacak*/
	if(isdirectory(path)==0){
		fprintf(stderr, "%s\n", path);
	}
	else if(isdirectory(path)<0){
		printf("Error! %s is not a directory or a file!\n",path );
		return ;
	}
	/*directory */
	if ((dirp = opendir(path)) == NULL || path[0]=='.') 
		return ;

	while ((direntp = readdir(dirp)) != NULL){
		if(direntp->d_name[0]=='.')
			continue;
		strcpy(dirName,path);
		strcat(dirName,"/");
		strcat(dirName,direntp->d_name);
		listLocal(dirName);
	}
	while ((closedir(dirp) == -1) && (errno == EINTR));
	return ;
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
	printf("ctrl+c catched\n");

	r_write(globalSocket, "die", 4);

	exit(EXIT_SUCCESS);
}







