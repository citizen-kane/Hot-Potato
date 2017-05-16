#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <fcntl.h>
#include <sys/select.h>

main (int argc, char *argv[]) {

	int master_port, s, rc, client_s, id, neighbour_s, p, temp, dummy_data, current, highest, temporary, random_neighbour, limit=88, signal1, limit2, check=0, target, decision_flag, oport, pport, len;
	int hop_count, confirm_data = 1, nudge, player_count ;
	struct hostent *master_hp, *client_hp, *neighbour_hp;
	fd_set readfds,tempfds;
	char client[256];
	struct sockaddr_in sin, client_sin, trial, neighbour_sin, incomin;
	char trace_post[2];
	strcpy(trace_post, "");

	socklen_t socket1 = sizeof(incomin);
	char host_name_array[64];


	/* check no of parameters passed */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <host-name> <port-number>\n", argv[0]);
		exit(1);
	}

	master_hp = gethostbyname(argv[1]);
	if (master_hp == NULL) {
		fprintf(stderr, "%s: host not found (%s)\n", argv[0], argv[1]);
		exit(1);
	}
	master_port = atoi(argv[2]);

	/* open a socket for connecting
  	* 4 steps:
  	*	1. create socket
  	*	2. add it to an address/port
  	*	3. connect
  	*	4. accept a connection
  	*/

	/* use address family INET and STREAMing sockets (TCP) */
 	s = socket(AF_INET, SOCK_STREAM, 0);
  	if ( s < 0 ) {
    	perror("socket:");
    	exit(s);
    }
    sin.sin_family = AF_INET;
	sin.sin_port = htons(master_port);
	memcpy(&sin.sin_addr, master_hp->h_addr_list[0], master_hp->h_length);

	rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
	if (rc < 0) {
		perror("connect :");
		exit(rc);
	}

	gethostname(client, sizeof(client));
	client_hp = gethostbyname(client);
	client_s = socket(AF_INET, SOCK_STREAM, 0);
	if ( client_s < 0 ) {
		perror("socket:");
		exit(client_s);
	}
	client_sin.sin_family = AF_INET;
	client_sin.sin_port = htons(0);
	memcpy(&client_sin.sin_addr, client_hp->h_addr_list[0], client_hp->h_length);

	rc = bind(client_s, (struct sockaddr *)&client_sin, sizeof(client_sin));
	if (rc < 0) {
		perror("bind :");
		exit(rc);
	}
	if (getsockname(client_s,(struct sockaddr *)&trial, &socket1) == -1)
    		perror("getsockname");
	else
		oport=ntohs(trial.sin_port);
	len = send(s, (char *)&oport, sizeof(oport), 0);
	
 	len = recv(s, &id, sizeof(id),0);
	if (len < 0) {
		perror("recv11");
		exit(1);
	}
	printf("Connected as player %d\n",id-1);

	len = recv(s, &hop_count, sizeof(hop_count), 0);
	if (len < 0) {
		perror("recv22");
		exit(1);
	}
	

	len = send(s, (char *)&confirm_data, sizeof(confirm_data), 0);
	len = recv(s, &player_count, sizeof(player_count), 0);

	memset(host_name_array, '\0', 64);

	len = recv(s, &pport, sizeof(pport), 0);
	len = send(s, (char *)&confirm_data, sizeof(confirm_data),0);
	len = recv(s, host_name_array, 64, 0);
	host_name_array[63]='\0';

	if (len < 0){
		perror("recv");
		exit(1);
	}

	if(strcmp(host_name_array, "localhost")==0)
		neighbour_hp = gethostbyname(client);
	else
		neighbour_hp = gethostbyname(host_name_array);
	if ( neighbour_hp == NULL) {
		fprintf(stderr, "%s: host not found (%s)\n", argv[0], client);
		exit(1);
  	}

  	rc = listen(client_s, 2);
	if (rc < 0) {
		perror("listen:");
		exit(rc);
	}

	neighbour_s = socket(AF_INET, SOCK_STREAM, 0);
	if (neighbour_s < 0) {
		perror("socket:");
		exit(s);
	}

	neighbour_sin.sin_family = AF_INET;
	neighbour_sin.sin_port = htons(pport);
	memcpy(&neighbour_sin.sin_addr, neighbour_hp->h_addr_list[0], neighbour_hp->h_length);

	len = recv(s, &nudge, sizeof(nudge), 0);
	if ( len < 0 ) {
		perror("recv");
		exit(1);
	}

	int confirm_data1=1;

	if (nudge==1) {
		len = send(s, (char *)&confirm_data1, sizeof(confirm_data1), 0);
		if(len!=sizeof(confirm_data1)) {
			perror("send");
		}

		p = accept(client_s, (struct sockaddr *)&incomin, &socket1);
		if (p<0) {
			perror("error at accept :");
			exit(rc);
		}

		temp=p;
		len = recv(s, &dummy_data, sizeof(dummy_data), 0);
		rc = connect(neighbour_s, (struct sockaddr *)&neighbour_sin, sizeof(neighbour_sin));
		if (rc < 0) {
			perror("connect :");
			exit(rc);
		}
	} else {

		rc = connect(neighbour_s, (struct sockaddr *)&neighbour_sin, sizeof(neighbour_sin));
		if (rc < 0) {
			perror("connect :");
			exit(rc);
		}
		len = send(s, (char *)&confirm_data1, sizeof(confirm_data1), 0);
		p = accept(client_s, (struct sockaddr *)&incomin, &socket1);
		temp = p;
		if (p < 0) {
			perror("error at accept 2:");
			exit(rc);
		}
	}

	FD_ZERO(&readfds);
	highest = s;	
	FD_SET(s, &readfds);
	FD_SET(neighbour_s, &readfds);
	if(neighbour_s > highest)
		highest = neighbour_s;
	FD_SET(temp, &readfds);
	if(temp > highest)
		highest = temp;

	sprintf(trace_post, "%d",id-1);
	char trace[2*hop_count+10];
	memset(trace, '\0', sizeof(char)*(2*hop_count+10));

	while(1)
	{	
		tempfds = readfds;
		temporary = select(highest + 1, &tempfds, NULL, NULL, NULL);
		if (FD_ISSET(s, &tempfds))
			current = s;
		else if(FD_ISSET(temp , &tempfds))
			current = temp;
		else if(FD_ISSET(neighbour_s, &tempfds))
			current = neighbour_s;

		len = recv(current, &check, sizeof(check), 0);
		if(check==2)
			break;
		else
		{
			len = send(current, (char *)&limit, sizeof(limit), 0);	
			len = recv(current, &limit2, sizeof(limit2), 0);
			decision_flag=0;
			len = send(current, (char *)&limit, sizeof(limit),0);
			
			while(1) {
				len = recv(current, trace+decision_flag, sizeof(trace),0);
				decision_flag += len;
				if(decision_flag == (2*hop_count+10))
					break;
			}
			decision_flag = 0;

			limit2 = limit2 - 1;

			if(limit2==0)
			{
				strcat(trace, trace_post);
				len = send(s, trace, sizeof(trace),0);
				printf("I'm it\n");
				continue;
			}
			else
			{
				int neighbour_id1, neighbour_id2;
				strcat(trace, trace_post);
				strcat(trace, ",");

				srand(hop_count+player_count);				
				random_neighbour = (rand())%2;
				if(random_neighbour == 0) {
					neighbour_id1=1;
					target = temp;
				}
				else {
					neighbour_id1=2;
					target = neighbour_s;
				}
				if(id==1) {
					if(neighbour_id1==2)
						neighbour_id2=id;
					else if (neighbour_id1==1)
						neighbour_id2=player_count-1;
				}
				else if(id==player_count) {
					if(neighbour_id1==1)
						neighbour_id2=id-2;
					else if(neighbour_id1==2)
						 neighbour_id2=0;	 
				}
				else {
					if(neighbour_id1==2)
						neighbour_id2=id;
					else if(neighbour_id1==1)
						neighbour_id2=id-2;
				}

				printf("Sending potato to %d \n",neighbour_id2);
				signal1 = 1;
				len = send(target, (char *)&signal1, sizeof(signal1),0);
				len = recv(target, &limit, sizeof(limit) , 0);
				len = send(target, &limit2, sizeof(limit2),0);
				len = recv(target, &limit, sizeof(limit) , 0);
				len = send(target, trace, sizeof(trace),0);
				continue;
			}
		}  	
	}

	close(client_s);
	close(neighbour_s);
	close(s);
	exit(0);
	


}
