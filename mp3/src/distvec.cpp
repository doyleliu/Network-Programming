#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

struct Node{
    int destination;
    int nexthop;
    int pathcost;
    struct Node* nextNode;
};

typedef Node* dvNode;

bool nodeexists[1000];

bool changeStatus = false;

int getNodeNum(string fileName){
    int nodeNum = 0;
    ifstream fin(fileName);

    int a, b, c;
    while(fin >> a >> b >> c){
        // cout << a << " " << b << " " << c << endl;
        if(a > nodeNum) nodeNum = a;
        else if(b> nodeNum) nodeNum = b;
    }
    // cout << nodeNum << endl;
    return nodeNum;

}

void makeGraph(string fileName, int **graph){
    ifstream fin(fileName);
    int a, b , c;
    while(fin >> a >> b >> c){
        // cout << a << " " << b << " " << c << endl;
        graph[a-1][b-1] = c;
        graph[b-1][a-1] = c;
        nodeexists[a - 1] = true;
        nodeexists[b - 1] = true;
    }
}

//init router table
void initRouterTable(int **graph, int nodeNum, dvNode *routerHeaders){
    for(int i = 0; i < nodeNum; i ++){
        routerHeaders[i] = NULL;
    }

    for(int i = 0; i < nodeNum; i ++){
        for(int j= 0; j < nodeNum; j ++){
            // if(graph[i][j]>0){
                dvNode tmpNode = (dvNode)malloc(sizeof(Node));
                tmpNode->destination = j;
                tmpNode->nexthop = j;
                tmpNode->pathcost = graph[i][j];
                tmpNode->nextNode = routerHeaders[i];
                routerHeaders[i] = tmpNode;
            // }
            
        }
    }
}

dvNode fullCopy(dvNode head){
    dvNode tmpNode = head;
    dvNode newNode = NULL;
    while(tmpNode){
        dvNode tmp=(dvNode)malloc(sizeof(Node));
        tmp->destination = tmpNode->destination;
        tmp->nexthop = tmpNode->nexthop;
        tmp->pathcost = tmpNode->pathcost;
        tmp->nextNode = newNode;
        newNode = tmp;
        tmpNode = tmpNode->nextNode;
    }
    return newNode;
}

void deleteNode(dvNode head){
    dvNode tmpNode = head;
    dvNode afterNode = tmpNode->nextNode;
    while(tmpNode){
        free(tmpNode);
        tmpNode = afterNode;
        if(afterNode) afterNode = afterNode->nextNode;
    }
    
}

void printTable(dvNode head){
    dvNode tmp= head;
    while(tmp){
        cout << tmp->destination + 1 << "\t\t" << tmp->pathcost << "\t" << tmp->nexthop + 1 << endl;
        tmp = tmp->nextNode;
    }
}

