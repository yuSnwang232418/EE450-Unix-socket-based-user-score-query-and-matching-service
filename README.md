USC EE450 2021fall Socket Programming Project

a. Yuxi Wang

b. Student ID: 4568893408

c.
## What I have done in the assignment

I completed all the required phase except Phase4(10 points extra)

** Phase 1A: 

1) serverC, serverS, serverP, serverT boot up

2) Both clientA and clientB boot up and read the input username (username1 and username2)

3) clientA and clientB send the inputs to the central server (serverC) over TCP

* note: - after both clientA and clientB input username, the received information will show on cerverC's "screen"

** Phase 1B: 

1) set the connection between cerverC and all other back-end servers

2) Requests sent from serverC to respective back-end servers over UDP

** Phase 2:

1) serverC get the all topology paths from serverT

2) serverC then get the related scores from serverS

3) serverC forward the topology and scores to serverP

4) serverP get the final smallest matching gap after calculation

** Phase 3:

1) serverP send the final result to serverC

2) serverC forward the corresponding path and compatibility scores to clientA and clientB

3) if no path connects both usernames, show that no matching can occur 

* note: - After get the matching gap, clientA and clientB will close but all servers will still run.
*         We can re-open clients and input usernames to get new matching gap again and again. 

d.
## Code files and function

1) clientA.cpp:
*   ./clientA : start the clientA; then enter <username> to boot up
    - Ask for user to input the username
	- Create TCP connection with serverC
    - Send username to central serverC over TCP
	- Receive final smallest matching gap from central serverC

2) clientB.cpp
*   ./clientB : start the clientB; then enter <username> to boot up
    - Ask for user to input the username
	- Create TCP connection with serverC
    - Send username to central serverC over TCP
	- Receive final smallest matching gap from central serverC

3) central.cpp
*   make serverC : run the central server
    - Get the input usernames from clientA and clientB over TCP
	- Forward the input usernames to serverT over UDP
	- Get the topology paths from serverT
	- Forward all names in topology paths to serverS over UDP
	- Get the scores related to names in the topology from serverS
	- Forward both paths and related scores to serverP over UDP
	- Get the result (compatibility score and corresponding path) from serverP
	- Forward the result to both clientA and clientB

4) serverT.cpp
*   make serverT : run the serverT
    - Get input usernames from serverC over UDP
	- Read the edgelist.txt to get the social network graph 
	- Find all topology paths using input names and graph
    - Send the paths to serverC
    - Determine whether there is no path between the input names

5）serverS.cpp
*   make serverS : run the serverS
    - Get names in all topology paths from serverC over UDP
	- Read the scores.txt to get the social network graph 
	- Send all scores related to names in all paths back to serverC

6) serverP.cpp
*   make serverP : run the serverS
    - Get all paths and related scores from serverC over UDP
	- Find the matching gaps of each path
    - Find the smallest matching gaps
	- Send the result (compatibility score and corresponding path) back to serverC

e.
## Format of message exchange
The message print on screen is the same as the requirement in project description;

1) clientA.cpp
    - Input "username1" e.g. "Victor"
    - Send username to central serverC (char clientInput[]) "username1" 
	- Receive another username and store it (char clientBMsg[]) "username2" e.g. "Oliver"
	- Receive compatibility score from serverC (char final_result[]) "compatibility score"
	- Receive path from serverC (char path[])
	- If can not find path, final_result[0] == '/'

2) clientB.cpp
    - Input "username2" e.g. "Oliver"
    - Send username to central serverC (char clientInput[]) "username2" 
	- Receive another username and store it (char clientBMsg[]) "username1" e.g. "Victor"
	- Receive compatibility score from serverC (char final_result[]) "compatibility score" 
	- Receive path from serverC (char path[])
	- If can not find path, final_result[0] == '/'

3) central.cpp
    - Get the "username1" and "username2" from clients (char clientAMsg[], char clientBMsg[]) e.g. "Victor", "Oliver"
	- Send the another input name to each client
	- Send input names to serverT (char input[]) "InputA", "InputB" e.g. "Victor", "Oliver"
	- Get names in all paths from serverT (char allnames[]) 
	- Get number of paths from serverT (int result_size)
	- Get all paths from serverT(char dataBuff2[][])
	- Send allnames to serverS (char allnames[])
	- Get related scores from serverS (char scores[])
	- Send allnames, scores, number of paths, paths to serverP (char allnames[], char scores[], char nmsg[], char dataBuff2[][])
	- Get pathindex from serverP (int pathindex)
	- Get compatibility score from serverP (char final_char_sum[])
	- Send compatibility score to both clientA and clientB(char final_char_sum[])
	- Send path to both clientA and clientB (char dataBuff2[pathindex])
	- If can not find path (result_size == 0) send error to both clientA and clientB (char errorFlag[])

4) serverT.cpp
    - Get input names from serverC (string msgA, msgB)
	- If can not find path, send error to serverC (char errorFlag[])
	- Send number all paths to serverC (char dataBuff[])
	- Send all paths to serverC (char dataBuff2[][])
	
5) serverS.cpp
    - Get allnames from serverC (char allnames[])
	- Send related scores to serverC (char dataBuff[])

6) serverP.cpp
    - Get allnames from serverC (char allnames[])
	- Get scores from serverC (char scores[])
	- Get number of paths from serverC (int result_size)
	- Get paths from serverC (char path[][])
	- Send pathindex to serverC (char indexmsg[])
	- send compatibility score to serverC (char final_char_sum)
g.
## Idiosyncrasy in project

I set the MAXDATASIZE = 500, if message exceeds this size, maybe should change this size in each .cpp.
I do not test very large inputs situation, but codes run well in the given example and my own similar testcases below the DATASIZE 500.

h.
## Reused Code

Codes about set the TCP and UDP connections (also like "socket()", "listen()", "send()", "recv()") learn from Beej’s socket programming tutorial.

## Besides
I add some header files in codes beyonds those in project doucument
/*
#include <iostream>
#include <sstream>
#include <fstream>
#include <bits/stdc++.h>
#include <algorithm>
#include <cmath>
#include <vector>
*/


	
	