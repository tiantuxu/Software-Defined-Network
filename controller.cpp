#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <iostream>
#include <fstream>
#include "message.h"
#include "graph.h"
#include <sys/time.h>

#define LISTENQ 10
#define MAXLINE 100

int SWITCH_NUM;
using namespace std;
struct sockaddr_in serveraddr;
struct sockaddr_in clientaddr;
struct hostent *hp;

//struct message;
int flag_register = 1;

std::map<int, vector<int>> neighbor_maps;
std::array<struct sockaddr_in, 1000> switch_addr;
std::array<int, 1000> is_active;

int open_listenfd(int port)
{ 
  int listenfd, optval=1; 
  //struct sockaddr_in serveraddr;
   
  /* Create a socket descriptor */ 
  if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    return -1; 
  
  /* Eliminates "Address already in use" error from bind. */ 
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  
		 (const void *)&optval , sizeof(int)) < 0) 
    return -1; 
 
  /* Listenfd will be an endpoint for all requests to port 
     on any IP address for this host */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET;  
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
  serveraddr.sin_port = htons((unsigned short)port);  
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
 
  /* Make it a listening socket ready to accept 
     connection requests */ 
/*
  if (listen(listenfd, LISTENQ) < 0)
    return -1; 
*/
  return listenfd; 
}

int check_registration(map<int, int>register_map){
    int bin;
    map<int, int>::iterator it;

    if(flag_register == 1){
        bin = 1;
        for (it = register_map.begin(); it != register_map.end(); ++it ) {
            bin = bin * it->second;
        }

        if (bin == 1) {
            flag_register = 0;
            cout << "All of the switches has been registered!" << endl;
            return 1;
        }
        else{
            return 0;
        }
    }
    return 0;
}

std::array<int, 1000> get_route_table(graph &g, int switch_id) {
    std::array<int, 1000> route_table;
    int j;
    vector<int> route;

    for (j = 1; j <= SWITCH_NUM; j++) {
        //cout << switch_id << endl;
        if((j != switch_id) && (is_active[j] == 1)) {
            result result1 = dijkstra(g, switch_id, j);
            //std::cout << "widest path from " << switch_id << " to " << j << " is " << result1.distance << endl;
            /*
            for (int x:result1.p) {
                std::cout << x << " ";
            }
            cout << endl;
            */
            //cout << "destination = " << j << " hop = " << result1.p[1] << endl;
            route_table[j] = result1.p[1];
        }
        else{
            route_table[j] = -1;
        }
    }
    //cout << route_table.size() << endl;
    return route_table;
}

graph addedge(graph &g, string fname){
    std::ifstream infile;
    infile.open(fname);

    int a,b,c,d;

    if(!infile) {
        cout << "Cannot open topology file!\n";
    }

    infile >> SWITCH_NUM;

    std::cout << "SWITCH_NUM = " << SWITCH_NUM << std::endl;

    while (!infile.eof()){
        infile >> a >> b >> c >> d;
        neighbor_maps[a].push_back(b);
        neighbor_maps[b].push_back(a);
        //g.addEdge(a, b, c);
        //cout << a << b << c << d << endl;
        add_edge(g,a,b,c);
    }

    return g;
}

graph update_topology(vector<int> is_dead, string fname){
    std::ifstream infile;
    infile.open(fname);
    graph g;

    int a,b,c,d;

    if(!infile) {
        cout << "Cannot open topology file!\n";
    }

    infile >> a;

    while (!infile.eof()){
        int flag = 0;
        infile >> a >> b >> c >> d;
        for (std::vector<int>::iterator it = is_dead.begin() ; it != is_dead.end(); ++it) {
            if(a == *it || b == *it){
                flag = 1;
            }
        }
        if(flag == 0){
            add_edge(g,a,b,c);
        }
    }
    return g;
}

graph rm_link(int start, int finish, string fname){
    std::ifstream infile;
    infile.open(fname);
    graph g;

    int a,b,c,d;

    if(!infile) {
        cout << "Cannot open topology file!\n";
    }

    infile >> a;

    while (!infile.eof()){
        int flag = 0;
        infile >> a >> b >> c >> d;

        if((a == start && b == finish) || (a == finish && b == start)){
            flag = 1;
            continue;
        }
        if(flag == 0){
            add_edge(g,a,b,c);
            //cout << "a = " << a << " b = " << b << endl;
        }
    }
    return g;
}

vector<int> check_dead(std::array<int, 1000> is_active){
    vector<int> dead;
    //map<int, int>::iterator iter;
    int i;

    for (i=1; i<=SWITCH_NUM; ++i ) {
        if (is_active[i] == 0){
            cout << "Switch ID " << i << "is dead" << endl;
            dead.push_back(i);
        }
    }
    return dead;
}

