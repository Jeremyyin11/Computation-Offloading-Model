#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <vector>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>      

using namespace std;

#define AWS_CLIENT_PORT "25785" 
#define MAXDATASIZE 100 
const char* localhost = "127.0.0.1";

struct c_result {
    char tt[MAXDATASIZE];
    char tp[MAXDATASIZE];
    char delay[MAXDATASIZE]; 
};

struct c_result c_result;

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

    if (argc != 4) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

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

    argu = string(argv[1]) + "@" + string(argv[2]) + "@" + string(argv[3]);
    
    freeaddrinfo(servinfo); 
    printf("The client is up and running. \n");

    memset(buf, 0, MAXDATASIZE);
    strcpy(buf, argu.c_str());

    if (send(sockfd, buf, MAXDATASIZE-1, 0) == -1)
        perror("send");

    printf("The client sent ID=<%s>, size=<%s>, and power=<%s> to AWS\n", argv[1], argv[2], argv[3]);


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
        char s[2] = "@";    
        char * pch;

        pch = strtok(buf, s);
        strcpy(c_result.tt, pch);
        vector<string> v;

        int i = 0;
        while (i < 2)
        {   
            pch = strtok (NULL, s);
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
        printf("The delay for link <%s> is <%.2f>ms\n",argv[1], delay);

    } else {

        printf("Found no matches for link <%s>\n", argv[1]);
    }     

    close(sockfd);
    return 0;
}
