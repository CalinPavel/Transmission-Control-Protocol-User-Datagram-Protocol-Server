#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"
#include <netinet/tcp.h>

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	fd_set read_fds, tmp_fds;

	if (argc < 4)
	{
		perror("Usage ./subscriber <ID_Client> <IP_Server> <Port_Server>");
		exit(0);
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	int flag = 1;
	int result = setsockopt(sockfd,		   /* socket affected */
							IPPROTO_TCP,   /* set option at TCP level */
							TCP_NODELAY,   /* name of option */
							(char *)&flag, /* the cast is historical cruft */
							sizeof(int));  /* length of option value */

	serv_addr.sin_family = AF_INET;
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	serv_addr.sin_port = htons(atoi(argv[3]));
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	// send the id/username with a new packet 
	int len = strlen(argv[1]);
	if (send(sockfd, argv[1], len, 0) < 0)
	{
		perror("eroare la send");
		exit(0);
	}

	while (1)
	{
		tmp_fds = read_fds;

		ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &tmp_fds))
		{
			// reading data from human user
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0)
			{
				break;
			}

			// sending a message to the server
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");
		}

		if (FD_ISSET(sockfd, &tmp_fds))
		{
			memset(buffer, 0, BUFLEN);
			n = recv(sockfd, buffer, BUFLEN, 0);
			DIE(n < 0, "recv");

			if (strcmp(buffer, WR_ID) == 0)
			{
				printf("Wrong id");
				exit(0);
				break;
			}
			printf("From server: %s\n", buffer);
		}
	}

	close(sockfd);

	return 0;
}
