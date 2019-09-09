#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <math.h>      

using namespace std;

#define AWS_CLIENT_PORT "26785" 
#define MAXDATASIZE 100 
const char* localhost = "127.0.0.1";

struct c_result {
    char tt[MAXDATASIZE];
    char tp[MAXDATASIZE];
    char delay[MAXDATASIZE]; 
};

struct c_result c_result;

struct client_input {
    char id[MAXDATASIZE];
    char size[MAXDATASIZE];
    char power[MAXDATASIZE];
};

struct client_input client_input;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    string argu;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	//from beej http://beej-zhcn.netdpi.net/06-client-server-background
    if ((rv = getaddrinfo(localhost, AWS_CLIENT_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
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
    
    freeaddrinfo(servinfo); 
    printf("The monitor is up and running. \n");

    while(1){
        memset(buf, 0, MAXDATASIZE);
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

   	    buf[numbytes] = '\0';

    	memset(client_input.id, '\0', sizeof(client_input.id));
    	memset(client_input.size, '\0', sizeof(client_input.size));
    	memset(client_input.power, '\0', sizeof(client_input.power));
    	char s1[2] = "@";    
    	char * pch;
	
   	    pch = strtok(buf, s1);
   	    strcpy(client_input.id, pch);
    	vector<string> v;

    	int i = 0;
    	while (i < 2)
    	{   
       	    pch = strtok (NULL, s1);
            v.push_back(pch);
            i++;
    	}

    	strcpy(client_input.size, v.at(0).c_str());
    	strcpy(client_input.power, v.at(1).c_str());

   	    printf("The monitor received input=<%s>, size=<%s>, and power=<%s> from the AWS\n",client_input.id, client_input.size, client_input.power);

    	memset(buf, 0, MAXDATASIZE);

    	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
    	}

    	buf[numbytes] = '\0';

    	if (string(buf).compare("NOT FIND") != 0){
            memset(c_result.tt, '\0', sizeof(c_result.tt));
            memset(c_result.tp, '\0', sizeof(c_result.tp));
            memset(c_result.delay, '\0', sizeof(c_result.delay));

            pch = strtok(buf, s1);
            strcpy(c_result.tt, pch);
            vector<string> v;

            int i = 0;
	        while (i < 2){   
                pch = strtok (NULL, s1);
                v.push_back(pch);
                i++;
            }

            strcpy(c_result.tp, v.at(0).c_str());
            strcpy(c_result.delay, v.at(1).c_str());
	    
	        double tt = atof(c_result.tt);
	        double tp = atof(c_result.tp);
	        double delay = atof(c_result.delay);
	        tt = round(tt * 100) / 100;
    	    tp = round(tp * 100) / 100;
    	    delay = round(delay * 100) / 100;
            printf("The result for link <%s>:\nTt = <%.2f>ms,\nTp = <%.2f>ms,\nDelay = <%.2f>ms”\n",client_input.id, tt,tp,delay);
    	} else {
            printf("Found no matches for link <%s>\n", client_input.id);
    	}     
    }
    close(sockfd);
    return 0;
}
