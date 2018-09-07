#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "main.h"
#include "serial.h"
#include "sys_manager.h"
#include "esl_attr.h"


//#include "can_comx.h"

//#define __TEST__

#ifdef __TEST__
#define DEBUG 1
#include "SDL.h"
#include "chat.h"

//#define CHAT_MAXPEOPLE 5
//#define CHAT_PORT 2222

static TCPsocket servsock = NULL;
static SDLNet_SocketSet socketset = NULL;

static struct {
    int active;
    TCPsocket sock;
    IPaddress peer;
    unsigned char name[256+1];
} people[CHAT_MAXPEOPLE];

static void cleanup(int exitcode)
{
	if ( servsock != NULL ) {
		SDLNet_TCP_Close(servsock);
		servsock = NULL;
	}
	if ( socketset != NULL ) {
		SDLNet_FreeSocketSet(socketset);
		socketset = NULL;
	}
	SDLNet_Quit();
	return;
}



void HandleServer(void)
{
	TCPsocket newsock;
	int which;
	unsigned char data;

	newsock = SDLNet_TCP_Accept(servsock);
	if ( newsock == NULL ) {
		return;
	}

	/* Look for unconnected person slot */
	for ( which=0; which<CHAT_MAXPEOPLE; ++which ) {
		if ( ! people[which].sock ) {
			break;
		}
	}
	
	if ( which == CHAT_MAXPEOPLE ) {
		/* Look for inactive person slot */
		for ( which=0; which<CHAT_MAXPEOPLE; ++which ) {
			if ( people[which].sock && ! people[which].active ) {
				/* Kick them out.. */
				data = CHAT_BYE;
				SDLNet_TCP_Send(people[which].sock, &data, 1);
				SDLNet_TCP_DelSocket(socketset,
				people[which].sock);
				SDLNet_TCP_Close(people[which].sock);
#ifdef DEBUG
				fprintf(stderr, "Killed inactive socket %d\n", which);
#endif
				break;
			}
		}
	}
	if ( which == CHAT_MAXPEOPLE ) {
		/* No more room... */
		data = CHAT_BYE;
		SDLNet_TCP_Send(newsock, &data, 1);
		SDLNet_TCP_Close(newsock);
#ifdef DEBUG
		fprintf(stderr, "Connection refused -- chat room full\n");
#endif
	} else {
	        /* Add socket as an inactive person */
	        people[which].sock = newsock;
	        people[which].peer = *SDLNet_TCP_GetPeerAddress(newsock);
	        SDLNet_TCP_AddSocket(socketset, people[which].sock);
#ifdef DEBUG
		fprintf(stderr, "New inactive socket %d\n", which);
#endif
	}
}


	/* Send a "new client" notification */
void SendNew(int about, int to)
{
	char data[512];
	int n;

	n = strlen((char *)people[about].name)+1;
	data[0] = CHAT_ADD;
	data[CHAT_ADD_SLOT] = about;
	memcpy(&data[CHAT_ADD_HOST], &people[about].peer.host, 4);
	memcpy(&data[CHAT_ADD_PORT], &people[about].peer.port, 2);
	data[CHAT_ADD_NLEN] = n;
	memcpy(&data[CHAT_ADD_NAME], people[about].name, n);
	SDLNet_TCP_Send(people[to].sock, data, CHAT_ADD_NAME+n);
}


void HandleClient(int which)
{
	char data[512];
	int i;

    /* Has the connection been closed? */
	if ( SDLNet_TCP_Recv(people[which].sock, data, 512) <= 0 ) {
#ifdef DEBUG
		fprintf(stderr, "Closing socket %d (was%s active)\n",
			which, people[which].active ? "" : " not");
#endif
	        /* Notify all active clients */
		if ( people[which].active ) {
			people[which].active = 0;
			data[0] = CHAT_DEL;
			data[CHAT_DEL_SLOT] = which;
			for ( i=0; i<CHAT_MAXPEOPLE; ++i ) {
				if ( people[i].active ) {
					SDLNet_TCP_Send(people[i].sock,data,CHAT_DEL_LEN);
				}
			}
		}
		SDLNet_TCP_DelSocket(socketset, people[which].sock);
		SDLNet_TCP_Close(people[which].sock);
		people[which].sock = NULL;
	} else {
		printf("recv:------\n");
		switch (data[0]) {
			case CHAT_HELLO: {
	                /* Yay!  An active connection */
				memcpy(&people[which].peer.port,
					&data[CHAT_HELLO_PORT], 2);
				memcpy(people[which].name,
					&data[CHAT_HELLO_NAME], 256);
				people[which].name[256] = 0;
#ifdef DEBUG
				fprintf(stderr, "Activating socket %d (%s)\n",
				which, people[which].name);
#endif
					 /* Notify all active clients */
					for ( i=0; i<CHAT_MAXPEOPLE; ++i ) {
						if ( people[i].active ) {
							SendNew(which, i);
						}
					}

					/* Notify about all active clients */
					people[which].active = 1;
					for ( i=0; i<CHAT_MAXPEOPLE; ++i ) {
						if ( people[i].active ) {
							SendNew(i, which);
						}
					}
				}
				break;
			default: {
				 /* Unknown packet type?? */;
				}
			break;
		}
	}
}

#endif

extern sys_manager_t g_sys_manager;

void sys_close(void)
{
	g_sys_manager.deinit(&g_sys_manager);
#ifdef __TEST__
	
#endif
}




int main(void)
{
	(void) signal(SIGINT, sys_close); 
	(void) signal(SIGQUIT, sys_close); 
	(void) signal(SIGTERM, sys_close); 
	(void) signal(SIGPIPE, sys_close);
#ifdef __TEST__
	IPaddress serverIP;
	int i;
	if ( SDLNet_Init() < 0 ) {
		fprintf(stderr, "Couldn't initialize net: %s\n",
		SDLNet_GetError());
		return -1;
	}

	for ( i=0; i<CHAT_MAXPEOPLE; ++i ){
		people[i].active = 0;
		people[i].sock = NULL;
	}

	/* Allocate the socket set */
	socketset = SDLNet_AllocSocketSet(CHAT_MAXPEOPLE+1);
	if ( socketset == NULL ) {
		fprintf(stderr, "Couldn't create socket set: %s\n",
		SDLNet_GetError());
		cleanup(2);
	}
//	unsigned char *host = "192.168.0.222";

    /* Create the server socket */
	SDLNet_ResolveHost(&serverIP, NULL, CHAT_PORT);
//	SDLNet_ResolveHost(&serverIP, host, CHAT_PORT);
	printf("Server IP: %x, %d\n", serverIP.host, serverIP.port);
	servsock = SDLNet_TCP_Open(&serverIP);
	if ( servsock == NULL ) {
		fprintf(stderr, "Couldn't create server socket: %s\n",
		SDLNet_GetError());
		cleanup(2);
	}
	SDLNet_TCP_AddSocket(socketset, servsock);

	while (1) {
		/* Wait for events */
		SDLNet_CheckSockets(socketset, ~0);

		/* Check for new connections */
		if ( SDLNet_SocketReady(servsock) ) {
			HandleServer();
		}

		/* Check for events on existing clients */
		for ( i=0; i<CHAT_MAXPEOPLE; ++i ) {
			if ( SDLNet_SocketReady(people[i].sock) ) {
				HandleClient(i);
			}
		}
	}
	cleanup(0);

#else

	g_sys_manager.init(&g_sys_manager);

	while(1){
		g_sys_manager.monitor(&g_sys_manager);
	}
#endif
}


