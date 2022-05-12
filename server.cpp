#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "struct.h"
#include <iostream>
#include "helpers.h"
#include <bits/stdc++.h>
#include <fstream>
#include <netinet/tcp.h>

#define MAXLINE 1024
#define CHUNKSIZE 1024

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

#define INVALID_MESSAGE ("Format invalid pentru mesaj!")
#define SUBSCRIBE_SUCCES_MESSAGE ("You have successfully subscribed to the topic!")
#define UNSUBSCRIBE_SUCCES_MESSAGE ("You have successfully unsubscribed to the topic!")
#define WRONG_COMMAND ("You have inserted a wrong instructuon!")
#define ALREADY_SUBSCRIBED ("You are already subscribed to this topic!")
#define MAX_CLIENTS_NO 100
#define FD_START 0

#define BUFLEN 256 // dimensiunea maxima a calupului de date

std::vector<User> users;

void init_topics()
{
	int i;
	Topic init_topic;
	strcpy(init_topic.name, "INIT");
	strcpy(init_topic.sf, "2");
	for (i = 1; i < MAX_CLIENTS_NO - 1; i++)
	{
		users[i].topics.push_back(init_topic);
	}
}

void init_users()
{
	int i;
	User us;
	us.soket = 0;
	us.connected = 0;
	for (i = 0; i < MAX_CLIENTS_NO + 1; i++)
	{
		users.push_back(us);
	}
	init_topics();
}

std::string convertToString(char *a, int size)
{
	int i;
	std::string s = "";
	for (i = 0; i < size; i++)
	{
		s = s + a[i];
	}
	return s;
}

void recv_from_udp(int sockfd)
{

	char buffer[CHUNKSIZE];
	struct sockaddr_in udp_addr;
	struct UdpPacket udp_data;
	int data_type;

	char *datagram = (char *)&udp_data;
	std::string ch;

	int rc = recvfrom(sockfd, datagram, sizeof(struct UdpPacket) + 1, 0, NULL, NULL);

	memcpy(&data_type, udp_data.topic + 50, 1);

	if (data_type == 0)
	{
		int32_t content[1500];
		u_int32_t sign;
		memcpy(&sign, udp_data.topic + 51, 1);
		memcpy(&content, udp_data.topic + 52, 1500);
		(*content) = ntohl(*content);
		if (sign == 1)
		{
			(*content) = (-1) * (int)(*content);
		}
		ch = std::to_string(*content);
	}

	if (data_type == 1)
	{
		u_int32_t content[1500];
		memcpy(&content, udp_data.topic + 51, 1500);
		*content = (float)((float)htons(*content)) / 100;
		ch = std::to_string((float)*content);
	}

	if (data_type == 2)
	{
		int32_t content[1500];
		u_int32_t sign;
		memcpy(&sign, udp_data.topic + 51, 1);
		memcpy(&content, udp_data.topic + 52, 4);
		(*content) = ntohl(*content);
		if (sign == 1)
		{
			(*content) = (-1) * (int)(*content);
		}
		int count = 0, i, k = 10;
		memcpy(&count, udp_data.topic + 56, 1);
		for (i = 1; i <= count - 1; i++)
		{
			k = k * 10;
		}
		std::cout << "K= " << k << "\n";
		float content_final = (float)(*content) / k;
		ch = std::to_string(content_final);
	}

	if (data_type == 3)
	{
		std::cout << "String"
				  << "\n";
		char content[1500];
		memcpy(&content, udp_data.topic + 51, 1500);
		ch = convertToString(content, 1500);
	}

	char buffer_tcp[BUFLEN];


	char *c = const_cast<char *>(ch.c_str());
	TcpPacket packet_to_send;

	for (int k = 1; k <= users.size(); k++)
	{
		for (int l = 1; l <= users[k].topics.size(); l++)
		{
			if (strcmp(datagram, users[k].topics[l].name) == 0 && users[k].connected == 1)
			{
				strcpy(packet_to_send.msg, c);

				// check if the current user has data in storage and send it
				if (!users[k].topics[l].storage.empty())
				{
					while (!users[k].topics[l].storage.empty())
					{
						TcpPacket packet_from_storage;
						strcpy(packet_from_storage.msg, c);
						packet_from_storage.len = strlen(packet_from_storage.msg + 1);
						send(users[k].soket, buffer_tcp, packet_to_send.len, 0);
						users[k].topics[l].storage.pop_back();
					}
				}

				packet_to_send.len = strlen(packet_to_send.msg) + 1;
				snprintf(buffer_tcp, packet_to_send.len, packet_to_send.msg);
				send(users[k].soket, buffer_tcp, packet_to_send.len, 0);
			}
			// store data if user is dissconected and sf flag is setted
			if (strcmp(datagram, users[k].topics[l].name) == 0 && users[k].connected == 0 && strcmp(users[k].topics[l].sf, "1") == 0)
			{
				users[k].topics[l].storage.push_back(c);
			}
		}
	}
}

