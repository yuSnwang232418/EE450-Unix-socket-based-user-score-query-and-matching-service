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
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h>

using namespace std;

#define SERVER_S_PORT "22408" //udp
#define CENTRAL_UDP_PORT "24408"
#define LOCAL_HOST "127.0.0.1" //define local host name
#define MAXDATASIZE 500

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;
int numbytes;
//struct sockaddr* their_addr;
struct sockaddr_storage their_addr;
socklen_t addr_len;
char buf[MAXDATASIZE];
char allnames[MAXDATASIZE];
char s[INET6_ADDRSTRLEN];


void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/********************************************************************************************/
//create UDP
//learn from Beej’s socket programming tutorial 
/********************************************************************************************/

void createUDP(){
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVER_S_PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		//return 1;
		exit(1);
	}
	
	//create UDP socket and bind
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
			continue;
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("listener:bind");
			continue;
		}
		break;
	}
	if(p==NULL){
		fprintf(stderr,"talker:failed to create socket\n");
		//return 2;
		exit(1);
	}
	cout << "The ServerS is up and running using UDP on port " << SERVER_S_PORT << "." <<endl;

}

/********************************************************************************************/
//rec request from C
/********************************************************************************************/
void recvfromC(){
	if((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
	//string msg = buf;
	//cout << buf <<endl;
	//return msg;
	strncpy(allnames, buf, sizeof(buf));
}


int main(){
	
    createUDP();
	addr_len = sizeof their_addr;
  
	while(true){
		//receive request from C
		recvfromC();
		cout << "The serverS received a request from Central to get the scores." << endl;
		//string msgA,msgB;
		/*****************************************************************************/
		/************************read score.txt**************************************/
		/***************************************************************************/
		//char allnames[MAXDATASIZE] = "Rachael King Hanieh";
        //cout << allnames << endl;
        //cout << sizeof(allnames) << endl;

        string input[MAXDATASIZE][2];
		ifstream myfile("scores.txt");
		if (!myfile.is_open()){
			cout << "can not open file" << endl;
			return 0;
		}
		int columns = 2; 
		int rows = 0;
		for (int i = 0; i < MAXDATASIZE; i++){
			for (int j = 0; j <= 1; j++){
            myfile >> input[i][j];
			}
		}
        //test
        // cout << input[0][0] <<endl;
        // cout << input[0][2] <<endl;
        // cout << input[5][1] << endl;
        // cout << input[6][0] << endl;
        /***********************************************************/
        //get the rows of input
        /***********************************************************/
		for (int i = 0; i < MAXDATASIZE; i++){
			if (input[i][0] == "") break;
			else rows++;
		}
        
        //cout << rows << endl;
        /***********************************************************/
        //input char -->string
        char temp[MAXDATASIZE];
        int counter = 0;
        int a = 0;
        string names[MAXDATASIZE];
        for (int i=0; i<sizeof(allnames);i++){
            if(allnames[i]==' '){
                //cout << "next" << i << endl;
                names[a]=temp;
                a++;
                counter = 0;
                memset(temp, 0, sizeof(temp));
            }else{
                temp[counter] = allnames[i];
                //cout << allnames[i];
                //cout << temp << endl;
                counter++;
            }
             names[a]=temp;
        }
        //cout << names[0] <<endl;
        //cout << names[1] <<endl;
        //cout << names[2] <<endl;
        // cout << names[3] <<endl;
        // cout << names[4] <<endl

        //get scores
        string scores[MAXDATASIZE];
        for(int i=0;i <=a; i++ ){
            for(int j=0; j < rows;j++){
                if(names[i] == input[j][0]){
                    scores[i]=input[j][1];
                    break;
                }

            }
        }
        //cout<< scores[0] << endl;
        //cout<< scores[1] << endl;
        //cout<< scores[2] << endl;
        
        //change format
        string res;
        for (int i = 0;i <= a;i++) {
            res += scores[i];
            res += " ";
        }
        //cout << res << endl;
        char dataBuff[MAXDATASIZE];
        memset(dataBuff,'\0',sizeof(dataBuff));
        strncpy(dataBuff, res.c_str(), res.length() + 1);
        //cout << dataBuff << endl;


		//send to C 
		if ((numbytes = sendto(sockfd, dataBuff, sizeof(dataBuff), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
			perror("talker: sendto");
			exit(1);
		}
		cout << "The serverS finished sending the scores to Central." << endl;
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}