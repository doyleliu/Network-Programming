#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <stack>
#include <cmath>
#include <cstdint>
#include <climits>


using namespace std;

bool nodeexists[1000];

struct Node{
    int val;
    struct Node* nextNode;
};
typedef Node* dvNode;

struct Path{
    string route;
    Path(){
        route = "";
    }
};

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


class graph{
    private:
        int V;
        int E;
        int **adj; // the linjie matrix
        int *vexs; // the vexs of the graph
    public:
        graph(int V, int E);
        ~graph();
        bool check_edge_value(int src, int dest, int weight);
        void makeGraph(string fileName);
        void dijkstra(int src, int prev[], int dist[], ofstream* fpOut);
        void dijkstrabet(int prev[], int dist[], ofstream* fpOut,string fileName);
        void changeState(int prev[], int dist[], ofstream* fpOut,string changeFile, string messageFile);

};

graph::graph(int V, int E){
    this->V = V;
    this->E = E;

    adj = new int *[V];
    for(int i = 0; i < V; i ++){
        adj[i] = new int[V];
        for(int j = 0; j < V; j ++){
            adj[i][j] = INT_MAX;
        }
    }
    vexs = new int[V];
    for(int i = 0; i < V; i ++){
        vexs[i] = i + 1;
    }
}


graph::~graph(){
    for(int i =0; i <V; i++){
        delete[] adj[i];
    }
    delete adj;
    delete[] vexs;
}

void graph::makeGraph(string fileName){

    ifstream fin(fileName);
    int a, b , c;
    while(fin >> a >> b >> c){
        // cout << a << " " << b << " " << c << endl;
        if(c == - 999) c= INT_MAX;
        adj[a-1][b-1] = c;
        adj[b-1][a-1] = c;
        E ++;
        nodeexists[a - 1] = true;
        nodeexists[b - 1] = true;
    }
}

void graph::dijkstra(int vex , int prev[], int dist[], ofstream* fpOut){
    if(nodeexists[vex] == false) return;
    int min;
    int tmp;
    int flag[this->V];
    Path * path = new Path[V];
    dvNode node[V];
    for(int i = 0; i < V ; i ++){
        node[i] = NULL;
        dvNode np = (dvNode)malloc(sizeof(Node));
        np->val = vex;
        np->nextNode = node[i] ;
        node[i]  = np;
    }

    for(int i = 0; i < V; i ++){
        flag[i] = 0;
        prev[i] = 0;
        dist[i] = adj[vex][i];
        path[i].route = "v"+to_string(vexs[vex]) + "-->" + "v" + to_string(vexs[i]);
        dvNode np = (dvNode)malloc(sizeof(Node));
        np->val = i;
        np->nextNode = node[i] ;
        node[i]  = np;
        
    }
    flag[vex] = 1;
    dist[vex] = 0;
    int k = 0;

    for(int i = 1; i < V; i++){
        min = INT_MAX;
        for(int j = 0; j < V; j ++){
            if(flag[j] == 0 && dist[j] < min){
                min = dist[j];
                k = j;
            }
        }
        flag[k] = 1;
        // cout << k << endl;
        for(int j = 0 ; j < V ; j ++){
            // if(adj[k][j] ==INT_MAX ){
            //     tmp = INT_MAX;
            // }
            // else{
            //     tmp = min + adj[k][j];
                
            // }

            // if(flag[j] == 0 && (tmp < dist[j]) && tmp != INT_MAX){
            // cout << "adj: " << adj[k][j] << " Int_max:" << INT_MAX << endl;
            // cout << "dist: " << dist[j] << " Int_max:" << INT_MAX << endl;
            if(flag[j] == 0 && (dist[j] >= (dist[k]+ adj[k][j])) && adj[k][j] != INT_MAX){
                int diststate = 0;
                if(dist[j] == (dist[k]+ adj[k][j]) && prev[j] <= k) diststate = 1;

                if(dist[k] != INT_MAX)
                    dist[j] = dist[k]+ adj[k][j];
                else dist[j] = INT_MAX;

                if(diststate == 0){
                    prev[j] = k;

                path[j].route = path[k].route + "-->v"+to_string(vexs[j]);
                dvNode np = (dvNode)malloc(sizeof(Node));
                np->val = j;
                
                np->nextNode = node[k] ;
                node[j]  = np;
                }
                
            }
        }
    }
    
    for(int i = 0; i < V ;i ++){
        if(dist[i] == INT_MAX) continue;

        if(nodeexists[i] == false) continue;
        // cout << "dist[" << i << "]: " << dist[i] << endl;
        dvNode np = node[i];
        stack <int> s;
        while(np ){
            // cout << vexs[np->val] << " ";
            s.push(vexs[np->val]);
            np = np->nextNode;
        }
        int cnt = 0;
        *fpOut << vexs[i]  << " ";
        
        while(s.size() > 0){
            // cout << s.top() << " ";
            if(cnt == 1){
                *fpOut << s.top()  << " ";
                break;
            }   
            s.pop();
            cnt ++;
        }
        *fpOut << dist[i] << endl;
        
        // cout << endl;
    }
    *fpOut << endl;
    // *fpOut << endl;
    // for(int i = 0;i < V;i++){
    //     if(dist[i]!=INT_MAX){
    //         cout<<path[i].route<<endl;
    //     }
    //     else{
    //         cout<<"no road"<<endl;
    //     }
    // }
}

