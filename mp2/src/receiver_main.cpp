/* 
 * File:   receiver_main.c
 * Author: 
 *
 * Created on
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <queue> 
#include <vector>
#include <cstring>
#include <sstream>
#include <unordered_map>

#define PACKETSIZE 1200
#define HEADSIZE 16

using namespace std;

struct sockaddr_in si_me, si_other;
struct Packet{
    long long int num;
    char pac[PACKETSIZE];
};

struct AckPkt{
    long long int num;
};

int s, slen;
int prevnum = 0;

void diep(char *s) {
    perror(s);
    exit(1);
}

// queue<Packet *> unAckPkt ;



int getHeader(char * buf){
    int tmp = 0;
    for(int i = 0; i < HEADSIZE; i ++){
        if(buf[i] < '0' || buf[i] > '9') break; 
        tmp = tmp * 10 + (buf[i] -'0');
    }
    return tmp;
}

class MyCompare
{
    public:
        bool operator()( char *left, char *right) const
        {
            return getHeader(left) > getHeader(right);
        }
};

void makeHeader(int num, char * ack_buf){
    stringstream ss;
    ss << num;
    string tmpstring = ss.str();
    
    int i = 0;
    for(i = 0; i < tmpstring.size();i ++){
        ack_buf[i] = tmpstring[i];
    }
    for(; i < HEADSIZE; i ++){
        ack_buf[i] = '\0';
    }
    ack_buf[i] = '\0';
    // printf("tmpstring is %s \n", ack_buf);
    return;
}

void writeBufferToFile(long long int num, long long int totalbytes, FILE *fptr, char *buf){
    // memset(buf + 64, 0 , PACKETSIZE);
    int bytestocopy = PACKETSIZE;
    if((num + 1) * PACKETSIZE > totalbytes)
        bytestocopy = totalbytes % PACKETSIZE;
    if(fwrite(&buf[HEADSIZE], sizeof(char), bytestocopy, fptr) != bytestocopy){
        printf("sender error!!");
        exit(1);
    }
    // if(prevnum >= num) 
        // printf("current writeNum %d \n", num);
        // printf("bytestocopy %d \n", bytestocopy);
    // if(bytestocopy < PACKETSIZE) printf("buffer %s \n", &buf[HEADSIZE]);
    
    prevnum = num;
    // buf[HEADSIZE + bytestocopy] = '\0';
    // printf("buffer %s \n", &buf[HEADSIZE]);
    // printf("Final do !");
    
    return;
}



void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    
    slen = sizeof (si_other);


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding Port%d \n", myUDPport);
    if (::bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");
    // printf("I am here !");

    
	/* Now receive data and send acknowledgements */  
    // struct sockaddr_in so_addr;
    socklen_t addrlen = sizeof(si_other);

    FILE *fp;
    fp = fopen(destinationFile, "wb");
    if(fp == NULL){
        fprintf(stderr, "File not exists!");
        exit(1);
    }

    char expected_buf[PACKETSIZE];
    char buf[PACKETSIZE + HEADSIZE];
    char ack_buf[HEADSIZE];
    // int base = 0;
    int ackedNum = 0;
    int recvNum= 0;
    // int nextPkt = 0;
    int recvBytes = 0;
    int totalPkt= 0;
    long long int bytesToTransfer = 0;
    // priority_queue<char *, vector<char *>, MyCompare> unAckPkt ;
    unordered_map<long long int, char *> unAckPkt;


    // while(true){
        if((recvBytes = recvfrom(s, expected_buf , sizeof(expected_buf) , 0, (struct sockaddr *) & si_other, &addrlen)) == -1){
                perror("recvfrom");
                exit(1);
        }
        // long long int tmp = 0;
        for(int i = 0; i < PACKETSIZE; i ++){
            if(expected_buf[i] < '0' || expected_buf[i] > '9') break; 
            bytesToTransfer = bytesToTransfer * 10 + (expected_buf[i] -'0');
        }

    printf("the file size is %lld \n", bytesToTransfer);
    if(bytesToTransfer % PACKETSIZE == 0) totalPkt = bytesToTransfer/ PACKETSIZE;
    else totalPkt = bytesToTransfer/ PACKETSIZE + 1;
        
    // }
    int lastrecvNum = 0;

    while(true){
        // Packet * tmpPacket;
        if((recvBytes = recvfrom(s, buf , sizeof(buf) , 0, (struct sockaddr *) & si_other, &addrlen)) == -1){
                perror("recvfrom");
                exit(1);
        }
        // printf("recvBytes %d \n", recvBytes);
        // buf[recvBytes] = '\0';
        if(recvBytes != HEADSIZE + PACKETSIZE) continue;
            // printf("recvNUm is %d ,recvBuf %s \n", recvBytes, &buf[HEADSIZE]);

        // buf = tmpPacket->pac;
       
        // break;
        recvNum = getHeader(buf);

        if(recvNum == ackedNum){
            
            // fwrite(&buf[HEADSIZE], 1, recvBytes - HEADSIZE, fp);
            writeBufferToFile(recvNum, bytesToTransfer, fp, buf);
            // printf("current ackedNum %d \n", ackedNum);
            // makeHeader(ackedNum,  ack_buf);
            // if(sendto(s, ack_buf, sizeof(ack_buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
            //     diep((char *)"send");
            //     exit(1);
            // }
            // char * prevbuffer = buf ;
            while(unAckPkt.size() > 0){
                
                // char * tmpbuffer = unAckPkt.top();
                // if(getHeader(prevbuffer) != getHeader(tmpbuffer) ){
                //     if(getHeader(prevbuffer) != (getHeader(tmpbuffer) - 1)){
                //         prevbuffer = tmpbuffer;
                //         break;
                //     }else{
                //         writeBufferToFile(getHeader(tmpbuffer), bytesToTransfer, fp, tmpbuffer); 
                //         unAckPkt.pop();
                //         delete prevbuffer;
                //         prevbuffer = tmpbuffer;
                //         ackedNum ++;
                //     } 
                    
                    
                // }
                // else{
                //     unAckPkt.pop();
                // }
                if(unAckPkt.find( ackedNum + 1) != unAckPkt.end()){
                    char * tmpbuffer = unAckPkt[ackedNum + 1];
                    // printf("Tmp buffer is %s", &tmpbuffer[HEADSIZE]);
                    writeBufferToFile(ackedNum + 1, bytesToTransfer, fp, tmpbuffer); 

                    unAckPkt.erase(ackedNum + 1);
                    ackedNum ++;
                }
                else{
                    break;
                }
                   
                
            }
            
            
            // ackedNum = ackedNum + 1;
            
            makeHeader(ackedNum ,  ack_buf);
            if(sendto(s, ack_buf, sizeof(ack_buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
                diep((char *)"send");
                exit(1);
            }
            ackedNum ++;
            

        }
        else if(recvNum > ackedNum){//the ack num is bigger than expected, which means out of order
            // AckPkt * tmpAckPkt;
            // tmpAckPkt->num = ackedNum;
            makeHeader(ackedNum - 1 ,  ack_buf);
            if(sendto(s, ack_buf, sizeof(ack_buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
                diep((char *)"send");
                exit(1);
            }

            if(unAckPkt.find(recvNum) == unAckPkt.end()){
                char * tmpbuffer =  new char[PACKETSIZE + HEADSIZE];
                // strcpy(tmpbuffer, buf);
                for(int cnt = 0; cnt <PACKETSIZE + HEADSIZE; cnt ++ ){
                    tmpbuffer[cnt] = buf[cnt];
                }
                // printf("Tmp buffer is %s", &tmpbuffer[HEADSIZE]);
                unAckPkt[recvNum] = tmpbuffer;
            }
            
        }
        // else{
        //     makeHeader(ackedNum - 1 ,  ack_buf);
        //     if(sendto(s, ack_buf, sizeof(ack_buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
        //         diep((char *)"send");
        //         exit(1);
        //     }
        // }

        // while(unAckPkt.size()>0 && ackedNum >= getHeader(unAckPkt.top())){
        //     unAckPkt.pop();
        // }
        // else{
        //     makeHeader(ackedNum ,  ack_buf);
        //     if(sendto(s, ack_buf, sizeof(ack_buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
        //         diep((char *)"send");
        //         exit(1);
        //     }
        //     unAckPkt.push(buf);
        // }
        // if(unAckPkt.size() > 30) unAckPkt.empty();
        // lastrecvNum = recvNum;
        // else{
        //     makeHeader(ackedNum,  ack_buf);
        //             if(sendto(s, ack_buf, sizeof(ack_buf), 0 , (struct sockaddr *) &si_other, slen) == -1){
        //                 diep((char *)"send");
        //                 exit(1);
        //             }
        //             unAckPkt.push(buf);
        // }
    //     printf("unAckPkt size %d \n", unAckPkt.size());

       	//printf("current ackedNum %d \n", ackedNum);
	//printf("current recvNum %d \n", recvNum);

        if(ackedNum  >= totalPkt ) break;
    
    }

    close(s);
	printf("%s received.", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}
