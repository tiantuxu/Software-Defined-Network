//
// Created by teddyxu on 1/21/18.
//

#ifndef ECE50863_PROJECT1_MESSAGE_H
#define ECE50863_PROJECT1_MESSAGE_H
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <array>
#include <tuple>
#include "graph.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>


#define REGISTER_REQUEST 1
#define REGISTER_RESPONSE 2
#define KEEP_ALIVE 3
#define ROUTE_UPDATE 4
#define TOPOLOGY_UPDATE 5

//#define SWITCH_NUM 6
extern int SWITCH_NUM;
/*
typedef struct message{
    int id;
    int type;
    std::map<int,int> active;
    std::vector<int> neighbor;
    std::map<int, struct sockaddr_in> addr_table;
    std::map<int, path> route_table;
} message;
*/
/*
typedef struct message{
    int id;
    int type;
    std::array<int, SWITCH_NUM + 1> active;
    std::array<int, SWITCH_NUM + 1> neighbor;
    std::array<struct sockaddr_in, SWITCH_NUM + 1> addr_table;
    std::array<int,SWITCH_NUM + 1> route_table;

} message;
*/

typedef struct message2{
    int id;
    int type;
    int SWITCH_NUM;
    std::array<int, 1000> active;
    std::array<int, 1000> neighbor;
    std::array<struct sockaddr_in, 1000> addr_table;
    std::array<int, 1000> route_table;

} message2;

#endif //ECE50863_PROJECT1_MESSAGE_H
