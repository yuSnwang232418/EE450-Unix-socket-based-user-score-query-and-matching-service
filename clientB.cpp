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

#define LOCAL_HOST "127.0.0.1" //define local host name
#define MAXDATASIZE 500
#define CENTRAL_SERVERC_PORT "26408" //define portnumber
#define ERROR_FLAG -1

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main() {
	//Booting up
	string str;
	cin >> str;
	cout << "The client is up and running" << endl;

	int sockfd, numbytes;
	char clientInput[MAXDATASIZE];
	char buf[MAXDATASIZE];
    char final_result[MAXDATASIZE];   //Compatibility score
	char path[MAXDATASIZE];            //related path
    char clientAMsg[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
    char s[INET6_ADDRSTRLEN];

    //learn from Beej’s socket programming tutorial 
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(LOCAL_HOST,CENTRAL_SERVERC_PORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    return 1;
	}
    /**********************************************************************/
	/**************************TCP*****************************************/
	/**********************************************************************/
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == ERROR_FLAG) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == ERROR_FLAG) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

    if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

	//printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

	while(true){
	/**********************************************************************/
	//send to central server c
	/**********************************************************************/
		memset(clientInput, '\0', sizeof(clientInput));
	    strncpy(clientInput, str.c_str(), MAXDATASIZE);
        //cout << "The input is " << str << endl; //test
		
		if (send(sockfd, clientInput, sizeof(clientInput), 0) == ERROR_FLAG) {
			perror("send");
			exit(1);
		}
		memset(clientAMsg, '\0', sizeof(clientAMsg));
		if (recv(sockfd, clientAMsg, sizeof(clientAMsg), 0) == ERROR_FLAG) {
			perror("recv");
		    exit(1);
		}

		cout << "The client sent " << clientInput << " to the Central server." << endl;
	    //cout << endl;

    /**********************************************************************/
    //receive result from central server
	/**********************************************************************/
	    memset(buf, '\0', sizeof(buf));
		if (recv(sockfd, buf, sizeof(buf), 0) == ERROR_FLAG) {
			perror("recv");
		    exit(1);
		}
	    //cout << buf <<endl;
		strncpy(final_result, buf, sizeof(buf));
		//cout << final_result << endl;
		if (final_result[0] == '/'){
		// 	memset(buf, '\0', sizeof(buf));
		//     if (recv(sockfd, buf, sizeof(buf), 0) == ERROR_FLAG) {
		// 	perror("recv");
		//     exit(1);
		// }

		    cout << "Found no compatibility for " << clientInput << " and " << clientAMsg <<endl;
		//return 0;
		    break;
		}

	    memset(buf, '\0', sizeof(buf));
		if (recv(sockfd, buf, sizeof(buf), 0) == ERROR_FLAG) {
			perror("recv");
			exit(1);
		}
		
		strncpy(path, buf, sizeof(buf));
		//cout << "Response is " << path << endl; //test
		
		
		int sizep;
		for (int i = 0; i < sizeof(path);i++){
			if (path[i] == ' ') break;
		    //cout << path[i] << endl;
			sizep = i;
		}

	    char temp[MAXDATASIZE];
        int counter = 0;
        int a = 0;
        int name_size = 0;

		//format change char to string
		// path --> path1
	    string path1[MAXDATASIZE];
        memset(temp,'\0',sizeof(temp));
        //a = 0;
        //counter = 0;
        for (int i=0; i<sizeof(path);i++){
			if(path[i]==' '){
				path1[a]=temp;
                a++;
                counter = 0;
                memset(temp, 0, sizeof(temp));
            }else{
				temp[counter] = path[i];
                counter++;
            }
            path1[a]=temp;
        }
        //cout << path1[0] << endl;
	    //cout << path1[1] << endl;
	    //cout << path1[2] << endl;
	    //cout << path1[3] << endl;
        
		/*********************************************************************************/
		/**********************************output result*********************************/
        /*********************************************************************************/
	    cout << "Found compatibility for " << path1[a-1] << " and " << path1[0] << ":" << endl;
	    for (int i=a-1; i>0; i--){
			cout << path1[i] << " --- ";
	    }
	    cout << path1[0]<< endl;
	    cout << "Compatibility score: " << final_result << endl;
        break;
	}
	close(sockfd);
	return 0;

}