int main(int argc, char **argv) {
    int listenfd, port;
    //struct sockaddr_in clientaddr;
    //struct hostent *hp;
    char *haddrp;
    int bytes_received;
    int bytes_sent;
    //char buf[MAXLINE];
    int clientlen;
    int j;
    vector<int> is_dead;
    int action_needed = 0;

    fd_set rfds;
    struct timeval tv,res;
    struct timeval tv0;
    tv0.tv_sec = 15;
    tv0.tv_usec = 0;

    int retval;

    map<int, int> register_map;
    map<int, struct timeval> timeout;
    map<int, struct timeval>::iterator time_iter;
    //vector<int>::iterator neighbor_iter;

    //message *msg = new message();
    //message *msg_send = new message();

    port = atoi(argv[1]); /* the server listens on a port passed on the command line */
    string fname = argv[2];

    std::ifstream infile;
    infile.open(fname);
    infile >>SWITCH_NUM;
    infile.close();


    graph g;
    g = addedge(g,fname);

    for(int i = 1; i <= SWITCH_NUM; i++){
        register_map[i] = 0;
    }

    listenfd = open_listenfd(port);
    clientlen = sizeof(clientaddr);

    message2 *msg = static_cast<message2 *>(malloc(sizeof(struct message2)));
    message2 *msg_send = static_cast<message2 *>(malloc(sizeof(struct message2)));
    
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(listenfd, &rfds);

        tv.tv_sec = 15;
        tv.tv_usec = 0;

        /* Check if all switches has been registered */
        if(check_registration(register_map)) {
            cout << "All switches has been registered, start sending ROUTE_UPDATE message" << endl;
            for (j = 1; j <= SWITCH_NUM; j++) {
                msg_send->id = 0;
                msg_send->type = ROUTE_UPDATE;
                msg_send->route_table = get_route_table(g, j);
                msg_send->SWITCH_NUM = SWITCH_NUM;

                hp = gethostbyaddr((const char *)&switch_addr[j].sin_addr.s_addr,
                                   sizeof(switch_addr[j].sin_addr.s_addr), AF_INET);
                haddrp = inet_ntoa(switch_addr[j].sin_addr);

                //std::cout << "hp " << hp->h_name << " haddrp " << haddrp << " port " << ntohs(switch_addr[j].sin_port) << std::endl;

                bytes_sent = sendto(listenfd, msg_send, sizeof(*msg_send), 0, (struct sockaddr *) &switch_addr[j], sizeof(switch_addr[j]));
                if (bytes_sent < 0){
                    perror("sending error!\n");
                }
            }
        }

        retval = select(listenfd + 1, &rfds, NULL, NULL, &tv);
        //cout << retval << endl;
        if (retval < 0){
            return -1;
        }

        if(FD_ISSET(listenfd, &rfds)){
            //cout << "controller is ready to read" << endl;
            bytes_received = recvfrom(listenfd, msg, sizeof(message2), 0, (struct sockaddr *) &clientaddr, (socklen_t*) &clientlen);
            if (bytes_received < 0){
                perror("receiving error!\n");
            }

            switch(msg->type) {
                case REGISTER_REQUEST:
                    std::cout << "REGISTER_REQUEST received from switch " << msg->id << std::endl;
                    //connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
                    register_map[msg->id] = 1;
                    is_active[msg->id] = 1;
                    switch_addr[msg->id] = clientaddr;
                    timeout[msg->id] = tv0;

                    msg_send->id = 0;
                    msg_send->SWITCH_NUM = SWITCH_NUM;
                    msg_send->type = REGISTER_RESPONSE;
                    for(j = 0; j <= SWITCH_NUM; j++){
                        msg_send->neighbor[j] = 0;
                    }

                    for (j = 1; j <= neighbor_maps[msg->id].size(); j++){
                        msg_send->neighbor[j] = neighbor_maps[msg->id][j-1];
                    }

                    msg_send->addr_table = switch_addr;
                    msg_send->active = is_active;

                    //cout << "sizeof" << sizeof(msg_send) << endl;

                    hp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
                                   sizeof(clientaddr.sin_addr.s_addr), AF_INET);
                    haddrp = inet_ntoa(clientaddr.sin_addr);

                    std::cout << "hp " << hp->h_name << " haddrp " << haddrp << " port " << ntohs(clientaddr.sin_port) << std::endl;

                    bytes_sent = sendto(listenfd, msg_send, sizeof(*msg_send), 0, (struct sockaddr *) &clientaddr, clientlen);
                    if (bytes_sent < 0) {
                        perror("sending error!\n");
                    }
                    std::cout << "REGISTER_RESPONSE sent from controller " << msg_send->id << std::endl;
                    cout << endl;

                    if(flag_register == 0){
                        flag_register = 1;
                        g = addedge(g,fname);
                        std::cout << "Sending ROTUE_UPDATE becasue switch " << msg->id << " is BACK_ONLINE!"<< std::endl;
                        for (j = 1; j <= SWITCH_NUM; j++) {
                            if (is_active[j] == 1) {
                                cout << "sending ROUTE_UPDATE to switch " << j << endl;

                                msg_send->id = 0;
                                msg_send->type = ROUTE_UPDATE;
                                msg_send->active = is_active;
                                msg_send->route_table = get_route_table(g, j);
                                msg_send->SWITCH_NUM = SWITCH_NUM;

                                hp = gethostbyaddr((const char *) &switch_addr[j].sin_addr.s_addr,
                                                   sizeof(switch_addr[j].sin_addr.s_addr), AF_INET);
                                haddrp = inet_ntoa(switch_addr[j].sin_addr);

                                //std::cout << "hp " << hp->h_name << " haddrp " << haddrp << " port "
                                //<< ntohs(switch_addr[j].sin_port) << std::endl;

                                bytes_sent = sendto(listenfd, msg_send, sizeof(*msg_send), 0,
                                                    (struct sockaddr *) &switch_addr[j], sizeof(switch_addr[j]));

                                if (bytes_sent < 0) {
                                    perror("sending error!\n");
                                }
                            }
                        }
                    }

                    break;
                case TOPOLOGY_UPDATE:
                    std::cout << "TOPOLOGY_UPDATE received from switch " << msg->id << std::endl;
                    timeout[msg->id] = tv0;

                    for(auto it:neighbor_maps[msg->id]){
                        //cout << it << endl;
                        //cout << msg->active[it] << endl;
                        //cout << endl;
                        if(it * (msg->active[it]) == 0){
                            cout << "----------------------------------------" << endl;
                            cout << "The link between " << msg->id << " and " << it <<" is dead" << endl;
                            cout << "----------------------------------------" << endl;

                            //action_needed = 1 - action_needed;

                            g = rm_link(msg->id, it, fname);
                            for (j = 1; j <= SWITCH_NUM; j++) {
                                if (is_active[j] == 1) {
                                    cout << "sending ROUTE_UPDATE to switch " << j << endl;

                                    msg_send->id = 0;
                                    msg_send->type = ROUTE_UPDATE;
                                    msg_send->active = is_active;
                                    msg_send->route_table = get_route_table(g, j);
                                    msg_send->SWITCH_NUM = SWITCH_NUM;

                                    hp = gethostbyaddr((const char *) &switch_addr[j].sin_addr.s_addr,
                                                       sizeof(switch_addr[j].sin_addr.s_addr), AF_INET);
                                    haddrp = inet_ntoa(switch_addr[j].sin_addr);

                                    //std::cout << "hp " << hp->h_name << " haddrp " << haddrp << " port "
                                              //<< ntohs(switch_addr[j].sin_port) << std::endl;

                                    bytes_sent = sendto(listenfd, msg_send, sizeof(*msg_send), 0,
                                                        (struct sockaddr *) &switch_addr[j], sizeof(switch_addr[j]));

                                    if (bytes_sent < 0) {
                                        perror("sending error!\n");
                                    }
                                }
                            }
                        }
                    }
                    is_active[msg->id] = 1;

                    /* Update the timeout map */
                    for (time_iter = timeout.begin(); time_iter != timeout.end(); ++time_iter ) {
                        if (time_iter->first != msg->id){
                            //cout << time_iter->second.tv_sec << endl;
                            time_iter->second.tv_sec = time_iter->second.tv_sec - (15-tv.tv_sec);
                            //cout << time_iter->second.tv_sec << endl;
                            if(time_iter->second.tv_sec < 0){
                                is_active[time_iter->first] = 0;
                            }
                        }
                    }

                    is_dead = check_dead(is_active);
                    if(!is_dead.empty()){
                        for (std::vector<int>::iterator it = is_dead.begin() ; it != is_dead.end(); ++it) {
                            cout << "----------------------------------------" << endl;
                            cout << "sending ROUTE_UPDATE to all switches because switch ID " << *it << "is dead" << endl;
                            cout << "----------------------------------------" << endl;
                        }

                        g = update_topology(is_dead, fname);

                        for (j = 1; j <= SWITCH_NUM; j++) {
                            if (is_active[j] == 1) {
                                cout << "sending ROUTE_UPDATE to switch " << j << endl;

                                msg_send->id = 0;
                                msg_send->type = ROUTE_UPDATE;
                                msg_send->active = is_active;
                                msg_send->route_table = get_route_table(g, j);
                                msg_send->SWITCH_NUM = SWITCH_NUM;

                                hp = gethostbyaddr((const char *) &switch_addr[j].sin_addr.s_addr,
                                                   sizeof(switch_addr[j].sin_addr.s_addr), AF_INET);
                                haddrp = inet_ntoa(switch_addr[j].sin_addr);

                                //std::cout << "hp " << hp->h_name << " haddrp " << haddrp << " port "
                                          //<< ntohs(switch_addr[j].sin_port) << std::endl;

                                bytes_sent = sendto(listenfd, msg_send, sizeof(*msg_send), 0,
                                                    (struct sockaddr *) &switch_addr[j], sizeof(switch_addr[j]));

                                if (bytes_sent < 0) {
                                    perror("sending error!\n");
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        else{
            cout << "No data within 15 seconds" << endl;
        }

    }
}
