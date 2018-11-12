/* 
 * File:   sender_main.c
 * Author: 
 *
 * Created on 
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include <queue> 
#include <vector>
#include <math.h>
#include <sys/time.h>
#include <sstream>
#include <errno.h>

#define PACKETSIZE 1200
#define HEADSIZE 16

using namespace std;

int SST = 128;
float CW = 256;

struct Packet{
    long long int num;
    char pac[PACKETSIZE];
};

struct AckPkt{
    long long int num;
};

struct sockaddr_in si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}

void makeHeader(int num, char * buf){
    stringstream ss;
    ss << num;
    string tmpstring = ss.str();
    //  = to_string(num);
    int i = 0;
    for(i = 0; i < tmpstring.size();i ++){
        buf[i] = tmpstring[i];
    }
    for(; i < HEADSIZE; i ++){
        buf[i] = '\0';
    }
    return;
}

void writeFileToBuffer(long long int num, long long int totalbytes, FILE *fptr, char *buf){
    int bytestocopy = PACKETSIZE;
    if((num + 1) * PACKETSIZE > totalbytes)
        bytestocopy = totalbytes % PACKETSIZE;
    if(fread(&buf[HEADSIZE], sizeof(char), bytestocopy, fptr) != bytestocopy){
        printf("sender error!!");
        exit(1);
    }
    buf[HEADSIZE + bytestocopy] = '\0';
    if(bytestocopy < PACKETSIZE){
        for(int i = bytestocopy; i < PACKETSIZE; i++)
            buf[HEADSIZE + i] = '\0';
    }
    // printf("sender element %d \n", num);
    // printf("bytestocopy %d \n",bytestocopy);
    // printf("Final do !");
    
    return;
}

int getIndex(char *ack_buf){
    // printf("ack_buf is %s \n", ack_buf);
    int tmpnum = 0;
    for(int i =0; i < HEADSIZE; i ++){
        if(ack_buf[i] != '\0'){
            tmpnum = tmpnum * 10 + (ack_buf[i] - '0');
        }
        else break;
    }
    return tmpnum;
}

void setTimeOut(struct timeval timev){
    if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(struct timeval)) < 0){
        fprintf(stderr, "Set TimeOut Error! \n");
        return;
    }
}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    //Open the file
    

    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }
    fseek(fp, 0L, SEEK_END);
    long long int size = ftell(fp);
    printf("the file size is %d \n", size);
    rewind(fp);

    if(bytesToTransfer > size)bytesToTransfer = size;

	/* Determine how many bytes to transfer */
    //bytesToTransfer

    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep((char *)"socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


	/* Send data and receive acknowledgements on s*/
    // struct sockaddr_in ret_addr;
    socklen_t addrlen = sizeof(si_other);

    int sentNum = 0 ;
    int base = 0;
    // int transferBytes = bytesToTransfer;
    // int ackedNum = 0;
    int dupAck = 0;
    bool isdup = false;
    bool isend = false;

    int state = 0; // slowstart = 0; congestionAvoidence = 1; fastrecovery = 2;

    int totalPkt = 0;
    int remainPkt  = 0;
    printf("there are %d bytes to transfer\n", bytesToTransfer);
    if(bytesToTransfer % PACKETSIZE == 0) remainPkt = bytesToTransfer/ PACKETSIZE;
    else remainPkt = bytesToTransfer/ PACKETSIZE + 1;
    totalPkt = remainPkt;
    int nextPkt = 0;
    int recvBytes = 0;
    char buf[PACKETSIZE + HEADSIZE];
    memset(buf , 0 , PACKETSIZE + HEADSIZE);
    queue<char *> unAckPkt ;
    char ack_buf[HEADSIZE];
    CW = 1;
    
    struct timeval timev;
    timev.tv_sec = 0;
    timev.tv_usec = 22000;

    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(struct timeval));
    
    // printf("there are %d pkt to transfer\n", remainPkt);
    // first send the packet information
    makeHeader(bytesToTransfer, buf);
    printf("Packet information %d pkt to transfer\n", totalPkt);
    if(sendto(s, buf, sizeof(buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
        diep((char *)"send");
            exit(1);
    }
    memset(buf , 0 , PACKETSIZE + HEADSIZE);
     float cwEnd = 0;
    


    while(base < totalPkt){
        if(CW + base <= totalPkt) cwEnd = CW + base;
        else cwEnd = totalPkt;
        memset(buf , 0 , PACKETSIZE + HEADSIZE);
        while(nextPkt < cwEnd){
            // printf("in the while loop \n");
            // printf("nextPkt is : %d \n", nextPkt);
            //decide whether to make a new pkt
            makeHeader(nextPkt, buf);
            writeFileToBuffer(nextPkt, bytesToTransfer, fp, buf);
            char *tmpbuf = new char[PACKETSIZE + HEADSIZE];
            for(int cnt  = 0; cnt < PACKETSIZE + HEADSIZE; cnt ++ ){
                tmpbuf[cnt] = buf[cnt];
            }
            //strcpy(tmpbuf, buf);
            unAckPkt.push(tmpbuf);
            // printf("tmpbuf %s \n", &tmpbuf[HEADSIZE]);
            // printf("buf %s \n", &buf[HEADSIZE]);

            // printf("buf is : %s \n", buf);
            //send pkt
            if(sendto(s, buf, sizeof(buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
                diep((char *)"send");
                exit(1);
            }
            // printf(tmpPacket->pac);
            nextPkt ++;
            // printf("nextPkt no.%d \n", nextPkt);
        }
        // printf("Enter Here \n"); // has reached this point;
        // break;
        while(cwEnd <= nextPkt){
            
            // printf("Enter Here \n");
            memset(ack_buf , 0 , sizeof(ack_buf));
            if((recvBytes = recvfrom(s, ack_buf , sizeof(ack_buf), 0, (struct sockaddr *) & si_other, & addrlen)) == -1){
                if(EAGAIN | EWOULDBLOCK){
                    SST = 1;
                    CW =  1;
                    char *tmpbuf = unAckPkt.front();
                    // nextPkt = base;
                    // printf("front element %d \n", getIndex(tmpbuf));
                    if(sendto(s, tmpbuf, sizeof(buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
                        diep((char *)"send");
                        exit(1);
                    }
                    
                    state = 0;

                    // setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(struct timeval));
                    //printf("hahahahahaha %s", &tmpbuf[HEADSIZE]);
                    //setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(struct timeval));
                    //continue;
                    break;
                }
                // perror("recvfrom");
                // exit(1);
            }
            int currentAck = getIndex(ack_buf);
            // printf("current ack np.%d \n", currentAck);
            if(state == 0){
                if(currentAck >= base){
                    // isdup = false;
                    dupAck = 0;
                    if(CW >= SST){
                        CW += 1/CW * (currentAck - base + 1);
                        state = 1; // congestion avoidence;
                    }
                    else CW = CW + (currentAck - base + 1);

                    base = currentAck + 1;
                }
                else if(currentAck == (base -1)){
                    dupAck ++ ;
                    if(dupAck == 3){
                        SST = CW/2;
                        CW = SST + 3;
                        // nextPkt = currentAck;
                        char *tmpbuf = unAckPkt.front();
                        // printf("font element2 %d \n", getIndex(tmpbuf));
                        if(sendto(s, tmpbuf, sizeof(tmpbuf), 0 , (struct sockaddr *) &si_other, slen) == -1){
                                diep((char *)"send");
                                exit(1);
                        }
                        state = 2; // fast recovery;
                        // break; 
                    }
                }
            }
            else if(state == 1){
                if(currentAck >= base){
                    dupAck = 0;                
                    CW += 1/CW * (currentAck - base + 1);
                    base = currentAck + 1;
                }
                else if(currentAck == (base -1)){
                    dupAck ++ ;
                    if(dupAck == 3){
                        SST = CW/2;
                        CW = SST + 3;
                        // nextPkt = currentAck;
                        char *tmpbuf = unAckPkt.front();
                        // printf("font element2 %d \n", getIndex(tmpbuf));
                        if(sendto(s, tmpbuf, sizeof(tmpbuf), 0 , (struct sockaddr *) &si_other, slen) == -1){
                                diep((char *)"send");
                                exit(1);
                        }
                        state = 2; // fast recovery;
                        // break; 
                    }
                }
            }
            else{
                if(currentAck >= base){
                    dupAck = 0;                
                    CW = SST;
                    base = currentAck + 1;
                    state =  1;
                }
                // else if(currentAck == (base -1)){
                //     dupAck ++ ;
                //     CW = CW + 1;
                // }
            }

            if(CW >= 500){CW = 500;}
            
            // char *tmpbuf = unAckPkt.front();
            while(unAckPkt.size() > 0 && getIndex(unAckPkt.front()) <=  currentAck  ){
                // printf("I am here!");
                char *tmpbuf = unAckPkt.front();
                // printf("front element %d \n", getIndex(tmpbuf));
                unAckPkt.pop();
                delete tmpbuf;
                
            } 
            
            //printf("base %d \n", base);
            //printf("CW %f \n", CW);
            // printf("totalPkt %d \n", totalPkt);
            
            if(CW + base < totalPkt) cwEnd = CW + base;
            else cwEnd = totalPkt;
            // printf("cwEnd %f \n", cwEnd);
            if(base >= totalPkt ){
                isend = true;
                break;
            } 
        }
        if(isend == true) break;
        // if(base >= totalPkt) break;
    }

    printf("Closing the socket\n");
    close(s);
    return;

}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);



    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}

