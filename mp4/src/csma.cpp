#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

class node{
    public:
        int collisionCount;
        int backoff;
        int backoffRange;
        int packetLength;
        node(int B, int L, int R){
            backoff = B;
            backoffRange = R;
            packetLength = L;
            collisionCount = 0;
        }

        void continueCount(){
            backoff -- ;
        };
};



vector<int> currentActive(vector<node *>& nodevec){
    vector<int> result;
    for(int i = 0 ; i < nodevec.size(); i++){
        if(nodevec[i]->backoff == 0){
            result.push_back(i);
        }
    }
    return result;
}

void countDown(int N, int L, int M, int totalTime, vector<int>& Rrange,vector<node *>& nodevec){
    bool channelIsIdle = true;
    int remainTime = totalTime;
    int idletime = 0;
    int totalPkt = 0;
    int collisionnum = 0;
    for(; remainTime >= 0; remainTime --){
        // cout << remainTime << endl;
        vector<int> tmp = currentActive(nodevec);
        // cout << "length: "<< tmp.size()<< endl;
        // for(int i = 0 ; i < nodevec.size(); i++){
        //     cout << nodevec[i]->backoff << " ";
        // }
        // cout << ":" << tmp.size() << "current time: " <<totalTime - remainTime + 1 << endl;
        if(tmp.size() == 0){
            idletime ++;
            if(channelIsIdle){
                for(int i = 0 ; i < nodevec.size(); i++){
                    nodevec[i]->continueCount() ;
                }
            }
            
        }
        else if(tmp.size() == 1){
            // cout << "nodevec[tmp[0]]->packetLength: " <<nodevec[tmp[0]]->packetLength << endl;
            channelIsIdle = false;
            int currentNum = tmp[0];
            if(nodevec[tmp[0]]->packetLength > 0 ){
                nodevec[tmp[0]]->packetLength -- ;
                totalPkt ++;
            }
            if(nodevec[tmp[0]]->packetLength == 0) channelIsIdle = true;
            // cout << "channelIsIdle: " << channelIsIdle << endl;
            
        }
        else{
            collisionnum ++;
            for(int i = 0; i < tmp.size(); i ++){
                
                // cout << "Current collisionCount: " << nodevec[tmp[i]]->collisionCount;
                // srand((unsigned)time(NULL)); 
                if(nodevec[tmp[i]]->collisionCount > M){
                    //drop packet;
                    nodevec[tmp[i]]->backoffRange = Rrange[0];
                    nodevec[tmp[i]]->backoff = rand() % (1 + nodevec[tmp[i]]->backoffRange) + 1;
                    nodevec[tmp[i]]->collisionCount = 0;
                    // cout << "Current packet length: " << nodevec[tmp[i]]->packetLength;
                }
                else{
                    nodevec[tmp[i]]->backoff = rand() % (1 + nodevec[tmp[i]]->backoffRange) + 1;
                    if(nodevec[tmp[i]]->backoffRange < Rrange[Rrange.size() - 1])
                        nodevec[tmp[i]]->backoffRange *= 2;
                    // cout << nodevec[i]->backoff << " ";
                    nodevec[tmp[i]]->collisionCount ++;
                }
                // nodevec[tmp[i]]->collisionCount ++;
            }

            for(int i = 0; i < nodevec.size(); i ++) nodevec[i]->backoff --;
        }

        if(channelIsIdle){
            for(int i = 0; i < nodevec.size(); i ++){
                if(nodevec[i]->packetLength == 0){
                    //remove packet
                    nodevec.erase (nodevec.begin()+i);
                    i --;
                }
            }
        }
        // else totalPkt ++;

        if(nodevec.size() == 0) break;
    }

    cout << "N: " << N  << " " << "channel utilization: " <<  totalPkt * 1.0 / (totalTime - remainTime + 1) << endl;
    // cout << "N: " << N  << " " << "channel idle fraction: " <<  idletime * 1.0 / (totalTime - remainTime + 1) << endl;
    // cout << "N: " << N  << " " << "channel collision number: " <<  collisionnum * 1.0 << endl;
}



int main(int argc, char **argv){
    if (argc != 2) {
        printf("Usage: ./csma inputfile\n");
        return -1;
    }
    srand((unsigned)time(NULL)); 
    string fileName = argv[1];

    ifstream fin(fileName);
    string line;
    char tmp;
    int N, L , R , M, T;
    fin >> tmp >> N;
    fin >> tmp>> L >> tmp; //N,L
    vector<int>Rrange;
    while(fin.get()!='\n'){
        fin>> R;
        Rrange.push_back(R);
    }
    
    fin >> tmp >> M;
    fin >> tmp >> T;

    int globalTime = 0;

    // vector<node *> nodevec(N);

    // for(int i = 0; i < nodevec.size(); i ++){
    //     nodevec[i] = new node(0, L, Rrange[0]);
    // }
    // countDown(N, L,  M, T, Rrange, nodevec);
    for(int j = 1; j <= 500; j ++){
        vector<node *> nodevec(j);

        for(int i = 0; i < nodevec.size(); i ++){
            nodevec[i] = new node(0, L, Rrange[0]);
        }
        countDown(j, L,  M, T, Rrange, nodevec);
    }


}