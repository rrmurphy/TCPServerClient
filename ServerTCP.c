/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "10010"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 255 // max number of bytes we can get at once
#define MAXSTRLEN 253

#define DISEMVOWELING (80)
#define CLENGTH (5)
#define UPPERCASING (10)

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void toUpperCase(char* string, int strlen) {
    int i;
    for (i = 0; i < strlen; i++) {
        char c = string[i];
        if (c >= 'a' && c <= 'z') {
            string[i] -= 'a' - 'A';
        }
    }
}

int countConsonants(char* string, int strlen) {
    toUpperCase(string, strlen);
    char consonants[] = "BCDFGHJKLMNPQRSTVWXYZ";
    int consonantCount = 0;
    int i;
    for (i = 0; i < strlen; i++) {
        int j = 0;
        for (j = 0; j < sizeof consonants / sizeof(char); j++) {
            if (string[i] == consonants[j]) {
                consonantCount++;
                break;
            }
        }
    }
    return consonantCount;
}

int removeVowels(char* string, int strlen) {
    // printf("%s\n", string);
    char vowels[] = "AEIOUaeiou";
    int currentChar = 0;
    int i; 
    for (i = 0; 1; i++) {
        outer:
        if (i >= strlen) {
            break;
        }
        char c = string[i];
        int j;
        for (j = 0; j < sizeof vowels / sizeof(char); j++) {
            if (string[i] == vowels[j]) {
                i++;
                goto outer;
            }
        }
        string[currentChar++] = string[i];
    }
    string[currentChar] = 0;
    // printf("%s\n", string);
    return strlen - currentChar;
}

int main(int argc, char *argv[]) {
    // TODO: CHANGE THIS
    char buffer[MAXDATASIZE];
    char string[MAXSTRLEN];
    int nbytes = sizeof buffer;
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
    
    if (argc != 2) {
        perror("Usage: portNo\n");
        return 1;
    }

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, // SOCKET CALLED HERE
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // BIND CALLED HERE
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) { // LISTEN CALLED HERE
		perror("listen");
		exit(1);
	}

	// sa.sa_handler = sigchld_handler; // reap all dead processes
	// sigemptyset(&sa.sa_mask);
	// sa.sa_flags = SA_RESTART;
	// if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	// 	perror("sigaction");
	// 	exit(1);
	// }

	printf("server: waiting for connections...\n");

	while (1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size); // ACCEPT CALLED HERE
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
        
        // DEBUGGING CODE
        //
		// inet_ntop(their_addr.ss_family,
		// 	get_in_addr((struct sockaddr *)&their_addr),
		// 	s, sizeof s);
		// printf("server: got connection from %s\n", s);

        int len = 0;
        int reqId = 0;
        int opCode = 0;
        if (read(new_fd, buffer, nbytes) != -1) {
            len = buffer[0];
            reqId = buffer[1];
            opCode = buffer[2];
            int i;
            for (i = 0; i <= len - 3; i++) {
                string[i] = buffer[i + 3];
            }
            string[i] = 0;
        }

        if (buffer[2] == UPPERCASING) { // Uppercasing
            toUpperCase(string, len - 3);
            int i;
            for (int i = 0; i < len - 3; i++) {
                buffer[i + 2] = string[i];
            }
            buffer[0] = len - 1;
            if (write(new_fd, buffer, len) == -1) {
                perror("send");
            }
        } else if (buffer[2] == CLENGTH) { // cLength
            buffer[0] = 3;
            buffer[2] = countConsonants(string, len - 3);
            // printf("%d\n", buffer[2]);
            if (write(new_fd, buffer, 3) == -1) {
                perror("send");
            }
        } else if (buffer[2] == DISEMVOWELING) { // Disemvoweling
            int removedChars = removeVowels(string, len - 3);
            len = strlen(string);
            int i;
            for (i = 0; i < len; i++) {
                buffer[i + 2] = string[i];
            }
            buffer[0] = len + 2;
            if (write(new_fd, buffer, len + 3) == -1) {
                perror("send");
            }
        }
        close(new_fd);
	}
    close(sockfd);

	return 0;
}
