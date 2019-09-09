#include <fstream>
#include <iostream>
#include <iterator> 
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

#define MAXBUFLEN 100
#define SERVER_B_PORT "22785"
const char* localhost = "127.0.0.1";
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main () {
	map<string, string > serverB_map;

	ifstream myFile("database_b.csv");
	if (!myFile.is_open()) {
		cout << "ERROR: OPEN FILE FAILED" << endl;
	}
	string LindId;
	string Bandwidth;
	string Length;
	string Velocity;
	string NoisePower;
	string str;

	while (myFile.good()) {
	    getline(myFile, LindId, ',');
	    getline(myFile, Bandwidth, ',');
	    getline(myFile, Length, ',');
	    getline(myFile, Velocity, ',');
	    getline(myFile, NoisePower, '\n');
	    str = Bandwidth + "@"+Length + "@" + Velocity + "@" + NoisePower;
	    serverB_map.insert(pair <string, string> (LindId, str)); 		
	}
	myFile.close(); 

	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	//from beej http://beej-zhcn.netdpi.net/06-client-server-background
	hints.ai_family = AF_UNSPEC;  
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;  

	if ((rv = getaddrinfo(localhost, SERVER_B_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {

		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
		  perror("listener: socket");
		  continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		  close(sockfd);
		  perror("listener: bind");
		  continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	addr_len = sizeof their_addr;

while(true){
	printf( "The ServerB is up and running using UDP on port <%s>.\n", SERVER_B_PORT);

	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, 
	(struct sockaddr *)&their_addr, &addr_len)) == -1) {

		perror("recvfrom");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("The ServerB received input <%s>\n", buf);

	string sendbuf;
	if(serverB_map.find(buf) != serverB_map.end()){
		printf("The server B has found <1> match\n");
		sendbuf = serverB_map[buf];
	} else {
		printf("The server B has found <0> match\n");
		sendbuf = "NOT FIND";
	}

	memset(buf, 0, MAXBUFLEN);
	strcpy(buf, sendbuf.c_str());
	buf[sendbuf.length()] = '\0';
	if ((numbytes = sendto(sockfd, buf, MAXBUFLEN-1 , 0,
     (struct sockaddr *)&their_addr, addr_len)) == -1) {

        perror("send");
        exit(1);
    }
 	printf("The Server B finished sending the output to AWS\n");

}	
	freeaddrinfo(servinfo);
	close(sockfd);

}
