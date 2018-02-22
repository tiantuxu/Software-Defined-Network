#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "message.h"
#include "graph.h"
#include <ctype.h>
#include <stdbool.h>//
#include <errno.h>//
#include <sys/time.h>
#include <netinet/in.h>
#include <vector>
#define K 5
#define M 3

using namespace std;
#define MAXLINE 100
//#define SWITCH_NUM 6                    ************* Subject to change**********/

int SWITCH_NUM;

/**********Input Variable Declaration************/

	struct message2;
	int switch_id;
	char *controller_hostname;
	int  controller_port;
	int failed_neighbour=-1;        //= 101;
	int verbosity = 0; // 0 for normal 1 for high
	int switch_socket_id;

/**********************************************/


void print_routing(array<int,1000> route_table);

/********************/

int main(int argc , char **argv)
{

    SWITCH_NUM = 1000;
    //cout << SWITCH_NUM << endl;
	//----variable for tracking active switch-------//
	int current_alive[SWITCH_NUM+1];			//link that is alive this round
	int previous_alive[SWITCH_NUM+1];		//link that is alive last round ie M*K seconds before
	int current_not_alive[SWITCH_NUM+1];
	memset(current_alive, 0, sizeof(current_alive[0])*(SWITCH_NUM+1));
	memset(previous_alive, 0, sizeof(previous_alive[0])*(SWITCH_NUM+1));
	memset(current_not_alive, 0, sizeof(current_not_alive[0])*(SWITCH_NUM+1));
	//****************DEAD SWITCH LINK***********//
	int dead_link[SWITCH_NUM+1];
	memset(dead_link, 0, sizeof(dead_link[0]) * SWITCH_NUM+1);
	//*******************************************//
	
	int counter = 0;
	bool inactive = false;

	
	//--------------address assignment------------//
	struct sockaddr_in switch_addr, neighbour_addr,serv_addr;
	bzero((char *) &switch_addr, sizeof(switch_addr));
	bzero((char *) &serv_addr, sizeof(serv_addr));
	bzero((char *) &neighbour_addr, sizeof(neighbour_addr));
	int slen=sizeof(switch_addr),slen2=sizeof(neighbour_addr),slen3=sizeof(serv_addr);
	
/* 	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 5; */
	
	map<int,struct sockaddr_in> active_neighbor_addr;   //<neighbor_id,neighbor_addr>
	char *haddrp;
	struct hostent *hp;
	struct hostent *hp1;
	
	
	//** normal mode:./switch switch_id controller_hostname controller_port*//
	if(argc==4)
	{
		switch_id = atoi(argv[1]);
		controller_hostname = argv[2];
		controller_port = atoi(argv[3]);
		cout<<"switchID: "<<switch_id<<"controller hostname "<<controller_hostname<<"controller port "<<controller_port<<endl;
	}
	
	/**normal mode with higher verbosity:./switch switch_id controller_hostname controller_port verbosity**/
	if(argc==5)
	{
		switch_id = atoi(argv[1]);
		controller_hostname = argv[2];
		controller_port = atoi(argv[3]);
		verbosity = atoi(argv[4]);
		cout<<"switchID: "<<switch_id<<"controller hostname "<<controller_hostname<<"controller port "<<controller_port<<"verbosity level"<<verbosity<<endl;
		
	}
	
	/** failure mode:./switch switch_id controller_hostname controller_port -f neighbor_id*/
	if (argc == 6)
	{
		switch_id = atoi(argv[1]);
		controller_hostname = argv[2];
		controller_port = atoi(argv[3]);
		string flag = argv[4];
		failed_neighbour = atoi(argv[5]);

		/* buffer[0] = *argv[1];   // This is the switch ID
		id = switch_id;
		ids[0] = *argv[1];  // Change ID here too	 */
	 	cout<<"switchID: "<<switch_id<<"controller hostname ";
		cout<<controller_hostname<<"controller port "<<controller_port<<"failed neighbor "<<failed_neighbour<<endl;
	  
	}   
   /** failure mode with high verbosity:./switch switch_id controller_hostname controller_port -f neighbor_id verbosity*/
	if (argc == 7)
   {
		switch_id = atoi(argv[1]);
		controller_hostname = argv[2];
		controller_port = atoi(argv[3]);
		string flag = argv[4];
		failed_neighbour = atoi(argv[5]);
		verbosity = atoi(argv[6]);
		cout<<"switchID:"<<switch_id<<"controller hostname ";
		cout<<controller_hostname<<"controller port "<<controller_port<<"failed neighbor "<<failed_neighbour<<"verbosity level"<<verbosity<<endl;
   }
	if ((switch_socket_id = socket(AF_INET, SOCK_DGRAM,0)) == -1)
   {
      perror("socket failed");
      exit(EXIT_FAILURE);
   }
   
	
/* 	switch_addr.sin_family = AF_INET;
	switch_addr.sin_port = htons(controller_port);
	switch_addr.sin_addr.s_addr =inet_addr("127.1.9.1"); */
	
	if ((hp = gethostbyname(controller_hostname)) == NULL)
		return -1; /* check h_errno for cause of error */
	bzero((char *) &serv_addr, sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr,(char *)&serv_addr.sin_addr.s_addr, hp->h_length); 
	serv_addr.sin_port = htons(controller_port);
	//serv_addr.sin_addr.s_addr =inet_addr("127.0.0.1");

	
//	setsockopt(switch_socket_id, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
   


	// Switch Registering Process 
	message2 *msg = static_cast<message2 *>(malloc(sizeof(message2)));
	message2 *msg_rev = static_cast<message2 *>(malloc(sizeof(message2)));

	msg->type=REGISTER_REQUEST;
	msg->id=switch_id;
	sendto(switch_socket_id, msg, sizeof(message2) , 0 , (struct sockaddr *) &serv_addr, slen3);
	cout<<"REGISTER_REQUEST is sent from SWITCH "<<switch_id<<endl;
	
	
	recvfrom(switch_socket_id, msg_rev, sizeof(message2), 0, (struct sockaddr *) &serv_addr, (socklen_t*)&slen3);
	
	SWITCH_NUM = msg_rev->SWITCH_NUM;
	//--------------Intialize Previous Alive-----------------------------//
	for(auto link:msg_rev->neighbor) previous_alive[link]=1;
	//------------------------------------------------------------------//
	cout<<"REGISTER_RESPONSE is received from the controller,active neighbor of "<<switch_id<<"is: "<<endl;
	
	
	//-------------log active_neighbor_addr information------------------------------  //
    array<int, 1000> active_neighbor;
    active_neighbor=msg_rev->active;    //active neighbor map
	for(auto neighbor_id:(msg_rev->neighbor))
	{
		if(active_neighbor[neighbor_id]==1)   //is neighbor and is alive
		{
			current_alive[neighbor_id]=1;   //Initialize current_alive
			active_neighbor_addr[neighbor_id]=(msg_rev->addr_table)[neighbor_id];    //----<>
//-----	--- sin_addr.s_addr =inet_addr("127.0.0.1"); ----------------------------------//
		}
	}
	
	
	//---------------check address received------------------------------------------//
	for(auto m:active_neighbor_addr)
	{
		neighbour_addr=m.second;
		hp1 = gethostbyaddr((const char *)&neighbour_addr.sin_addr.s_addr,sizeof(neighbour_addr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(neighbour_addr.sin_addr);	
		cout<<"switch"<<m.first<<" hp " << hp1->h_name << " haddrp "<< haddrp << " port " << ntohs(neighbour_addr.sin_port) << endl; 
	}
		
	//--------------print information got from controller---------------------------//
	
	
	//--Send Initial KEEP ALIVE messages after learning about the active neighbour_addrs-//
	for(auto m:active_neighbor_addr)
	{
		if(m.first==failed_neighbour) continue; //don't send keep alive to through failed link
		bzero(msg,sizeof(message2));
		neighbour_addr=m.second;
		msg->id=switch_id;
		msg->type=KEEP_ALIVE;
		if(verbosity == 1) cout<<"send keep alive from "<<switch_id<<"to "<<m.first<<endl;
		sendto(switch_socket_id, msg, sizeof(message2) , 0 , (struct sockaddr *) &neighbour_addr, slen2);
	}
	
	
	//--------------Select Process-------------------------------------//
	struct timeval selTimeout;
	selTimeout.tv_sec = K;              /* timeout (secs.) */
	selTimeout.tv_usec = 0;            /* 0 microseconds */
	fd_set readfd;
	int ready;
	bool checker = false;

	
	while(1)
	{
		
	if(checker)
    {
       selTimeout.tv_sec = K;       /* timeout (secs.) */
       selTimeout.tv_usec = 0;            /* 0 microseconds */
       checker = false;
    }
	FD_ZERO(&readfd);
	FD_SET(switch_socket_id, &readfd);
	ready = select(switch_socket_id+1, &readfd, NULL, NULL, &selTimeout);
	//cout << "ready = " << ready << endl;

	
	if ((ready < 0) && (errno != EINTR))
	{
	 printf("select error");
	}
	
	if(ready > 0 && FD_ISSET(switch_socket_id, &readfd))
	{
		bzero(msg,sizeof(message2));
		bzero(msg_rev,sizeof(message2));
		if(verbosity == 1) cout<<"A message is received on switch "<<switch_id;
		
		//-----------error handling------------------------------------------//
		if (recvfrom(switch_socket_id, msg_rev, sizeof(message2), 0, (struct sockaddr *) &switch_addr, (socklen_t*)&slen) == -1)
		{
		perror("RecvFrom failed");
		exit(EXIT_FAILURE);
		}
		
		//--------------------------------------------------------------------//
		if(verbosity == 1) cout<<"Type of message received is "<<msg_rev->type<<endl;
		
		if(msg_rev->type==KEEP_ALIVE||msg_rev->id!=0)
		{
			if(msg_rev->id!=failed_neighbour)	//The switch should not process any message from the neighbor through failed link
			{
				//-------------Setting the status of received switches----------------//
				current_alive[msg_rev->id]=1;     //Initialize current_alive
				active_neighbor[msg_rev->id]=1;
//				if(active_neighbor_addr.find(msg_rev->id)==active_neighbor_addr.end())
//				{
	//--------	 sin_addr.s_addr =inet_addr("127.0.0.1"); ----------------------------------//
					active_neighbor_addr[msg_rev->id]=switch_addr;
//				}
				
				if(verbosity==1)
					cout<<"Keep alive received from "<<msg_rev->id<<inet_ntoa(switch_addr.sin_addr)<<":"<<ntohs(switch_addr.sin_port)<<endl;
				
				//-------------UNREACHEABLE now REACHABLE again PRINTING---------------//
				
				for(int y1 = 1; y1 <=SWITCH_NUM; ++y1)
				{
					if(dead_link[y1] == 1 && msg_rev->id == y1 )     //This will only be true when all links have registered
					{
						cout << "----------------------------------------" << endl;
						cout<<"Neighbour switch "<<msg_rev->id<<" is reachable again"<<endl;
						cout << "----------------------------------------" << endl;

						dead_link[y1] = 0;
						
						msg->id=switch_id;
						msg->type=TOPOLOGY_UPDATE;
						for(auto m:active_neighbor_addr)
							msg->active[m.first]=1;                 //assign active neighbors 
						sendto(switch_socket_id, msg, sizeof(message2) , 0 , (struct sockaddr *) &serv_addr, slen3); //**Send Update Immediately
					}            

				}
			}			
		}
		
		
		
		
		if(msg_rev->type==ROUTE_UPDATE||msg_rev->id==0)
		{
			cout<< "switch "<< atoi(argv[1]) << " received ROUTE_UPDATE from controller"<<endl;
			print_routing(msg_rev->route_table);
		}
    }
		// -------Timer Expired After K seconds-----//
		if(ready == 0)  
		{
			checker = true;
			if(verbosity == 1) printf("Timer Expired After K seconds\n");
			counter++;		
				
			if(counter==M) //----------------	Reach M*K seconds----------//
			{
				for(int y1 = 1; y1 <=SWITCH_NUM; ++y1)
				{
					if(current_alive[y1]==0&&previous_alive[y1]==1)
					{
						current_not_alive[y1] = 1;    //It means this link is not alive anymore after M*K seconds     
						inactive = true;
						dead_link[y1]=1;
						active_neighbor_addr.erase(y1);					
					}

				}
			if(inactive)
			{
				cout << "----------------------------------------" << endl;
				cout<<"For SWITCH "<< switch_id <<" current unreachable NEIGHBORS are "<< endl;


				for(int y1 = 1; y1 <= SWITCH_NUM; ++y1)
				{
					if(current_not_alive[y1] == 1) cout<< y1 <<" ";
				}
				cout<<endl;
				cout << "----------------------------------------" << endl;
			}
			
			counter = 0; 
			//-------------------Reset current_alive previous_alive----------------//
			for(int y1 = 1; y1 <=SWITCH_NUM; ++y1)
			{
				inactive = false;            
				current_not_alive[y1] = 0;
				previous_alive[y1]=current_alive[y1];
				current_alive[y1]=0;
				
			}
				
			}
			
		   //-------------Send Topology Update Periodically-------------------------------------------//

			if(verbosity==1) cout<<"Now send TOPOLOGY_UPDATE"<<endl;
			bzero(msg,sizeof(message2));
			msg->id=switch_id;
			msg->type=TOPOLOGY_UPDATE;
			for(auto m:active_neighbor_addr){
				msg->active[m.first]=1;
                //cout << m.first << endl;
                //cout << msg->active[m.first] << endl;
            }
			sendto(switch_socket_id,msg,sizeof(message2),0,(struct sockaddr *) &serv_addr,slen3);      //assign active neighbors
            
			if(verbosity == 1)	cout<<"TOPOLOGY Messages sent from"<< switch_id<<endl;
			
			
			//-------------Send KEEP ALIVE Periodically-------------------------------------------//
			bzero(msg,sizeof(message2));
			msg->id=switch_id;
			msg->type=KEEP_ALIVE;
			for(auto m:active_neighbor_addr)
			{
				if(m.first==failed_neighbour) continue; //don't send keep alive to through failed link
				neighbour_addr=m.second;
				sendto(switch_socket_id, msg, sizeof(message2) , 0 , (struct sockaddr *) &neighbour_addr, slen2);
				
				if(verbosity == 1) cout<<"send keep alive from "<<switch_id<<" to "<<m.first<<endl;
			}	
		}

	}
	close(switch_socket_id);
	return 0;
}



void print_routing(array<int, 1000> route_table)
{
	//cout << "SWITCH_NUM = " << SWITCH_NUM << endl;
	for(int y1=1;y1<=SWITCH_NUM;y1++)
	{
		if(y1==switch_id) continue;
		cout<<"To destination "<<y1<<" Next hop is "<<route_table[y1]<<" "<<endl;
		//cout << "here" << endl;
	}
}
		
		
		














































