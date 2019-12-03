#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define Default_PORT "80"

#define MAXDATASIZE 200

void read_commands();
void set_requets();
void read_file(char *data[10], char path[50]);
void empty_buf(char buffer[]);

char command[400][4][30];
int file_length = 0;
char requests[400][199];

int main(int argc, char *argv[])
{

    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET_ADDRSTRLEN];
    char *port;

    if (argc < 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    if (argc < 3) {
        port = Default_PORT;
    }
    else{
        port = argv[2];
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, &(((struct sockaddr_in*)p->ai_addr)->sin_addr), s, sizeof s);

    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);


    read_commands();

    set_requets();

    int i;
    for(i=0;i<file_length;i++){

        printf("%d\n\n", i);

	    if (send(sockfd, &requests[i], strlen(&requests[i]), 0) == -1){
                perror("send");
                //close(sockfd);
                exit(1);
        }
	
	    //empty_buf(buf);
        if(command[i][0][7] == 'g'){

            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
		        //close(sockfd);
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("\n\nclient: received\n'%s'\n",buf);


            if(buf[9] == '2'){
		        //empty_buf(buf);
                if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
		            //close(sockfd);
                    exit(1);
                }
                buf[numbytes] = '\0';
                printf("\n\nclient: received\n'%s'\n",buf);
                
            }

        }
        else{


	        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
		        //close(sockfd);
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("\n\nclient: received\n'%s'\n",buf);


        }

    }

    close(sockfd);

    return 0;
}

void read_commands(){


    FILE *f;

    f = fopen("commands", "r");

    if(f == NULL){
        printf("error\n");
    }


    for(int i=0;i<400;i++){

        fscanf(f,"%s",&command[i][0]);
        fscanf(f,"%s",&command[i][1]);
        fscanf(f,"%s",&command[i][2]);
        fscanf(f,"%s",&command[i][3]);

        if(command[i][0][7] != 'g' && command[i][0][7] != 'p'){
            break;
        }

        file_length++;
    }

}

void set_requets(){

    for(int i=0;i<file_length;i++){
        if(command[i][0][7] == 'g'){

            strcpy(&requests[i], "GET ");
            strcat(&requests[i], command[i][1]);
            strcat(&requests[i], " HTTP/1.1\n");
            strcat(&requests[i], "header\n");
            strcat(&requests[i], "header\n");
            strcat(&requests[i], "header\n");
        }
        else{
            char *data[10];
            read_file(data,&command[i][1]);
            strcpy(&requests[i], "POST ");
            strcat(&requests[i], command[i][1]);
            strcat(&requests[i], " HTTP/1.1\n");
            strcat(&requests[i], "header\n");
            strcat(&requests[i], "header\n");
            strcat(&requests[i], "header\n\n");
            strcat(&requests[i], "Data: ");
            int j=0;
            while(strlen(&data[j]) > 0){
                strcat(&requests[i],&data[j]);
                strcat(&requests[i], " ");
                j++;
            }
            strcat(&requests[i], "\n");
        }
    }
}

void read_file(char *data[10], char path[30]){

    FILE *f;
    f = fopen(path, "r");
    if(f == NULL){
        printf("error\n");
    }
    
    int i=0;
    fscanf(f,"%s",&data[i]);
    while(strcmp(&data[i], "hh")!=0){
        i++;
        fscanf(f,"%s",&data[i]);
    }
    strcpy(&data[i],"");
}

void empty_buf(char buffer[]){
    for(int i=0;i<strlen(buffer);i++){
        buffer[i] = ' ';
    }
}
