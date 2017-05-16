/******************************************************************************
 *
 *  File Name........: listen.c
 *
 *  Description......:
 *	Creates a program that establishes a passive socket.  The socket 
 *  accepts connection from the speak.c program.  While the connection is
 *  open, listen will print the characters that come down the socket.
 *
 *  Listen takes a single argument, the port number of the socket.  Choose
 *  a number that isn't assigned.  Invoke the speak.c program with the 
 *  same port number.
 *
 *  Revision History.:
 *
 *  When	Who         What
 *  09/02/96    vin         Created
 *
 *****************************************************************************/

/*........................ Include Files ....................................*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

main (int argc, char *argv[]) {

	int port, player_count, player_count_temp, a=1, hop_count, s, rc, len, p, i, first_random_player, order, highest, decision_flag=0;
	int id=0;
	struct hostent *hp, *ihp;
	char host[64];
	struct sockaddr_in sin, incoming;
	//int **player;
	fd_set readfds;

	/* check no of parameters passed */
	if ( argc != 4 ) {
		fprintf(stderr, "Usage: %s <port-number> <number-of-players> <hops>\n", argv[0]);
		exit(1);
	}

  	port = atoi(argv[1]);
	player_count = atoi(argv[2]);
	player_count_temp = player_count;
	hop_count = atoi(argv[3]);

	/* check hop count */
	if (hop_count<0) {
		fprintf(stderr, "hop count should be non-negative\n");
		exit(1);
	}
	/* check player count */
	if (player_count<1) {
		fprintf(stderr, "player count should be greater than one\n");
		exit(1);
	}

	/* fill in hostent struct for self */
  	gethostname(host, sizeof(host));
  	hp = gethostbyname(host);
  	if ( hp == NULL ) {
    	fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    	exit(1);
  	}
  	printf("Potato Master on %s\n",host);
	printf("Players = %d\n",player_count);
	printf("Hops = %d\n", hop_count);

 	/* open a socket for listening
  	* 4 steps:
  	*	1. create socket
  	*	2. bind it to an address/port
  	*	3. listen
  	*	4. accept a connection
  	*/

	/* use address family INET and STREAMing sockets (TCP) */
 	s = socket(AF_INET, SOCK_STREAM, 0);
  	if ( s < 0 ) {
    	perror("socket:");
    	exit(s);
    }

    /* set up the address and port */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&a, sizeof(a)) < 0) {
        perror("etsockopt: ");
        close(s);
        exit(1);
    }

    /* bind socket s to address sin */
  	rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
  	if ( rc < 0 ) {
    	perror("bind:");
    	exit(rc);
  	}

  	rc = listen(s, player_count);
  	if ( rc < 0 ) {
    	perror("listen:");
    	exit(rc);
  	}

  	char host_name_arr[player_count+2][64];

  	//Initializing player matrix to store all players details - socket file descriptor and port number for the player
	int player[player_count+1][2];
  	//player = (int **)malloc((player_count+1)*sizeof(int));
	//for(i = 0; i <= player_count; i++)
	//	player[i] = (int*)malloc(2*sizeof(int));

  	while (player_count_temp > 0) {
  		int player_port, confirm_data;

  		len = sizeof(sin);
  		p = accept(s, (struct sockaddr *)&incoming, &len);
		//printf("%d", sizeof(player[id][0]));
  		if ( p < 0 ) {
  			perror("bind:");
  			exit(rc);
  		}

  		ihp = gethostbyaddr((char *)&incoming.sin_addr, sizeof(struct in_addr), AF_INET);
  		id++;
  		printf("player %d is on %s\n", (id-1), ihp->h_name);

  		len = recv(p, &player_port, sizeof(player_port), 0);
		//printf("2: %d\n", len);
		//printf("3: %d\n", player_port);
  		if (len<0) {
  			perror("recv_new");
  			exit(1);
  		}

  		player[id][0] = p;
  		player[id][1] = player_port;
		
  		memset(host_name_arr[id],'\0',64);
  		strcpy(host_name_arr[id], ihp->h_name);
		//printf("%d", sizeof(id));
  		len = send(player[id][0], (char *)&id, sizeof(id),0);
		len = send(player[id][0], (char *)&hop_count, sizeof(hop_count),0);
		len = recv(player[id][0], &confirm_data, sizeof(confirm_data),0);
		len = send(player[id][0], (char *)&player_count, sizeof(player_count),0);

		player_count_temp--;
  	}

  	for (i = 1; i<=id; i++) {
  		int confirm_data;
  		if (i==id) {
  			len = send(player[i][0], (char *)&(player[1][1]), sizeof(player[1][1]), 0);
  			if (len != sizeof(player[1][1])) {
				perror("send");
				exit(1);
  			}
			len = recv(player[i][0], &confirm_data, sizeof(confirm_data),0);
			len = send(player[i][0], host_name_arr[1], 64, 0);
  		} else {
  			len = send(player[i][0], (char *)&(player[i+1][1]), sizeof(player[i+1][1]), 0);
  			if (len != sizeof(player[i+1][1])) {
				perror("send");
				exit(1);
  			}
			len = recv(player[i][0], &confirm_data, sizeof(confirm_data),0);
			len = send(player[i][0], host_name_arr[i+1], 64, 0);
  		}
  	}

  	int confirm_data, seconds = 1;
  	int nudge = 1;

  	len= send(player[1][0], (char *)&(nudge), sizeof(nudge),0);
	if(len!=sizeof(nudge))
	{
		perror("send");
		exit(1);
	}
	len= recv(player[1][0], &confirm_data, sizeof(confirm_data), 0);

	for(i = id; i>0; i--) {
		if(i==1) {
			len = send(player[i][0], (char *)&(nudge), sizeof(nudge),0);
			if(len!=sizeof(nudge))
			{
				perror("send");
				exit(1);
			}
			continue;
		}
		nudge=0;
		len = send(player[i][0], (char *)&(nudge), sizeof(nudge),0);
		if(len!=sizeof(nudge))
		{
			perror("send");
			exit(1);
		}
		len = recv(player[i][0], &confirm_data, sizeof(confirm_data) , 0);
	}

	sleep(seconds);

	if(hop_count == 0) {
		int send_data = 2;
		
		for(i = 1; i <= id; i++)
		{
			len = send(player[i][0],(char *)&(send_data), sizeof(send_data),0);
		}
		close(s);
		for(i = 1; i <= id ; i++)
		{
			close(player[i][0]);
		}
		exit(1);	
	}

	//Randomizing:
	srand(hop_count+player_count);
	first_random_player = ((rand())%id) + 1;
	order = first_random_player-1;
	//v = v%id; v++;

	printf("All players present, sending potato to player %d \n", order);

	//Declaring trace storage:

	char trace[2*hop_count+10];
	memset(trace, '\0', sizeof(char)*(2*hop_count+10));	
	strcat(trace,"");

	int nudge1 = 1;
	len = send(player[first_random_player][0], (char *)&(nudge1), sizeof(nudge1),0);
	len = recv(player[first_random_player][0], &confirm_data, sizeof(confirm_data) , 0);

	//Sending hop_count to the first random player:
	len = send(player[first_random_player][0], (char *)&(hop_count), sizeof(hop_count), 0);
	len = recv(player[first_random_player][0], &confirm_data, sizeof(confirm_data), 0);
	len = send(player[first_random_player][0], (char *)trace, sizeof(trace), 0);

	FD_ZERO(&readfds);
	highest = player[1][0];
	for(i = 1; i <= id; i++) {
		FD_SET(player[i][0], &readfds);
		if(player[i][0] > highest)
			highest = player[i][0];
	}

	char trace2[2*hop_count+10];
	memset(trace2,'\0',sizeof(char)*(2*hop_count+10));
	
	select(highest + 1, &readfds, NULL, NULL, NULL);
	
	for(i = 1; i <= id; i++) {
		if (FD_ISSET(player[i][0] , &readfds)) {
			while(1) {
				len = recv(player[i][0], trace2+decision_flag, sizeof(trace2), 0);
				decision_flag = decision_flag + len;
				if(decision_flag==(2*hop_count+10))
					break;
			}
			decision_flag=0;
			break;
		}
	}
	printf("Trace of potato: \n%s \n", trace2);

	int send_data2 = 2;
	for(i = 1; i <= id; i++)
		len = send(player[i][0], (char *)&(send_data2), sizeof(send_data2),0);

	close(s);

	for(i = 1; i <= id; i++)
		close(player[i][0]);

	exit(0);

}