void graph::dijkstrabet(int prev[], int dist[], ofstream* fpOut,string fileName){
    ifstream fin(fileName);
    int orig, dest;
    string msg;

    while(fin >> orig >> dest){
        getline(fin,msg);
        string hoproute = "";
        int currentpos = orig - 1;
        bool isFirst = true;
        bool brokenstate = false;
        cout << "currentpos: " << currentpos << endl;
        // while(currentpos != dest - 1){
            hoproute = hoproute + " " + to_string(currentpos + 1);
            int min;
            int tmp;
            int flag[this->V];
            Path * path = new Path[V];
            dvNode node[V];
            for(int i = 0; i < V ; i ++){
                node[i] = NULL;
                dvNode np = (dvNode)malloc(sizeof(Node));
                np->val = currentpos;
                np->nextNode = node[i] ;
                node[i]  = np;
            }

            for(int i = 0; i < V; i ++){
                flag[i] = 0;
                prev[i] = 0;
                dist[i] = adj[currentpos][i];
                path[i].route = "v"+to_string(vexs[currentpos]) + "-->" + "v" + to_string(vexs[i]);
                dvNode np = (dvNode)malloc(sizeof(Node));
                np->val = i;
                np->nextNode = node[i] ;
                node[i]  = np;
                
            }
            flag[currentpos] = 1;
            dist[currentpos] = 0;
            int k = 0;

            for(int i = 1; i < V; i++){
                min = INT_MAX;
                for(int j = 0; j < V; j ++){
                    if(flag[j] == 0 && dist[j] < min){
                        min = dist[j];
                        k = j;
                    }
                }
                flag[k] = 1;
                // cout << k << endl;
                for(int j = 0 ; j < V ; j ++){
                    // if(adj[k][j] ==INT_MAX ){
                    //     tmp = INT_MAX;
                    // }
                    // else{
                    //     tmp = min + adj[k][j];
                        
                    // }

                    // if(flag[j] == 0 && (tmp < dist[j])){
                    //     dist[j] = tmp;
                    //     prev[j] = k;

                    //     path[j].route = path[k].route + "-->v"+to_string(vexs[j]);
                    //     dvNode np = (dvNode)malloc(sizeof(Node));
                    //     np->val = j;
                        
                    //     np->nextNode = node[k] ;
                    //     node[j]  = np;
                    // }
                    if(flag[j] == 0 && (dist[j] >= (dist[k]+ adj[k][j])) && adj[k][j] != INT_MAX){
                        int diststate = 0;
                        if(dist[j] == (dist[k]+ adj[k][j]) && prev[j] <= k) diststate = 1;
                        if(dist[k] != INT_MAX)
                            dist[j] = dist[k]+ adj[k][j];
                        else dist[j] = INT_MAX;

                        if(diststate == 0){
                            prev[j] = k;

                            path[j].route = path[k].route + "-->v"+to_string(vexs[j]);
                            dvNode np = (dvNode)malloc(sizeof(Node));
                            np->val = j;
                            
                            np->nextNode = node[k] ;
                            node[j]  = np;
                        }
                        
                    }
                }
            }
            if(dist[dest - 1]!=INT_MAX && dist[dest-1] >=0){
                *fpOut << "from " << orig << " to " << dest << " cost " << dist[dest- 1] << " hops " ;
                dvNode np = node[dest - 1];
                stack <int> s;
                while(np ){
                    // cout << vexs[np->val] << " ";
                    s.push(vexs[np->val]);
                    np = np->nextNode;
                }
                int cnt = 0;
                while(s.size() > 1){
                   *fpOut << s.top()  << " ";  
                    s.pop();
                    cnt ++;
                }

                *fpOut << "message" << msg << endl;
            }
            else{
                *fpOut << "from " << orig << " to " << dest << " cost infinite hops unreachable " << "message" << msg << endl;
            }
        // }
    }
}

void graph::changeState(int prev[], int dist[], ofstream* fpOut, string changeFile, string messageFile){
    
    ifstream fin(changeFile);
    int a, b ,c;
    // cout << changeFile << endl;
    while(fin >> a >> b >> c){
        // cout << a << " " << b << " " << c << endl;
        *fpOut << endl;
        if(c < 0) c= INT_MAX;
        cout << "c: " << c << endl;
        adj[a-1][b-1] = c;
        adj[b-1][a-1] = c;
        for(int i = 0; i < V; i ++){
            
            this->dijkstra(i, prev, dist , &*fpOut);
        }
        this->dijkstrabet(prev, dist , &*fpOut, messageFile);
    }

    
}

int getNodeNum(string fileName){
    int nodeNum = 0;
    ifstream fin(fileName);

    int a, b, c;
    while(fin >> a >> b >> c){
        // cout << a << " " << b << " " << c << endl;
        if(a > nodeNum) nodeNum = a;
        else if(b> nodeNum) nodeNum = b;
    }
    cout << nodeNum << endl;
    return nodeNum;

}


int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }

    // FILE *fpOut;
    // fpOut = fopen("output.txt", "w");
    // fclose(fpOut);
    ofstream fpOut("output.txt");
    for(int i = 0 ; i < 1000; i ++){
        nodeexists[i]  = false;
    }

    int nodeNum = getNodeNum(argv[1]);
    
    graph newgraph(nodeNum, 0);
    newgraph.makeGraph(argv[1]);

    int prev[nodeNum];
    int dist[nodeNum];

    for(int i = 0; i < nodeNum; i ++){
        newgraph.dijkstra(i, prev, dist , &fpOut);
    }
    newgraph.dijkstrabet(prev, dist , &fpOut, argv[2]);

    newgraph.changeState(prev, dist, &fpOut,argv[3], argv[2]);

    return 0;
}