// --- TCP ---
void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

void init_clients(int *clients)
{
	int i;

	for (i = 0; i < MAX_CLIENTS_NO; i++)
	{
		clients[i] = 0;
	}
}

void print_clients(int *clients)
{
	int i;

	printf("Clienti conectat: ");
	for (i = 0; i < MAX_CLIENTS_NO; i++)
	{
		if (clients[i])
		{
			printf("%d ", i);
		}
	}
	printf("\n");
}

void write_clients(char *buffer, int *clients)
{
	int i;
	int len;

	snprintf(buffer, BUFLEN - 1, "Clienti conectati: ");
	for (i = 0; i < MAX_CLIENTS_NO; i++)
	{
		if (clients[i])
		{
			len = strlen(buffer);
			snprintf(buffer + len, BUFLEN - len - 1, "%d ", i);
		}
	}
	len = strlen(buffer);
	snprintf(buffer + len, BUFLEN - len - 1, "\n");
}

void add_client(int id, int *clients)
{
	clients[id - FD_START] = 1;
	printf("%d\n", id - FD_START);
}

void rm_client(int id, int *clients)
{
	clients[id - FD_START] = 0;
}

int is_client_connected(int id, int *clients)
{
	return id >= FD_START && clients[id - FD_START];
}

void send_info_to_clients(int *clients)
{
	int i, n;
	char buffer[BUFLEN];

	write_clients(buffer, clients);

	for (i = 0; i < MAX_CLIENTS_NO; i++)
	{
		if (clients[i])
		{
			n = send(i + FD_START, buffer, strlen(buffer) + 1, 0);
			DIE(n < 0, "send");
		}
	}
}

int count(char *str)
{
    int check = 0;
    int counter = 0; 
    while (*str)
    {
        if (*str == ' ' || *str == '\n' || *str == '\t')
            check = 0;

        else if (check == 0)
        {
            check = 1;
            ++counter;
        }
        ++str;
    }
    return counter;
}

void instructions(char *instruction, int i)
{
	int counter = count(instruction);
	char *command;
	command = strtok(instruction, " ");
	char *topic;
	topic = strtok(NULL, " ");
	char buffer_tcp[BUFLEN];
	int len;
	bool verify = 1;


	if (strcmp(command, "subscribe") == 0)
	{
		if(counter == 2) {
			goto jump;
		}
		char *sf;
		sf = strtok(NULL, " ");

		// add info to database
		// check if the topic already exists
		for (int j = 1; j <= users[i].topics.size(); j++)
		{
			if (strcmp(topic, users[i].topics[j].name) == 0)
			{
				len = strlen(ALREADY_SUBSCRIBED) + 1;
				snprintf(buffer_tcp, len, ALREADY_SUBSCRIBED);
				send(i, buffer_tcp, len, 0);
				goto jump;
				break;
			}
		}
		// build new topic
		Topic new_topic;
		strcpy(new_topic.name, topic);
		strcpy(new_topic.sf, sf);

		users[i].topics.push_back(new_topic);

		len = strlen(SUBSCRIBE_SUCCES_MESSAGE) + 1;
		snprintf(buffer_tcp, len, SUBSCRIBE_SUCCES_MESSAGE);
		send(i, buffer_tcp, len, 0);
		goto end;
	}
jump:
	if (strcmp(command, "unsubscribe") == 0)
	{
		for (int k = 0; k <= users[i].topics.size(); k++)
		{
			if (strcmp(users[i].topics[k].name, topic) == -10)
			{
				users[i].topics.erase(users[i].topics.begin() + k);
				users[i].topics.erase(users[i].topics.begin() + k);
				len = strlen(UNSUBSCRIBE_SUCCES_MESSAGE) + 1;
				snprintf(buffer_tcp, len, UNSUBSCRIBE_SUCCES_MESSAGE);
				send(i, buffer_tcp, len, 0);
			}
		}
	}
	else{
			len = strlen(WRONG_COMMAND) + 1;
			snprintf(buffer_tcp, len, WRONG_COMMAND);
			send(i, buffer_tcp, len, 0);
	}
	end:
		int a;
}

bool check_id(char *id)
{
	char *name;
	char *check;
	name = strtok(id, " ");
	check = strtok(NULL, " ");
	if (check == NULL)
	{
		return 1;
	}
	return 0;
}