//get to update the vector
void tableUpdate(int **graph, int nodeNum, dvNode *routerHeaders, dvNode * newRouterHeaders){
    for(int i = 0; i < nodeNum; i ++ ){
        dvNode tmpNode = fullCopy(routerHeaders[i]);
        // printTable(tmpNode);
        for(int j = 0 ; j < nodeNum; j ++){
            if(graph[i][j] > 0){
                dvNode t = routerHeaders[j];
                dvNode head = NULL;
                while(t){
                    dvNode np = (dvNode)malloc(sizeof(Node));
                    np->destination = t->destination;
                    np->nexthop = j;
                    // cout << t->pathcost << endl;
                    if(t->pathcost == -999) np->pathcost = -999;
                    else np->pathcost = t->pathcost + graph[i][j];
                    np->nextNode = head;
                    head = np;

                    t = t->nextNode;
                }

                dvNode t2 = head;
                while(t2){
                    // if(t2->pathcost == -999){
                    //     cout << "i am here here!!" << endl;
                    //     t2 = t2->nextNode;
                    //     continue;
                    // }
                    // cout << "i am here!" << endl;
                    dvNode t1 = tmpNode;
                    int destination = t2->destination;
                    while(t1 && (t1->destination!=destination)){
                        // cout << "t1: " << t1->destination << " t2: " << destination<< endl;
                        t1 = t1->nextNode;
                    } 
                    // cout << "end" << endl;
                    if(t1 == NULL){
                        if(t2->destination != i){
                            // cout << "i am here1!" << endl;
                            changeStatus = true;
                            dvNode n1 = (dvNode)malloc(sizeof(Node));
                            n1->destination = t2->destination;
                            n1->nexthop = t2->nexthop;
                            n1->pathcost = t2-> pathcost;

                            n1->nextNode = tmpNode;
                            tmpNode = n1;
                        }
                        else{
                            // cout << "i am here2!" << endl;
                            changeStatus = true;
                            dvNode n1 = (dvNode)malloc(sizeof(Node));
                            n1->destination = i;
                            n1->nexthop = i;
                            n1->pathcost = 0;

                            n1->nextNode = tmpNode;
                            tmpNode = n1;
                        }
                    }
                    else{
                        if(t1->nexthop == j){
                            if(t1->pathcost != t2->pathcost){
                                // cout << "i am here2!" << endl;
                                changeStatus = true;
                            } 
                            t1->pathcost = t2->pathcost;
                        }
                        else if(t1->pathcost == -999 && t2->pathcost != -999){
                            changeStatus = true;
                            t1->pathcost = t2->pathcost;
                            t1->nexthop = t2->nexthop;
                        }
                        else if(t2->pathcost == -999){
                            t2 = t2;
                        }
                        else{
                            if(t2->pathcost < t1->pathcost){
                                // cout << "i am here!" << endl;
                                // cout << t2->destination << " " << t1->destination<<endl;
                                // cout << ": "<<t2->pathcost << " " << t1->pathcost<<endl;
                                changeStatus = true;
                                t1->pathcost = t2->pathcost;
                                t1->nexthop = t2->nexthop;
                                // cout << ", " << t2->pathcost << " " << t1->pathcost<<endl;
                            }
                            else if(t2->pathcost == t1->pathcost && t2->nexthop < t1->nexthop){
                                changeStatus = true;
                                t1->nexthop = t2->nexthop;

                            }
                        }
                    }

                    t2 = t2->nextNode;
                }
            }
        }

        newRouterHeaders[i] = tmpNode;        

    }

    for(int i = 0 ; i < nodeNum; i ++){
        deleteNode(routerHeaders[i]);
        routerHeaders[i] = newRouterHeaders[i];
    }
    // cout << changeStatus << endl;
}

dvNode changeOrder(dvNode head){
    if(head == NULL || head->nextNode == NULL){
        return head;
    }
    dvNode prev = head;
    dvNode cur = head->nextNode;
    dvNode temp = head->nextNode->nextNode;

    while(cur){
        temp = cur->nextNode;
        cur->nextNode = prev;
        prev = cur;
        cur = temp;
    }
    head->nextNode = NULL;
    return prev;

}

void sendMessage(ofstream* fpOut, string fileName, dvNode *routerHeaders , int nodeNum ){
    ifstream fin(fileName);
    int orig, dest;
    string msg;
    // fin >> orig >> dest;
    
    while(fin >> orig >> dest){
        getline(fin,msg);
        // cout << "origin: " << orig << " destination: " << dest << " message:" << msg << endl;
        int hopnum = 0;
        string hoproute = "";
        int currentpos = orig - 1;
        bool isFirst = true;
        bool brokenstate = false;
        // cout << "currentpos: " << currentpos << endl;
        while(currentpos != dest - 1){
            hoproute = hoproute + " " + to_string(currentpos + 1);
            dvNode tmpNode = routerHeaders[currentpos];
            while(tmpNode->destination != dest - 1){
                tmpNode = tmpNode->nextNode;
            
            } 
            currentpos = tmpNode->nexthop;
            if(tmpNode->pathcost == -999){
                brokenstate = true;
                break;
            }
            if(isFirst) hopnum = tmpNode->pathcost;
            isFirst = false;
        }

        if(brokenstate == true){
            *fpOut << "from " << orig << " to " << dest << " cost infinite hops unreachable " << "message" << msg << endl;
        }
        else *fpOut << "from " << orig << " to " << dest << " cost " << hopnum << " hops" << hoproute << " message" << msg << endl;
    }

}


