#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>

#define LOCAL_HOST "127.0.0.1" //define local host name
#define MAXDATASIZE 500
#define SERVER_T_PORT "21408" //UDP
#define SERVER_S_PORT "22408"
#define SERVER_P_PORT "23408"
#define CENTRAL_UDP_PORT "24408"
#define CENTRAL_A_PORT "25408" //TCP
#define CENTRAL_B_PORT "26408"
#define BACKLOG 10 //pending connection queue
using namespace std;

//UDP
int sockfd_UDP;
struct addrinfo hints_UDP, *servinfo_UDP,*p_UDP,*serverTinfo,*serverSinfo,*serverPinfo;
int rv_UDP;
int numbytes_UDP;
struct sockaddr_storage their_addr_UDP;
socklen_t addr_len_UDP;
//socklen_t addr_len;

//TCP
int sockfd_TCP, new_fd_TCP,sockfd_TCP_B, new_fd_TCP_B; 
struct addrinfo hints_TCP, *servinfo_TCP, *p_TCP,hints_TCP_B, *servinfo_TCP_B, *p_TCP_B;
struct sockaddr_storage their_addr; 
socklen_t add_size;
struct sigaction sa;
int yes = 1;
char s[INET6_ADDRSTRLEN];
int rv,rv_B;
struct sockaddr_storage their_addr_B; 
//socklen_t sin_size_B;
//char msg_C[100];
char dataBuff[MAXDATASIZE];
char allnames[MAXDATASIZE];    //allnames in the paths
char dataBuff2[MAXDATASIZE][MAXDATASIZE];  //all paths
char scores[MAXDATASIZE];     //scores related to allnames
char final_char_sum[MAXDATASIZE];  //final Compatibility score
int result_size;
// char clientAMsg[100];
// char clientBMsg[100];
char clientAMsg[MAXDATASIZE];
char clientBMsg[MAXDATASIZE];

void sigchld_handler(int s)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/********************************************************************************************/
//create UDP and bind
//learn from Beej’s socket programming tutorial 
/********************************************************************************************/
void createUDP(){
	memset(&hints_UDP, 0, sizeof hints_UDP);
	hints_UDP.ai_family = AF_UNSPEC;
	hints_UDP.ai_socktype = SOCK_DGRAM;
	if ((rv_UDP = getaddrinfo(LOCAL_HOST, CENTRAL_UDP_PORT, &hints_UDP, &servinfo_UDP)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDP));
		//return 1;
		exit(1);
	}

	for(p_UDP = servinfo_UDP; p_UDP != NULL; p_UDP = p_UDP->ai_next){
		if((sockfd_UDP = socket(p_UDP->ai_family,p_UDP->ai_socktype,p_UDP->ai_protocol))==-1){
			continue;
		}
		if(bind(sockfd_UDP, p_UDP->ai_addr, p_UDP->ai_addrlen) == -1){
			close(sockfd_UDP);
			perror("listener:bind");
			continue;
		}
		break;
	}
	if(p_UDP==NULL){
		fprintf(stderr,"listener:failed to bind socket\n");
		//return 2;
		exit(1);
	}
	addr_len_UDP = sizeof their_addr_UDP;
} 
/********************************************************************************************/
//create TCP_A
/********************************************************************************************/
void createTCPA(){

    memset(&hints_TCP, 0, sizeof hints_TCP);
	hints_TCP.ai_family = AF_UNSPEC;
	hints_TCP.ai_socktype = SOCK_STREAM;
	//hints_TCPA.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(LOCAL_HOST, CENTRAL_A_PORT, &hints_TCP, &servinfo_TCP)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
		//return 1;
	}

    //loop and bind
	for (p_TCP = servinfo_TCP; p_TCP != NULL; p_TCP = p_TCP->ai_next) {
		if ((sockfd_TCP = socket(p_TCP->ai_family, p_TCP->ai_socktype,p_TCP->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd_TCP, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd_TCP, p_TCP->ai_addr, p_TCP->ai_addrlen) == -1) {
			close(sockfd_TCP);
			perror("server: bind");
			continue;
		}
		break;
	}
    if (p_TCP == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
		//return 2;
	}
}
/********************************************************************************************/
//create TCP_B
/********************************************************************************************/
void createTCPB(){
    memset(&hints_TCP_B, 0, sizeof hints_TCP_B);
	hints_TCP_B.ai_family = AF_UNSPEC;
	hints_TCP_B.ai_socktype = SOCK_STREAM;
	//hints_TCPA.ai_flags = AI_PASSIVE;

	if ((rv_B = getaddrinfo(LOCAL_HOST, CENTRAL_B_PORT, &hints_TCP_B, &servinfo_TCP_B)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_B));
		exit(1);
		//return 1;
	}

    //loop and bind
	for (p_TCP_B = servinfo_TCP_B; p_TCP_B != NULL; p_TCP_B = p_TCP_B->ai_next) {
		if ((sockfd_TCP_B = socket(p_TCP_B->ai_family, p_TCP_B->ai_socktype,p_TCP_B->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd_TCP_B, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd_TCP_B, p_TCP_B->ai_addr, p_TCP_B->ai_addrlen) == -1) {
			close(sockfd_TCP_B);
			perror("server: bind");
			continue;
		}
		break;
	}
    if (p_TCP_B == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
		//return 2;
	}

}
/********************************************************************************************/
//TCP listen
//learn from Beej’s socket programming tutorial 
/********************************************************************************************/
void TCPlisten(){
	if (listen(sockfd_TCP, BACKLOG) == -1) {
			perror("listen");
			exit(1);
		}
	else if (listen(sockfd_TCP_B, BACKLOG) == -1) {
			perror("listen");
			exit(1);
		}

	sa.sa_handler = sigchld_handler; // read all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	//printf("server: waiting for connections...\n");
}

