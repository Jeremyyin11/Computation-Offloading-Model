
#include <iostream>
#include <iterator> 
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <math.h>      
using namespace std;

#define SERVER_A_PORT "21785"
#define SERVER_B_PORT "22785"
#define SERVER_C_PORT "23785"
#define AWS_UDP_PORT 24785

#define MAXBUFLEN 100 // number of bytes
#define AWS_CLIENT_PORT "25785" 
#define AWS_MONITOR_PORT "26785"
#define BACKLOG 10 // pending connections queue
const char* localhost = "127.0.0.1";
struct sockaddr_in AWS_P;
struct info {
    char id[MAXBUFLEN];
    char bandwidth[MAXBUFLEN];
    char length[MAXBUFLEN];
    char velocity[MAXBUFLEN];
    char noise_power[MAXBUFLEN];
};

struct client_input {
    char id[MAXBUFLEN];
    char size[MAXBUFLEN];
    char power[MAXBUFLEN];
};
struct c_result {
    char tt[MAXBUFLEN];
    char tp[MAXBUFLEN];
    char delay[MAXBUFLEN]; 
};

struct c_result c_result;
struct info detail;
struct client_input client_input;


bool flag = true;
char final_res[MAXBUFLEN];
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}


void *get_in_addr(struct sockaddr *sa_client)
{
    if (sa_client->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa_client)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa_client)->sin6_addr);
}