void changeState(ofstream* fpOut, string fileName, string msgfile, dvNode *routerHeaders , dvNode * newrouterHeaders, int nodeNum ,int **graph){
    ifstream fin(fileName);
    changeStatus = true;
    int a, b ,c;
    while(fin >> a >> b >> c){
        cout << a << b << c << endl;
        *fpOut << endl;
        graph[a-1][b-1] = c;
        graph[b-1][a-1] = c;
        initRouterTable(graph, nodeNum, routerHeaders);
        int cnt = 0;
        while(true){
            changeStatus = false;
            tableUpdate(graph, nodeNum, routerHeaders, newrouterHeaders);
            cnt = cnt + 1;
            // printTable(routerHeaders[0]);
            
            // cout << "change status: " << changeStatus << endl;
            if(!changeStatus) break;
        }

        if(cnt % 2 == 0){
            for(int i = 0 ; i < nodeNum; i ++){
                routerHeaders[i] = changeOrder(routerHeaders[i]);
            }
        }

        for(int i = 0 ; i < nodeNum; i ++){
            dvNode tmpNode = routerHeaders[i];

            // int Nodecnt = 0; 
            // while(tmpNode){
            //     if(tmpNode->pathcost == -999) Nodecnt ++;
            //     tmpNode = tmpNode->nextNode;
            // }
            // if(Nodecnt == nodeNum - 1) continue;
            // tmpNode = routerHeaders[i];
            if(nodeexists[i] == false) continue;

            while(tmpNode){
                // if(Nodecnt == 0 && tmpNode->nextNode == NULL) break;
                if(tmpNode->pathcost != -999)
                    *fpOut << tmpNode->destination + 1 << " " << tmpNode->nexthop + 1 << " " << tmpNode->pathcost << endl;
                tmpNode = tmpNode->nextNode;
                // Nodecnt ++;
            }
            *fpOut << endl;
        }

        sendMessage(fpOut, msgfile, routerHeaders, nodeNum);


    }

}




int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./distvec topofile messagefile changesfile\n");
        return -1;
    }

    // FILE *fpOut;
    // fpOut = fopen("output.txt", "w");
    ofstream fpOut("output.txt");
    
    // cout << argv[1]<< endl;
    for(int i = 0 ; i < 1000; i ++){
        nodeexists[i]  = false;
    }

    int nodeNum = getNodeNum(argv[1]);
    // cout << nodeNum << endl;

    int **graph; // top of the graph and init it
    graph = new int *[nodeNum];
    for(int i = 0; i < nodeNum; i ++){
        graph[i] = new int[nodeNum];
        for(int j = 0; j < nodeNum; j ++)
            if(i == j) graph[i][j] = 0;
            else graph[i][j] = -999;
    }
    // cout << "I am here"<< endl;
    makeGraph(argv[1], graph);
    dvNode routerHeaders[nodeNum];
    dvNode newrouterHeaders[nodeNum];

    initRouterTable(graph, nodeNum, routerHeaders);
    // printTable(routerHeaders[0]);
    
    int cnt = 0;
    while(true){
        changeStatus = false;
        tableUpdate(graph, nodeNum, routerHeaders, newrouterHeaders);
        cnt = cnt + 1;
        // printTable(routerHeaders[0]);
        
        // cout << "change status: " << changeStatus << endl;
        if(!changeStatus) break;
    }

    if(cnt % 2 == 0){
        for(int i = 0 ; i < nodeNum; i ++){
            routerHeaders[i] = changeOrder(routerHeaders[i]);
        }
        // printTable(routerHeaders[3]);
    }

    //pring topology entries
    for(int i = 0 ; i < nodeNum; i ++){
        dvNode tmpNode = routerHeaders[i];
        // int Nodecnt = 0; 
        //     while(tmpNode){
        //         if(tmpNode->pathcost == -999) Nodecnt ++;
        //         tmpNode = tmpNode->nextNode;
        //     }
        // if(Nodecnt == nodeNum - 1) continue;
        // tmpNode = routerHeaders[i];
        if(nodeexists[i] == false) continue;

        while(tmpNode){
            // if(Nodecnt == 0 && tmpNode->nextNode->nextNode == NULL) break;
            if(tmpNode->pathcost != -999)
                fpOut << tmpNode->destination + 1 << " " << tmpNode->nexthop + 1 << " " << tmpNode->pathcost << endl;
            tmpNode = tmpNode->nextNode;
            // Nodecnt ++;
        }
        fpOut << endl;
    }

    //send message
    sendMessage(&fpOut, argv[2], routerHeaders, nodeNum);

    //change status
    changeState(&fpOut, argv[3], argv[2], routerHeaders , newrouterHeaders, nodeNum ,graph);

    // fclose(fpOut);

    return 0;
}

