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

#define SERVER_P_PORT "23408" //udp
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
char scores[MAXDATASIZE];
char allnames[MAXDATASIZE];
char path[MAXDATASIZE][MAXDATASIZE];
int result_size;
string t_output[MAXDATASIZE];
int copy_a = 0;

char s[INET6_ADDRSTRLEN];
//vector<vector<string> > result;

string doubleConverToString(double d){
    ostringstream os;
    if(os << d) return os.str();
    return "invalid conversion";
}

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
	
	if ((rv = getaddrinfo(LOCAL_HOST, SERVER_P_PORT, &hints, &servinfo)) != 0){
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
	cout << "The ServerP is up and running using UDP on port " << SERVER_P_PORT << "." <<endl;

}

/********************************************************************************************/
//rec request from C
/********************************************************************************************/
void recvfromC(){
    //allnames
	if((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
    strncpy(allnames, buf, sizeof(buf));
	//cout << allnames <<endl;
    //scores
    memset(buf,'\0',sizeof(buf));
    if((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
    strncpy(scores, buf, sizeof(buf));
	//cout << scores <<endl;
    //char nmsg[1];
    //get number of path
    memset(buf,'\0',sizeof(buf));
    if((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
		perror("recvfrom");
		exit(1);
	}
    result_size = buf[0] - '0';
    //cout << result_size <<endl;

    memset(buf,'\0',sizeof(buf));
	memset(path,'\0',sizeof(path));
    for (int i= 0; i< result_size; i++){
        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1, 0, (struct sockaddr *)&their_addr, &addr_len))==-1){
            perror("recvfrom");
		    exit(1);
        }
        strncpy(path[i], buf, sizeof(buf));
        
        //cout << path[i]<< i << endl;
        //cout << dataBuff << endl;
    }



}


int main(){
	
    createUDP();
	addr_len = sizeof their_addr;
    //string msgA,msgB;
	while(true){
		recvfromC();
		cout << "The serverP received the topology and score information." << endl;
		//break;
        //input char -->string
        char temp[MAXDATASIZE];
        int counter = 0;
        int a = 0;
        int name_size = 0;
        memset(temp, 0, sizeof(temp));
        /******************************************************/
        //******************allnames string*******************/
        //***************parameter string allnames1[]********/
        string allnames1[MAXDATASIZE];
        for (int i=0; i<sizeof(allnames);i++){
            if(allnames[i]==' '){
                //cout << "next" << i << endl;
                allnames1[a]=temp;
                a++;
                counter = 0;
                memset(temp, 0, sizeof(temp));
            }else{
                temp[counter] = allnames[i];
                //cout << allnames[i];
                //cout << temp << endl;
                counter++;
            }
            allnames1[a]=temp;
        }
        name_size = a;
        //cout << name_size <<endl;
        //cout<< allnames1[0] <<endl;
        /******************************************************/
        //******************scores string*******************/
        //***************parameter string scores1[]********/
        string scores1[MAXDATASIZE];
        memset(temp,'\0',sizeof(temp));
        a = 0;
        counter = 0;
        for (int i=0; i<sizeof(scores);i++){
            if(scores[i]==' '){
                scores1[a]=temp;
                a++;
                counter = 0;
                memset(temp, 0, sizeof(temp));
            }else{
                temp[counter] = scores[i];
                counter++;
            }
            scores1[a]=temp;
        }
        //cout <<scores1[0] <<endl;
        /******************************************************/
        //******************path string*******************/
        //***************parameter string path1[][]********/
        int path_size[MAXDATASIZE];
        string path1[result_size][MAXDATASIZE];
        for (int i = 0; i< result_size;i++){
            //string scores1[MAXDATASIZE];
            memset(temp,'\0',sizeof(temp));
            a = 0;
            counter = 0;
            for (int j=0; j<sizeof(path[i]);j++){
                if(path[i][j]==' '){
                    path1[i][a]=temp;
                    a++;
                    counter = 0;
                    memset(temp, 0, sizeof(temp));
                }else{
                    temp[counter] = path[i][j];
                    counter++;
                }
                path1[i][a]=temp;
            }
            path_size[i] = a-1;
        }
        // cout << path_size[0] <<endl;
        // cout << path_size[1] <<endl;
        //cout << path1[0][0] <<endl;
        //cout << path1[0][3] <<endl;
        //cout << allnames1[0] << endl;
        //cout << allnames1[3] << endl;
        //cout << sizeof(path1[1]) <<endl;

        int index[MAXDATASIZE][MAXDATASIZE];
        for (int i = 0; i < result_size; i++){
            for (int j = 0; j <= path_size[i];j++){
                for(int k = 0; k <= name_size; k++){
                    //cout << path1[i][j]<< k <<endl;
                    if(allnames1[k] == path1[i][j]){
                        index[i][j] = k;
					    //cout << index[i][j] << endl;
                        //break;
                    }
                }
            }
        }
        /***************************************************************/
        /************************scores to int *************************/
        /**************************************************************/
        //i_scores[] int scores
        char c_scores[MAXDATASIZE];
        int i_scores[name_size+1];
        for (int i = 0; i <= name_size; i++){
            memset(c_scores,'\0',sizeof(c_scores));
            strncpy(c_scores,scores1[i].c_str(),scores1[i].length());
            //cout << c_scores[0] << endl;
            i_scores[i] =  atoi(c_scores);
        }
        //  cout << i_scores[0] <<endl;
        //  cout << i_scores[1] <<endl;
        //  cout << i_scores[2] <<endl;
        //  cout << i_scores[3] <<endl;

        /********************************************************/
        /************************find gap*********************/
        //vector<vector<double> > gap[MAXDATASIZE][MAXDATASIZE];
        double gap[MAXDATASIZE][MAXDATASIZE];
        double sum[MAXDATASIZE];
        //vector<double> sum[MAXDATASIZE];
        for (int i = 0; i< MAXDATASIZE; i++){
            sum[i] = 0;
        }
        for (int i = 0; i< MAXDATASIZE; i++){
            for(int j = 0; j < MAXDATASIZE; j++){
                gap[i][j] = 0;
            }
        }


        
        int l_n;
        int r_n;
        for (int i = 0;i <result_size;i++){
            for (int j = 0 ; j< path_size[i];j++){
                int k = j+1;
                l_n = i_scores[index[i][j]];
                //cout << l_n<<endl;
                r_n = i_scores[index[i][k]];
                //cout << r_n << endl;
                gap[i][j] = double(abs(l_n-r_n))/double((l_n+r_n));
                //cout<< gap[i][j] << i << j<< endl;
                sum[i]+=gap[i][j];
            } 
        }
	    //cout << sum[0]<< endl;
	    //cout << sum[1] << endl;
        double min_sum;
        min_sum = sum[0];
        int path_index = 0;;
        for (int i= 0; i< result_size; i++){
            if (sum[i] < min_sum && sum[i] != 0){
                min_sum = sum [i];
                path_index = i;
            }
        }


        string final_sum;
        final_sum = doubleConverToString(min_sum);
        char final_char_sum[MAXDATASIZE];
        memset(final_char_sum,'\0',sizeof(final_char_sum));
        strncpy(final_char_sum,final_sum.c_str(),final_sum.length());
        //cout << min_sum << endl;
	    //cout << final_char_sum << endl;
	    //cout << path_index << endl;

	    /********************************************************************************************/
        /******************************************send**********************************************/
        //********************************************************************************************/
	    char indexmsg[1];
        indexmsg[0] = path_index+'0';
	    if ((numbytes = sendto(sockfd, indexmsg, sizeof(indexmsg), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            perror("talker: sendto");
		    exit(1);
        }
        if ((numbytes = sendto(sockfd, final_char_sum, sizeof(final_char_sum), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            perror("talker: sendto");
		    exit(1);
	    }
	    cout << "The serverP finished sending the results to Central." << endl;
    }
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}