int AB_Connection (char link_id[], char port){
        //set up UDP from Beej   http://beej-zhcn.netdpi.net/06-client-server-background
    int m = 0;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char* PORT;
    int numbytes;
    char result[MAXBUFLEN];

    if (port == 'A') {
        PORT = (char *)SERVER_A_PORT;
    }
    else if (port == 'B') {
        PORT = (char *)SERVER_B_PORT;
    }

	//from beej http://beej-zhcn.netdpi.net/06-client-server-background
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(localhost, PORT, &hints, &servinfo))
        != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
            == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
	//from beej  http://beej-zhcn.netdpi.net/06-client-server-background
    memset((char *)&AWS_P, 0, sizeof(AWS_P));
    AWS_P.sin_family = AF_INET;
    AWS_P.sin_port = htons(AWS_UDP_PORT);	
    if(inet_aton(localhost, &AWS_P.sin_addr) == 0){
	printf("inet_aton() failed\n");
	exit(1);
    }

    if(bind(sockfd, (struct sockaddr*)&AWS_P, sizeof(AWS_P)) == -1){
	perror("bind");
	exit(1);
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }


    if ((numbytes = sendto(sockfd, link_id, MAXBUFLEN, 0, p->ai_addr,p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    
    printf("The AWS sent link ID=<%s> to Backend-Server <%c> using UDP over port <%d>\n", client_input.id, port, AWS_UDP_PORT);

    memset(result,0,sizeof(result));
	//from beej  http://beej-zhcn.netdpi.net/06-client-server-background
    if ((numbytes = recvfrom(sockfd, result, sizeof result, 0 , NULL, NULL)) == -1) {
        perror("talker: recvfrom");
        exit(1);
    }


    result[numbytes] = '\0';

    if (string(result).compare("NOT FIND") != 0){
        flag = false;
        m = 1;
        memset(detail.bandwidth, '\0', sizeof(detail.bandwidth));
        memset(detail.length, '\0', sizeof(detail.length));
        memset(detail.velocity, '\0', sizeof(detail.velocity));
        memset(detail.noise_power, '\0', sizeof(detail.noise_power));

        char s[2] = "@";
        char * pch;
        vector<string> v;
        pch = strtok(result, s);
        strcpy(detail.bandwidth, pch);

        int i = 0;
        while (i < 3)
        {   
            pch = strtok (NULL, s);
            v.push_back(pch);
            i++;
        }

        strcpy(detail.length, v.at(0).c_str());
        strcpy(detail.velocity, v.at(1).c_str());
        strcpy(detail.noise_power, v.at(2).c_str());


    }

    printf("The AWS received <%d> matches from Backend-Server <%c> using UDP over port <%d>\n", m, port, AWS_UDP_PORT);


    close(sockfd);

    return 0;
}

int C_Connection (char link_id[]){
        //set up UDP from Beej
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char result[4 * MAXBUFLEN];
	//from beej  http://beej-zhcn.netdpi.net/06-client-server-background
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(localhost, SERVER_C_PORT, &hints, &servinfo))
        != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
            == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }

    memset((char *)&AWS_P, 0, sizeof(AWS_P));
    AWS_P.sin_family = AF_INET;
    AWS_P.sin_port = htons(AWS_UDP_PORT);	
    if(inet_aton(localhost, &AWS_P.sin_addr) == 0){
	printf("inet_aton() failed\n");
	exit(1);
    }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    memset(result,0,sizeof(result));
    string param = "";
    param = string(client_input.size) + "@" + string(client_input.power) + "@" + string(detail.bandwidth) 
            + "@" + string(detail.length) + "@" + string(detail.velocity) + "@" + string(detail.noise_power)
            + "@" + string(client_input.id);

    strcpy(result, param.c_str());


    if ((numbytes = sendto(sockfd, result, sizeof result, 0, p->ai_addr,p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    printf("The AWS sent link ID=<%s>, size=<%s>, power=<%s>, and link information to Backend-Server C using UDP over port <%d>\n", 
            client_input.id, client_input.size, client_input.power, AWS_UDP_PORT);

    memset(result,0,sizeof(result));

    if ((numbytes = recvfrom(sockfd, result, sizeof result, 0 , NULL, NULL)) == -1) {
        perror("talker: recvfrom");
        exit(1);
    }

    strcpy(final_res, result);


    printf("The AWS received outputs from Backend-Server C using UDP over port <%d>\n", AWS_UDP_PORT);



    return 0;
}

int main(int argc, char const *argv[])
{
    int sock_client, new_fd_client, sock_monitor, new_fd_monitor; 
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; 
    socklen_t sin_size;

    int yes=1;
    char s_client[INET6_ADDRSTRLEN];
    int rv_client, rv_monitor;

    char buf[MAXBUFLEN];
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
	//from beej http://beej-zhcn.netdpi.net/06-client-server-background
    if ((rv_client = getaddrinfo(NULL, AWS_CLIENT_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_client));
        return 1;
    }


    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock_client = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sock_client, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sock_client, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock_client);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); 

    if (listen(sock_client, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }


    if ((rv_monitor = getaddrinfo(NULL, AWS_MONITOR_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_monitor));
        return 1;
    }


    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock_monitor = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sock_monitor, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

    if (bind(sock_monitor, p->ai_addr, p->ai_addrlen) == -1) {
        close(sock_monitor);
        perror("server: bind");
        continue;
    }

    break;
    }

    if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); 

    if (listen(sock_monitor, BACKLOG) == -1) {
        perror("listen");
            exit(1);
    }

    flag = true;
    sin_size = sizeof their_addr;
    new_fd_monitor = accept(sock_monitor, (struct sockaddr *) &their_addr, &sin_size);
    if (new_fd_monitor == -1) {
        perror("accept");
        exit(1);
    }
   

    while(1) {
        sin_size = sizeof their_addr;
        new_fd_client = accept(sock_client, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd_client == -1) {
            perror("accept");
            continue;
        }

        printf( "The AWS is up and running.\n");

        inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        s_client, sizeof s_client);

        memset(buf, 0, MAXBUFLEN);

        if ((numbytes = recv(new_fd_client, buf, MAXBUFLEN-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
    
        buf[numbytes] = '\0';


        if ((numbytes = send(new_fd_monitor, buf, MAXBUFLEN-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        memset(client_input.id, '\0', sizeof(client_input.id));
        memset(client_input.size, '\0', sizeof(client_input.size));
        memset(client_input.power, '\0', sizeof(client_input.power));

        char s[2] = "@";    
        char * pch;

        pch = strtok(buf, s);
        strcpy(client_input.id, pch);
        vector<string> v;

        int i = 0;
        while (i < 2)
        {   
            pch = strtok (NULL, s);
            v.push_back(pch);
            i++;
        }

        strcpy(client_input.size, v.at(0).c_str());
        strcpy(client_input.power, v.at(1).c_str());

        printf("The AWS received link ID=<%s>, size=<%s>, and power=<%s> from the client using TCP over port <%s>\n"
            ,client_input.id, client_input.size, client_input.power, AWS_CLIENT_PORT);

       

        printf("The AWS sent link ID=<%s>, size=<%s>, and power=<%s> to the monitor using TCP over port <%s>\n"
            ,client_input.id, client_input.size, client_input.power, AWS_MONITOR_PORT);
        
	flag = true;
        AB_Connection(client_input.id, 'A');
        AB_Connection(client_input.id, 'B');
	
	memset(final_res, 0, MAXBUFLEN);

        if (flag){
            char NF[9] = "NOT FIND";
            strcpy(final_res, NF);
        } else{
            C_Connection(client_input.id);
        }

        memset(buf, 0, MAXBUFLEN);

        strcpy(buf, final_res);

        if (send(new_fd_client, buf, MAXBUFLEN-1, 0) == -1)
            perror("send");
        if (send(new_fd_monitor, buf, MAXBUFLEN-1, 0) == -1)
            perror("send");

        if (string(buf).compare("NOT FIND") != 0){
            memset(c_result.tt, '\0', sizeof(c_result.tt));
            memset(c_result.tp, '\0', sizeof(c_result.tp));
            memset(c_result.delay, '\0', sizeof(c_result.delay));

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

            printf("The AWS sent delay=<%.2f>ms to the client using TCP over port <%s>\n", delay, AWS_CLIENT_PORT);
            printf("The AWS sent detailed results to the monitor using TCP over port <%s>\n", AWS_MONITOR_PORT);
        }else{
            printf("The AWS sent No Match to the monitor and the client using TCP over ports <%s> and <%s>, respectively\n", AWS_MONITOR_PORT, AWS_CLIENT_PORT);
        }
    }
    return 0;
}
