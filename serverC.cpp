#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>      
#include <fstream>
#include <iostream>
#include <iterator> 
#include <string>
#include <vector>
#include <sstream>

using namespace std;

#define MAXBUFLEN 100
#define SERVER_C_PORT "23785"
const char* localhost = "127.0.0.1";

struct info {
    char id[MAXBUFLEN];
    char bandwidth[MAXBUFLEN];
    char length[MAXBUFLEN];
    char velocity[MAXBUFLEN];
    char noise_power[MAXBUFLEN];
    char size[MAXBUFLEN];
    char power[MAXBUFLEN];
};

struct info param;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main () {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 
	//from beej http://beej-zhcn.netdpi.net/06-client-server-background
	if ((rv = getaddrinfo(localhost, SERVER_C_PORT, &hints, &servinfo)) != 0) {
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
    printf( "The ServerC is up and running using UDP on port <%s>.\n", SERVER_C_PORT);

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, 
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {

	perror("recvfrom");
	exit(1);
    }

	
    buf[numbytes] = '\0';

    memset(param.bandwidth, '\0', sizeof(param.bandwidth));
    memset(param.length, '\0', sizeof(param.length));
    memset(param.velocity, '\0', sizeof(param.velocity));
    memset(param.noise_power, '\0', sizeof(param.noise_power));
    memset(param.size, '\0', sizeof(param.size));
    memset(param.power, '\0', sizeof(param.power));
    memset(param.id, '\0', sizeof(param.id));

    char s[2] = "@";
    char * pch;
    vector<string> v;
    pch = strtok(buf, s);
    strcpy(param.size, pch);

    int i = 0;
    while (i < 6)
    {   
        pch = strtok (NULL, s);
        v.push_back(pch);
        i++;
    }

    strcpy(param.power, v.at(0).c_str());
    strcpy(param.bandwidth, v.at(1).c_str());
    strcpy(param.length, v.at(2).c_str());
    strcpy(param.velocity, v.at(3).c_str());
    strcpy(param.noise_power, v.at(4).c_str());
    strcpy(param.id, v.at(5).c_str());

    printf("The Server C received link information of link <%s>, file size <%s>, and signal power <%s>\n", param.id, param.size, param.power);

    double bandwidth = atof(param.bandwidth);
    double length = atof(param.length);
    double velocity = atof(param.velocity);
    double noise_power = atof(param.noise_power);
    double size = atof(param.size);
    double power = atof(param.power);
    double signal_power_watt = pow(10,(double)power/10) / 1000;
    double noise_power_watt = pow(10, (double)noise_power/10) / 1000;
    double capacity = bandwidth * pow(10, 6) * (log(1 + signal_power_watt / noise_power_watt) / log(2));
    double tt = (size / capacity) * 1000 ;
    double tp = (length * pow(10, 3) / (velocity * pow(10, 7))) * 1000 ;
    double delay = (tt + tp ) ;



    printf("The server C finished the calculation for link <%s>\n", param.id);

    memset(buf,0,sizeof(buf));
    string param = "";

    ostringstream streamObj;
    streamObj << fixed;
    streamObj << tt;
    string strTT = streamObj.str();

    ostringstream streamObj2;
    streamObj2 << fixed;
    streamObj2 << tp;
    string strTP = streamObj2.str();

    ostringstream streamObj3;
    streamObj3 << fixed;
    streamObj3 << delay;
    string strDl = streamObj3.str();

    param = strTT + "@" + strTP + "@" + strDl;
    strcpy(buf, param.c_str());
    if ((numbytes = sendto(sockfd, buf, MAXBUFLEN-1 , 0,
     (struct sockaddr *)&their_addr, addr_len)) == -1) {
        perror("123");
        exit(1);
    }
    printf("The Server C finished sending the output to AWS\n");

}	
	freeaddrinfo(servinfo);
	close(sockfd);

}
