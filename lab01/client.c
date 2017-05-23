#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h> //strcmp()
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>

#define CLIENT_ID_LENGTH	16
#define MSG_LENGTH			256
#define MULTICAST_ADDR		"228.28.28.28"
#define MULTICAST_PORT		6789

#define CHECK(f, r)                       				         \
    do {                                                         \
        if (f == r)               				                 \
        {                                                        \
            perror(#f);                   		                 \
            exit(0);											 \
        }                                                        \
    } while (0)

struct msg_str {
	char id[CLIENT_ID_LENGTH];
	char msg[MSG_LENGTH];
	int msg_len;
};

int fd_tcp = -1;
int fd_udp = -1;
int fd_multicast = -1;

const char *multimedia_msg = "\n\
░░░░░░░░▄▀▀▀░░░░░░░▀▄░░░░░░░░\n\
░░░░░░░▄▀░░░░░░░░░░░░▀▄░░░░░░\n\
░░░░░░▄▀░░░░░░░░░░▄▀▀▄▀▄░░░░░\n\
░░░░▄▀░░░░░░░░░░▄▀░░██▄▀▄░░░░\n\
░░░▄▀░░▄▀▀▀▄░░░░█░░░▀▀░█▀▄░░░\n\
░░░█░░█▄░░░░█░░░▀▄░░░░░▐░█░░░\n\
░░▐▌░░█▀░░░▄▀░░░░░▀▄▄▄▄▀░░█░░\n\
░░▐▌░░█░░░▄▀░░░░░░░░░░░░░░█░░\n\
░░▐▌░░░▀▀▀░░░░░░░░░░░░░░░░▐▌░\n\
░░▐▌░░░░░░░░░░░░░░░▄░░░░░░▐▌░\n\
░░▐▌░░░░░░░░░▄░░░░░█░░░░░░▐▌░\n\
░░░█░░░░░░░░░▀█▄░░▄█░░░░░░▐▌░\n\
░░░▐▌░░░░░░░░░░▀▀▀▀░░░░░░░▐▌░\n\
░░░░█░░░░░░░░░░░░░░░░░░░░░█░░\n\
░░░░▐▌▀▄░░░░░░░░░░░░░░░░░▐▌░░\n\
░░░░░█░░▀░░░░░░░░░░░░░░░░▀░░░\n\
╦═╗╔═╗╔══║░║╔═╗░░╦═╗╦░╦═╗░░╦░\n\
║░║╠═╣╠═░║░║║░║░░║░║║░║░║░░║░\n\
╩═╝║░║║░░╚═╝╩╩╝░░╩═╝╩░╩═╝░░╩░\n\
═╦║░║╔═╗═╦═░░╔═╗╔═╗╔═╗╦═╗░╔═╗\n\
░║║░║╚═╗░║░░░╠╦╝╠╣░╠═╣║░║░░╔╝\n\
╚╝╚═╝╚═╝░║░░░║╚═╚═╝║░║╩═╝░░▄░\n";


void sigint_hdl (int signum __attribute__((unused)) ) {
	
	CHECK(shutdown(fd_tcp, SHUT_RDWR), -1);
	CHECK(close(fd_tcp), -1);
	printf("Koniec programu\n");
	exit(0);
}

int main(int argc, char **argv)
{

	//id, ip, port
	
	if (argc < 4) {
		printf("Zbyt malo argumentow\n");
		exit(1);
	}
	
	char *p = argv[3];
	while (*p != 0) {
		if (isdigit((int)*p) == 0) {
			printf("Argument 3 powinien byc liczba\n");
			exit(1);
		}
		p++;
	}
	const uint16_t port = atoi(argv[3]);
	struct sockaddr_in netaddr;

	netaddr.sin_family = AF_INET;
	netaddr.sin_port = htons(port);
	CHECK(inet_aton(argv[2], &netaddr.sin_addr), 0);
	
	//TCP
	CHECK((fd_tcp = socket(AF_INET, SOCK_STREAM, 0)), -1);
	CHECK(connect(fd_tcp, (struct sockaddr*)&netaddr, sizeof(netaddr)), -1);
	printf("TCP connect\n");
	
	//UDP
	CHECK((fd_udp = socket(AF_INET, SOCK_DGRAM, 0)), -1);
	CHECK(connect(fd_udp, (struct sockaddr*)&netaddr, sizeof(netaddr)), -1);
	CHECK(send(fd_udp, (void*)multimedia_msg, 0, 0), -1);
	printf("UDP connect\n");
	
	//UDP MULTICAST
	netaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	netaddr.sin_port = htons(MULTICAST_PORT);
	CHECK((fd_multicast = socket(AF_INET, SOCK_DGRAM, 0)), -1);
	if (argc == 5) {
		CHECK(bind(fd_multicast, (struct sockaddr*)&netaddr, sizeof(netaddr)), -1);
	
		//CHECK(inet_aton(MULTICAST_ADDR, &netaddr.sin_addr), 0);
		//CHECK(connect(fd_multicast, (struct sockaddr*)&netaddr, sizeof(netaddr)), -1);
		
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDR);         
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
		CHECK(setsockopt(fd_multicast, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)), -1);
	}
	
	if (fd_tcp == -1 || fd_udp == -1 || fd_multicast == -1) {
		printf("Something goes wrong\n");
	}

	signal(SIGINT, sigint_hdl);
	
	struct pollfd fds[4] = {//Pierwszy to socket, drugi wejście
							{fd_tcp, POLLIN | POLLRDHUP, 0},
							{fd_udp, POLLIN, 0},
							{fd_multicast, POLLIN, 0},
							{0, POLLIN, 0}
						}; 
	
	struct msg_str msg;
	char multimedia_buffer[2048];
	int multimedia_length;
	while(1) {
		CHECK(poll(fds, 4, -1), -1); //To zwraca tez POLLIN gdy socket zamkniety ;(
									//Trzeba uzyc POLLRDHUP z _GNU_SOURCE
		
		if(fds[0].revents & (POLLERR | POLLHUP | POLLRDHUP))
			break;
			
		if(fds[0].revents & POLLIN) {
			CHECK(recv(fd_tcp, (void*)&msg, sizeof(msg), 0), -1);
			printf("%s: %s", msg.id, msg.msg);
			fds[0].revents = 0;	
		}
		
		if (fds[1].revents & POLLIN) {
			CHECK((multimedia_length = recv(fd_udp, (void*)multimedia_buffer, 2048, 0)), -1);
			multimedia_buffer[multimedia_length] = '\0';
			printf("Multimedia data from UDP: \n %s", multimedia_buffer);
			fds[1].revents = 0;
		}
		
		if (fds[2].revents & POLLIN) {
			CHECK((multimedia_length = recv(fd_multicast, (void*)multimedia_buffer, 2048, 0)), -1);
			multimedia_buffer[multimedia_length] = '\0';
			printf("Multimedia data from multicast: \n %s", multimedia_buffer);
			fds[2].revents = 0;
		}
		
		if (fds[3].revents & POLLIN) {
			CHECK((msg.msg_len = read(0, msg.msg, MSG_LENGTH-1)), -1);
			msg.msg[msg.msg_len] = '\0';
			if (strcmp(msg.msg, "m\n") == 0) {
				printf("Multimedia sent to UDP!\n");
				CHECK(send(fd_udp, (void*)multimedia_msg, 1938, 0), -1);
				continue;
			}
			else
			if (strcmp(msg.msg, "n\n") == 0) {
				printf("Multimedia sent to multicast!\n");
				netaddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
				//CHECK(send(fd_multicast, (void*)multimedia_msg, 1938, 0), -1);
				CHECK(sendto(fd_multicast, (void*)multimedia_msg, 1938, 0, (struct sockaddr *) &netaddr, sizeof(netaddr)), -1);
				continue;
			}
			strcpy(msg.id, argv[1]);
			CHECK(send(fd_tcp, (void*)&msg, sizeof(msg), 0), -1);
			fds[3].revents = 0;
		}
	}
	
	kill(getpid(), SIGINT);
	return 0;
}