/********************************************************************************************/
//send request to T
//parameter: (string) input name from client A or B
//get all paths and allnames in paths
/********************************************************************************************/
void requestT(string input){
	// get serverT info
	memset(&hints_UDP, 0, sizeof hints_UDP);
	hints_UDP.ai_family = AF_UNSPEC;
	hints_UDP.ai_socktype = SOCK_DGRAM;

	if ((rv_UDP = getaddrinfo(LOCAL_HOST, SERVER_T_PORT, &hints_UDP, &serverTinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDP));
		//return 1;
		exit(1);
	}
	// send the inputA or inuptB request to serverT
	if ((numbytes_UDP = sendto(sockfd_UDP, input.c_str(), strlen(input.c_str())+1, 0, serverTinfo->ai_addr, serverTinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}

}
/********************************************************************************************/
//send request to S
//parameter: allnames  
//send allnames in the paths to get their scores
/********************************************************************************************/
void requestS(){
	// get serverT info
	memset(&hints_UDP, 0, sizeof hints_UDP);
	hints_UDP.ai_family = AF_UNSPEC;
	hints_UDP.ai_socktype = SOCK_DGRAM;

	if ((rv_UDP = getaddrinfo(LOCAL_HOST, SERVER_S_PORT, &hints_UDP, &serverSinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDP));
		//return 1;
		exit(1);
	}
	// send the request to serverS
	if ((numbytes_UDP = sendto(sockfd_UDP,allnames, sizeof(allnames), 0, serverSinfo->ai_addr, serverSinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}

}
/********************************************************************************************/
//send request to P
//parameter: allnames;scores; number of paths ; all paths
//get the final Compatibility score
/********************************************************************************************/
void requestP(int result_size){
	// get serverT info
	memset(&hints_UDP, 0, sizeof hints_UDP);
	hints_UDP.ai_family = AF_UNSPEC;
	hints_UDP.ai_socktype = SOCK_DGRAM;

	if ((rv_UDP = getaddrinfo(LOCAL_HOST, SERVER_P_PORT, &hints_UDP, &serverPinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_UDP));
		//return 1;
		exit(1);
	}
	// send the request to serverS
	/********************************send allnames and scores in paths************************************************************/
	if ((numbytes_UDP = sendto(sockfd_UDP,allnames, sizeof(allnames), 0, serverPinfo->ai_addr, serverPinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
	if ((numbytes_UDP = sendto(sockfd_UDP,scores, sizeof(scores), 0, serverPinfo->ai_addr, serverPinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
	//int b = result.size();
    /********************************send number of paths************************************************************/
	char nmsg[1];
    nmsg[0] = result_size+'0';
    //cout << nmsg << endl;
    if ((numbytes_UDP = sendto(sockfd_UDP, nmsg, sizeof(nmsg), 0,  serverPinfo->ai_addr, serverPinfo->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
    /********************************send all paths************************************************************/
    for (int i= 0; i< result_size; i++){
        memset(dataBuff,'\0',sizeof(dataBuff));
        strncpy(dataBuff, dataBuff2[i], strlen(dataBuff2[i])+1);
        if ((numbytes_UDP = sendto(sockfd_UDP, dataBuff, sizeof(dataBuff), 0, serverPinfo->ai_addr, serverPinfo->ai_addrlen)) == -1){
            perror("talker: sendto");
            exit(1);
        }
    }

}
/**************************************************************************************************/
//receive topology from T
//parameters: allnames(char allnames[]); number of paths(int result_size); all paths (char dataBuff2)
/**************************************************************************************************/
int recvfromT(){
    //char stopmsg;
    int m = 0;
    //vector<string> names;
    /********************************receive allnames in paths************************************************************/
	if((numbytes_UDP = recvfrom(sockfd_UDP, dataBuff, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &add_size))==-1){
		perror("recvfrom");
		exit(1);
	}
    //cout << dataBuff <<endl;
	strncpy(allnames, dataBuff, sizeof(dataBuff));
    if (allnames[0] == '1'){
		//cout << "error" <<endl;
		return 0;
	}else{
		/********************************receive number of paths************************************************************/
    memset(dataBuff,'\0',sizeof(dataBuff));
    if((numbytes_UDP = recvfrom(sockfd_UDP, dataBuff, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &add_size))==-1){
		perror("recvfrom");
		exit(1);
	}
    //cout << dataBuff <<endl;
    int result_size = dataBuff[0] - '0';
    //cout << result_size << endl;

    /********************************receive all paths************************************************************/
    memset(dataBuff,'\0',sizeof(dataBuff));
	memset(dataBuff2,'\0',sizeof(dataBuff2));
    for (int i= 0; i< result_size; i++){
        if ((numbytes_UDP = recvfrom(sockfd_UDP, dataBuff, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &add_size))==-1){
            perror("recvfrom");
		    exit(1);
        }
        strncpy(dataBuff2[i], dataBuff, sizeof(dataBuff));
        //cout << dataBuff2[i]<< endl;
        //cout << dataBuff << endl;
    }
	//cout << dataBuff2[0]<< endl;
	//cout << dataBuff2[1]<< endl;
	//cout<< "test flag" << allnames << endl;
	return result_size;

	}
}
/********************************************************************************************/
//receive scores from S
//parameter: scores (char scores)
/********************************************************************************************/
void recvfromS(){
    memset(dataBuff,'\0',sizeof(dataBuff));
	if((numbytes_UDP = recvfrom(sockfd_UDP, dataBuff, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &add_size))==-1){
		perror("recvfrom");
		exit(1);
	}
    //cout << "test flag"<< dataBuff <<endl;
	strncpy(scores, dataBuff, sizeof(dataBuff));
	//cout << scores << endl;

}
/********************************************************************************************/
//receive result from P
//parameter: pathindex(int pathindex): to get the related path
//parameter: Compatibility score (char final_char_sum[])
/********************************************************************************************/
int recvfromP(){
	int pathindex;
    memset(dataBuff,'\0',sizeof(dataBuff));
	if((numbytes_UDP = recvfrom(sockfd_UDP, dataBuff, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &add_size))==-1){
		perror("recvfrom");
		exit(1);
	}
	pathindex = dataBuff[0] - '0';
	memset(dataBuff,'\0',sizeof(dataBuff));
	if((numbytes_UDP = recvfrom(sockfd_UDP, dataBuff, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &add_size))==-1){
		perror("recvfrom");
		exit(1);
	}
	strncpy(final_char_sum, dataBuff, sizeof(dataBuff));
	//cout << final_char_sum << endl;
    return pathindex;
}


int main(){
	result_size = -1;
    //printf("The Central server is up and running.\n");
	cout << "The Central server is up and running." << endl;
	//cout << endl;
	/*****************************************UDP*************************************************************/ 
	createUDP();
	/**********************************TCP with clientA ******************************************************/
	createTCPA();
	/**********************************TCP with clientB*******************************************************/
	createTCPB();
    /************************************* TCP listen   ******************************************************/
    TCPlisten();
    /**********************************communicate with client************************************************/
	while (true) { // main accept() loop
	    if(result_size == 0){
			//here: //cout << "error: can not find input name or no path between two input names" << endl;
			here: char errorFlag[1];
            errorFlag[0] = (result_size-1)+'0';
			//cout << clientAMsg << endl;
			if (send(new_fd_TCP, errorFlag,sizeof(errorFlag), 0) == -1){
				perror("send");
			}
			//cout << clientBMsg << endl;
			//if (send(new_fd_TCP, clientBMsg,sizeof(clientBMsg), 0) == -1){
			//	perror("send");
			//}
			if (send(new_fd_TCP_B, errorFlag,sizeof(errorFlag), 0) == -1){
				perror("send");
			}
			//if (send(new_fd_TCP_B, clientAMsg,sizeof(clientAMsg), 0) == -1){
			//	perror("send");
			//}

		}
		memset(clientAMsg,'\0',sizeof(clientAMsg));
		memset(clientBMsg,'\0',sizeof(clientBMsg));
		string inputA,inputB;
		add_size = sizeof their_addr;
        //addr_len = sizeof their_addr;
		new_fd_TCP = accept(sockfd_TCP, (struct sockaddr*)&their_addr, &add_size);
		new_fd_TCP_B = accept(sockfd_TCP_B, (struct sockaddr*)&their_addr, &add_size);
		if (new_fd_TCP == -1) {
			perror("accept");
			continue;
		}
		if (new_fd_TCP_B == -1) {
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr*)&their_addr),s, sizeof s);
		//printf("server: got connection from %s\n", s);

				//char username[50];
				/****************************************receive from A *******************************************************************/
				if (recv(new_fd_TCP, clientAMsg,MAXDATASIZE, 0) == -1)
					perror("recv");
				inputA = clientAMsg;
				cout << "The Central server received input=\"" << clientAMsg << "\" from the client using TCP over port " << CENTRAL_A_PORT << "." <<endl;
				/****************************************receive from B *******************************************************************/
				if (recv(new_fd_TCP_B, clientBMsg,MAXDATASIZE, 0) == -1)
					perror("recv");	
				//cout<<clientMsg<<endl;
				inputB = clientBMsg;
				cout << "The Central server received input=\"" << clientBMsg << "\" from the client using TCP over port " << CENTRAL_B_PORT<< "." <<endl;
			    if (send(new_fd_TCP, clientBMsg,sizeof(clientBMsg), 0) == -1){
				perror("send");
			    }
				if (send(new_fd_TCP_B, clientAMsg,sizeof(clientAMsg), 0) == -1){
				perror("send");
			    }
                //cout << endl;
				/**************************************************************************************************************************/
				// send request to T
				/**************************************************************************************************************************/
				requestT(inputA);
				requestT(inputB);
				cout << "The Central server sent a request to Backend-Server T." << endl;
				//return 0;
				/**************************************************************************************************************************/
		        // rec from T
		        //parameter: dataBuff2[][];  topology send to P
		        //parameter: allnames[]; names send to S
		        /******************************************************************************************************************* ******/
                //int result_size;
		        result_size = recvfromT();
				if (result_size == 0) goto here;
		        //cout << "test: all names" <<allnames <<endl;
		        //cout << "test: path"<<dataBuff2[0]<< endl;
	            //cout << dataBuff2[1]<< endl;
		        //cout << dataBuff2[2]<< endl;
		        cout <<"The Central server received information from Backend-Server T using UDP over port "<<  CENTRAL_UDP_PORT << "." <<endl;  
		        /**************************************************************************************************************************/
		        // send to S
		        //parameter: allnames[]; names send to S
		        /**************************************************************************************************************************/
		        requestS();
		        cout << "The Central server sent a request to Backend-Server S." << endl;
		        /**************************************************************************************************************************/
		        // rec from S
		        //parameter: scores[]; scores send to P
		        /**************************************************************************************************************************/
		        recvfromS();
		        //cout << "test: scores"<< scores << endl;
		        cout <<"The Central server received information from Backend-Server S using UDP over port "<<  CENTRAL_UDP_PORT << "." <<endl;
		        /**************************************************************************************************************************/
		        // send to P
		        //parameter: allnames[]; names send to P
		        //parameter: scores[]; scores send to P
		        //parameter: dataBuff2[][];  topology send to P
		        /**************************************************************************************************************************/
		        requestP(result_size);
		        cout << "The Central server sent a processing request to Backend-Server P." << endl;
		        /**************************************************************************************************************************/
		        // recv from P
		        //parameter: pathindex[]; 
		        //parameter: final_char_sum[]; result send to A/B
		        /**************************************************************************************************************************/
		        int pathindex;
		        pathindex = recvfromP();
				cout << "The Central server received the results from backend server P." << endl;
		        //cout << pathindex <<endl;
		        //cout << dataBuff2[pathindex] << endl;
                /**************************************************************************************************************************/
		        //send result to A
		        //parameter: dataBuff2[pathindex]; path send to A 
		        //parameter: final_char_sum[]; result send to A
		        /**************************************************************************************************************************/
                if (send(new_fd_TCP, final_char_sum,sizeof(final_char_sum), 0) == -1){
					perror("send");
				}
				if (send(new_fd_TCP,dataBuff2[pathindex] ,sizeof(dataBuff2[pathindex]), 0) == -1){
					perror("send");
				}
		        cout << "The Central server send the result to client A." <<endl;
	            /*************************************************************************************************************************/
		        //send result to B
		        //parameter: dataBuff2[pathindex]; path send to B
		        //parameter: final_char_sum[]; result send to B
		        /*************************************************************************************************************************/
	            if (send(new_fd_TCP_B, final_char_sum,sizeof(final_char_sum), 0) == -1){
					perror("send");
				}
				if (send(new_fd_TCP_B,dataBuff2[pathindex] ,sizeof(dataBuff2[pathindex]), 0) == -1){
					perror("send");
				}
		        cout << "The Central server send the result to client B." <<endl;
				memset(final_char_sum,'\0',sizeof(final_char_sum));
				//break;
		
		close(new_fd_TCP); // parent doesn't need this
		close(new_fd_TCP_B);
	}
		return 0;

}