bool check_user_id(char *id)
{
	int i;
	for (i = 0; i < MAX_CLIENTS_NO; i++)
	{
		if (strcmp(users[i].id, id) == 0)
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	// -- UDP --
	int sock_udp;
	char buffer[MAXLINE];
	struct sockaddr_in serv_addr, cli_addr;

	// UDP
	if ((sock_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("eroare la crearea socketului udp");
		exit(EXIT_FAILURE);
	}

	// TCP
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(&cli_addr, 0, sizeof(cli_addr));

	// Filling server information
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[1]));

	// Bind the socket with the server address UDP
	if (bind(sock_udp, (const struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	// Bind TCP
	int ret = bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	int flag = 1;
	int result = setsockopt(sockfd,		   /* socket affected */
							IPPROTO_TCP,   /* set option at TCP level */
							TCP_NODELAY,   /* name of option */
							(char *)&flag, /* the cast is historical cruft */
							sizeof(int));  /* length of option value */

	// -- UDP --

	// -- TCP --
	int newsockfd, portno, dest;
	char buffer_tcp[BUFLEN], *id;
	struct sockaddr_in serv_addr_tcp, cli_addr_tcp;
	int n, i;
	socklen_t clilen;
	int clients[MAX_CLIENTS_NO];

	fd_set read_fds; 
	fd_set tmp_fds;	 
	int fdmax;		 

	init_clients(clients);

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	FD_SET(sock_udp, &read_fds);
	fdmax = sockfd;

	int dim = 0;
	if (sockfd > dim)
		dim = sockfd;
	if (sock_udp > dim)
		dim = sock_udp;

	init_users();
	std::string to_send;
	char udp_topic;
	FD_SET(0, &read_fds);

	while (1)
	{
		tmp_fds = read_fds;

		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		for (i = 0; i <= fdmax; i++)
		{

			if (FD_ISSET(i, &tmp_fds))
			{
				if (i == sock_udp)
				{
					recv_from_udp(sock_udp);
					break;
				}
				if (i == sockfd)
				{
					printf("TCP");
					clilen = sizeof(cli_addr_tcp);
					newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr_tcp, &clilen);
					DIE(newsockfd < 0, "accept");

					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax)
					{
						fdmax = newsockfd;
					}

					add_client(newsockfd, clients);

					print_clients(clients);
					send_info_to_clients(clients);
					users[i].connected = 1;
				}
				else
				{
					//recieve data
					memset(buffer_tcp, 0, BUFLEN);
					n = recv(i, buffer_tcp, sizeof(buffer_tcp), 0);
					DIE(n < 0, "recv");

					if (n == 0)
					{
						// conection closed
						printf("Socket-ul client %d a inchis conexiunea\n", i);
						users[i].connected = 0;
						rm_client(i, clients);
						close(i);

						print_clients(clients);
						send_info_to_clients(clients);

						// remove socket
						FD_CLR(i, &read_fds);
					}
					else
					{
						// reading instuction and modify the database
						char buffer_tcp_cp[BUFLEN];
						strcpy(buffer_tcp_cp, buffer_tcp);
						if (check_id(buffer_tcp) == 1)
						{
							char *user_id;
							user_id = strtok(buffer_tcp, " ");

							for (int k = 0; k < MAX_CLIENTS_NO; k++)
							{
								if (strcmp(users[k].id, user_id) == 0)
								{
									if (users[k].connected == 0)
									{
										goto reconnect;
									}
									printf("There is a problem\n");
									printf("Client from socket %d has closed the connection!\n", i);
									rm_client(i, clients);

									int len = strlen(WR_ID) + 1;
									snprintf(buffer_tcp, len, WR_ID);
									send(i, buffer_tcp, len, 0);

									print_clients(clients);
									send_info_to_clients(clients);

									FD_CLR(i, &read_fds);
									break;
								}
							}

							printf("New client %s, connected from %s : %d\n", user_id, inet_ntoa(cli_addr_tcp.sin_addr), atoi(argv[1]));
							goto connect;
						reconnect:
							printf("Client %s, reconnected from %s : %d\n", user_id, inet_ntoa(cli_addr_tcp.sin_addr), atoi(argv[1]));
						connect:
							users[i].soket = i;
							users[i].connected = 1;
							strcpy(users[i].id, user_id);
						}
						else
						{
							instructions(buffer_tcp_cp, i);
						}

						ret = sscanf(buffer_tcp, "%d", &dest);
					}
				}
			}
		}
	}
	// Close the socket
	close(sockfd);

	return 0;
}
