#include <stdio.h>
#include <vector>
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
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include<bits/stdc++.h>
#include <algorithm>
#include <cmath>

using namespace std;

#define SERVER_T_PORT "21408" //udp
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
char s[INET6_ADDRSTRLEN];
vector<vector<string> > result;

void *get_in_addr(struct sockaddr *sa)
{
if (sa->sa_family == AF_INET) {
return &(((struct sockaddr_in*)sa)->sin_addr);
}
return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/********************************************************************************************/
//find all path
/********************************************************************************************/

void findpath(vector<string> names, vector<vector<int> > links,int n, int b, int e,vector<int> left, vector<string> path) {
	int i,j;
	for (i = 0; i < left.size(); i++) {
		if (links[b][left[i]] == 1) {
			path.push_back(names[left[i]]);
			if (left[i] == e) {
				for (j = 0; j < path.size(); j++) {
					//cout << path[j] << ' ';
				}
                result.push_back(path);
				//cout << endl;
				//return;
			}
			else {
				vector<int> newleft(left);
				newleft.erase(newleft.begin()+i);
				findpath(names, links, n, left[i], e, newleft, path);
			}
			path.pop_back();
		}
	}
}

/********************************************************************************************/
//create UDP
//learn from Beej’s socket programming tutorial 
/********************************************************************************************/

void createUDP(){
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVER_T_PORT, &hints, &servinfo)) != 0){
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
	cout << "The ServerT is up and running using UDP on port " << SERVER_T_PORT << "." <<endl;

}

