#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define Default_PORT "80"

#define BACKLOG 10

#define MAXDATASIZE 200

void split_buffer(char buffer[MAXDATASIZE], char *arr[30]);

void sigchld_handler(int s){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void empty_buf(char buffer[]){
    for(int i=0;i<strlen(buffer);i++){
        buffer[i] = ' ';
    }
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    char buf[MAXDATASIZE];
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET_ADDRSTRLEN];
    int rv;
    char *port;


    if (argc < 2) {
        port = Default_PORT;
    }
    else{
        port = argv[1];
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) { // main accept() loop

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            //perror("accept");
            continue;
	    //exit(0);
        }
        inet_ntop(their_addr.ss_family,&(((struct sockaddr_in*)&their_addr)->sin_addr), s, sizeof s);

        printf("server: got connection from %s\n", s);
        

        if (!fork()) { // this is the child process
            //close(sockfd); // child doesn't need the listener

	        int l = 0;
    	    while(1){

                printf("%d\n", l);

                //empty_buf(buf);
                if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1){
                    perror("recv");
                    //close(new_fd);
                    exit(0);
                }
                buf[numbytes] = '\0';
                printf("\nserver: received:\n'%s'\n",buf);
    		

    	        if(strlen(buf) == 0){
    		        break;
    	        }
    		
                if(buf[0] == 'G'){
         
                    char *arr[30];
                    split_buffer(buf,arr);
                    int flag = 0;
                    char *data[10];
                    FILE *f;
                    f = fopen(arr[1], "r");
                    if(f == NULL){
                        //printf("error\n");
                        flag = 1;
                    }
                    if(flag == 0){

                        if (send(new_fd, "HTTP/1.1 200 OK\r\n\r\n", 24, 0) == -1){
                            perror("send");
    			            //close(new_fd);
                            exit(0);
                        }

                        int i=0;
                        fscanf(f,"%s",&data[i]);
                        while(strcmp(&data[i], "hh")!=0){
                            i++;
                            fscanf(f,"%s",&data[i]);
                        }
                        strcpy(&data[i],"");

                        char message[MAXDATASIZE];

                        strcpy(&message, "Data: ");
                        int j=0;
                        while(strlen(&data[j]) > 0){

                            strcat(&message,&data[j]);
                            strcat(&message, " ");
                            j++;
                        }

                        if (send(new_fd,  message, strlen(message), 0) == -1){
                            perror("send");
    			            //close(new_fd);
                            exit(0);
                        }

                    }
                    else{
                        if (send(new_fd, "HTTP/1.1 404 Not Found\r\n\r\n", 31, 0) == -1){
                            perror("send");
    			            //close(new_fd);
                            exit(0);
                        }
                    }

                }
                else if(buf[0] == 'P'){
    	

    		        if (send(new_fd, "HTTP/1.1 200 OK\r\n\r\n", 24, 0) == -1){
                        perror("send");
    		            //close(new_fd);
                        exit(0);
                    }
    		
                }
                l++;

    	    }

	    }

        
        close(new_fd); // parent doesn't need this
    }

    close(sockfd);
    return 0;
}

void split_buffer(char buffer[MAXDATASIZE], char *arr[30]){

    int j = 0;
    arr[j] = strsep(&buffer, " ");

    while(arr[j] != NULL){

        j++;
        arr[j] = strsep(&buffer, " ");

    }

}



