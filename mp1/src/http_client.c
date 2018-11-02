#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>


#define MAXDATASIZE 5000
#define PORT "80"
#define FileNameSize 1000

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if(argc != 2){
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //process input into useful information
    char fileName[FileNameSize];
    char delims[] = "/";
    char *ipAddrwithPort = strtok(argv[1], delims);
    ipAddrwithPort = strtok (NULL, "/");
    char *tmpName= strtok(NULL, "/");
    int pos = 0;
    while(tmpName != NULL){
        for(int i = 0; i < strlen(tmpName); i ++){
            fileName[pos] = tmpName[i];
            pos ++ ;
        }
        fileName[pos] = '/';
        pos ++;
        tmpName= strtok(NULL, "/");

    }
    if(pos > 0) fileName[pos-1] = '\0';
    // printf("The divided input3 is %s\n", fileName);
    char * ipAddr = strtok(ipAddrwithPort, ":");
    // printf("The divided ip address is %s\n", ipAddr);
    char * port = strtok(NULL,"\0");
    // printf("The divided port is %s\n", port);
    if(port == NULL || strlen(port) <= 0) port = PORT;
    //  printf("The port is %s\n", port);

    if((rv = getaddrinfo(ipAddr, port, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next ){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
		return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);


    //set our http get file
    memset(buf, '\0', sizeof buf);
    sprintf(buf, "GET /%s HTTP/1.1\r\n", fileName);
    sprintf(buf+strlen(buf), "User-Agent:  Wget/1.12 (linux-gnu)\r\n");
    sprintf(buf+strlen(buf), "Host:  %s:%s\r\n", ipAddr, port);
    sprintf(buf+strlen(buf), "Connection:  Keep-Alive\r\n\r\n");

    if((numbytes = send(sockfd, buf, strlen(buf), 0)) == -1){
        perror("send");
        exit(1);
    }

    FILE *fp;
    fp = fopen("output", "wb");
    memset(buf, '\0', sizeof buf);

    // remove protocal lines

    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        exit(1);
    }
    char * tmp = strstr(buf, "\r\n\r\n");
    if(tmp != buf){
        int prevsize = (int) (tmp - buf);
        fwrite(buf + prevsize + 4, sizeof(char), numbytes - prevsize - 4 , fp);

    }
    else fwrite(buf, sizeof(char), numbytes, fp);
    memset(buf, '\0', sizeof buf);



    while(numbytes != 0){
        if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
            perror("recv");
            exit(1);
        }
        fwrite(buf, sizeof(char), numbytes, fp);
    }

    // if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
    //     perror("recv");
    //     exit(1);
    // }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n", buf);
    
    fclose(fp);
    close(sockfd);

    return 0;
}