/********************************************************************************************/
//rec request from C
/********************************************************************************************/
string recvfromC(){
	if((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
	string msg = buf;
	//cout << msg <<endl;
	return msg;
}


int main(){
	int errorflag=0;
    createUDP();
	addr_len = sizeof their_addr;
    string msgA,msgB;
	while(true){
        if (errorflag == 1) {
            //here: cout << "error: can not find input name or no path between two input names" << endl;
            here: char errorFlag[1];
            errorFlag[0] = errorflag+'0';
            //errorflag = 0;
            //cout << nmsg << endl;
            if ((numbytes = sendto(sockfd, errorFlag, sizeof(errorFlag), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
                perror("talker: sendto");
		        exit(1);
            }
            errorflag = 0;
        }
		msgA=recvfromC();
		msgB=recvfromC();
		//cout<< msgA << endl;
		//cout<< msgB << endl;
		cout << "The serverT received a request from Central to get the topology." << endl;
        /********************************************************************************************/
        //topology
        /********************************************************************************************/
        string input[MAXDATASIZE][2];
        ifstream myfile("edgelist.txt");
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

        /***********************************************************/
        //get the rows of input
        /***********************************************************/
        for (int i = 0; i < MAXDATASIZE; i++){
            if (input[i][0] == "") break;
            else rows++;
        }
        //cout << columns << endl;
        //get the all name of input
        vector<string> names;
        for (int i = 0; i < rows; i++){
            for(int j = 0; j < columns; j++){
                names.push_back(input[i][j]);
            }
        }  
        //test
        //  for (int i = 0; i < names.size(); i++)
        // {
        //     cout << names[i] << endl;
        // }
        // cout << "1" <<endl;
        /***********************************************************/
        /*********************rank and delete************************/
        sort(names.begin(),names.end());	//rank
        names.erase(unique(names.begin(),names.end()),names.end());	//delete
        int errorflag1 = 1;
        int errorflag2 = 1;
        //cout << msgA << " " << msgB << endl;
        //test
        for (int i = 0; i < names.size(); i++){
            //cout << names[i] << errorflag << endl;
            if(msgA == names[i]) errorflag1 = 0;
            if(msgB == names[i]) errorflag2 = 0;
            //cout << names[i] << errorflag1 << errorflag2 << endl;
        }
        if (errorflag1 == 1 || errorflag2 == 1) errorflag = 1;
        if (errorflag == 1) goto here;
        /************************************************************/
        /*********************Create adjacency matrix***************/
        /**********************************************************/
        string x1,x2;
        x1 = "a";
        int counter = 0;
        int j,k;
        vector<vector<int> > links(names.size(), vector<int>(names.size()));
        x1 = input[counter][0];
        x2 = input[counter][1];
        //cout << names.size() <<endl;
        while (x1!=""){
            for (j = 0; j < names.size(); j++) {
                if (names[j] == x1) break;
            }
            for (k = 0; k < names.size(); k++) {
                if (names[k] == x2) break;
            }
            links[j][k] = links[k][j] = 1;
            x1 = input[counter][0];
            x2 = input[counter][1];
            counter++;
        }
        //cout << counter <<endl;
        //test link matrix
        // int m=1;
        // for (int i = 0; i < names.size(); i++){
        //     for (int j = 0; j < names.size(); j++){
        //         cout << links[i][j];
        //         if (m==6) {
        //             cout << "\n" <<endl;
        //             m=1;
        //         }
        //         else m++;
        //     }
        // }
        /************************************************************/
        /*********************find path*****************************/
        /**********************************************************/
        string beg, en;
        //test
        beg = msgA;
        en = msgB;
        int i;
        for (j = 0; j < names.size(); j++) {
            if (names[j] == beg) break;
	    }
	    for (k = 0; k < names.size(); k++) {
            if (names[k] == en) break;
	    }
        //cout << j << endl;
        //cout << k << endl;

	    vector<int> left;
	    for (i = 0; i < names.size(); i++) {
            if(i!=j) left.push_back(i);
        }

        vector<string> path;
	    path.push_back(names[j]);
	    findpath(names, links,names.size(), j, k, left, path);
        //return 0;
        //test output
        // vector<vector<string>>::iterator iter;
        // for (iter = result.begin(); iter != result.end() ; ++iter) {
        //     for (int i = 0; i < (*iter).size(); ++i) {
        //         cout << (*iter)[i] << " " ;
        //     }
        // cout << endl;
        // }

	    /********************************************************************************************/
        /******************************************send**********************************************/
        /********************************************************************************************/
        //vector to string 
        string compare[MAXDATASIZE];
        int ccounter=0;
        for (int i=0; i < result.size();i++){
            for (int j=0; j< result[i].size();j++){
                compare[ccounter] = result[i][j];
                ccounter++;
                //cout <<   result[i][j] << endl;
                //res += " ";
            }
        }
        //cout << res << endl;
        // for (int i = 0; i < ccounter; i++){
        //     cout << compare[i] << endl;
        // }
        vector<string> res;
        for (int i = 0; i < ccounter; i++){
            res.push_back(compare[i]);
        }
        sort(res.begin(),res.end());	//rank
        res.erase(unique(res.begin(),res.end()),res.end());	//delete

        // for (int i = 0; i < res.size(); i++){
        //     cout << res[i] << endl;
        // }
        string final_res;
        for (int i = 0;i<res.size();i++) {
            final_res += res[i];
            final_res += " ";
        }
        //cout << final_res <<endl;
        char dataBuff[MAXDATASIZE];
        memset(dataBuff,'\0',sizeof(dataBuff));
        strncpy(dataBuff, final_res.c_str(), final_res.length() + 1);
        /********************************result format change****************************************************/
        /********************************************************************************************************/
        string res2[result.size()];
        for (int i=0; i < result.size();i++){
            for (int j=0; j< result[i].size();j++){
                res2[i] += result[i][j];
                res2[i] += " ";
            }
        }
        char dataBuff2[MAXDATASIZE][MAXDATASIZE];
        //memset(dataBuff2,'\0',sizeof(dataBuff2));
        for (int i=0;i < result.size(); i++){
            strncpy(dataBuff2[i], res2[i].c_str(), res2[i].length() + 1);
            //cout << "test" <<dataBuff2[i] << endl;
        }

        /********************************send names ************************************************************/
        //here:
        // if (errorflag == 1){
        //     //string errorFlag = "#";
        //     char errorFlag[1];
        //     errorFlag[0] = errorflag + '0';
        //     if ((numbytes = sendto(sockfd, errorFlag,sizeof(errorFlag), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
        //         perror("talker: sendto");
		//         exit(1);
        //     }
        //     cout << "error"  << endl;
        // }else{
            if ((numbytes = sendto(sockfd, dataBuff, sizeof(dataBuff), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
                perror("talker: sendto");
		        exit(1);
	        }
            int b = result.size();
            char nmsg[1];
            nmsg[0] = b+'0';
            //cout << nmsg << endl;
            if ((numbytes = sendto(sockfd, nmsg, sizeof(nmsg), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
                perror("talker: sendto");
		        exit(1);
            }
        /********************************send result************************************************************/
            for (int i= 0; i< result.size(); i++){
                memset(dataBuff,'\0',sizeof(dataBuff));
                strncpy(dataBuff, dataBuff2[i], strlen(dataBuff2[i])+1);
                if ((numbytes = sendto(sockfd, dataBuff, sizeof(dataBuff), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
                    perror("talker: sendto");
                    exit(1);
                }
            }
        cout << "The serverT finished sending the topology to Central." << endl;
        result.clear();
       // }
	
	}

	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}