#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <sys/types.h>*/
/*#include <sys/socket.h>*/
#include <sys/fcntl.h>
/*#include <netdb.h>*/
#include <netinet/in.h>
#include <unistd.h>

#define PORT 1201
#define BUF_SIZE 1024

int msg(int fd, char *msg)
{
	int len;
	len = strlen(msg);
	write(fd, msg, len);
	return len;
}

int http(int sock_fd)
{
	int len;
	int read_fd;
	int request_size;
	int scaned_size;
	char buf[BUF_SIZE];
	char meth_name[16];
	char url_addr[256];
	char http_ver[64];
	char *url_file;

	/*request -> buf*/
	request_size = read(sock_fd, buf, BUF_SIZE);
    if (request_size < 0){
        perror("Error: Can't read a request");
      	return -1;
    }
	
	/*buf -> meth_name, uri_addr, http_ver*/
    scaned_size = sscanf(buf, "%s %s %s", meth_name, url_addr, http_ver);
	if (scaned_size < 0){
		perror("Error: Can't scan buffer");
		return -1;
	}
	if (strcmp(meth_name, "GET") != 0){
		msg(sock_fd, "405 Method Not Allowed\n");
		perror("Error: Unsupported Method");
		return 405;
	}

	read_fd = open(url_addr+1, O_RDONLY, 0666);
	if (read_fd == -1){
		msg(sock_fd, "404 Not Found\n");
		return 404;
	}

	msg(sock_fd, "HTTP1.1 200 OK\r\ntext/html\r\n");
	len = read(read_fd, buf, BUF_SIZE);
	write(sock_fd, buf, len);

	close(read_fd);
}

int main(void)
{
	int read_sock_fd;
	int write_sock_fd;
	struct sockaddr_in read_addr;
	struct sockaddr_in write_addr;
	int binded;
	int listened;
	int write_len;
	read_addr.sin_family = AF_INET;
	read_addr.sin_port = htons(PORT);
	read_addr.sin_addr.s_addr = INADDR_ANY;

	read_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (read_sock_fd < 0){
		perror("Error: Can't make socket");
		return -1;
	}

	binded = bind(read_sock_fd, (struct sockaddr *)&read_addr, sizeof(read_addr));
	if (binded < 0) {
		perror("Error: Can't bind socket");
		close(read_sock_fd);
		return -1;
	}

	listened = listen(read_sock_fd, 5);
	if (listened < 0) {
		perror("Error: Can't listen");
		close(read_sock_fd);
		return -1;
	}

/*	while (1) {*/
		write_len = sizeof(write_addr);
		write_sock_fd = accept(read_sock_fd, (struct sockaddr *)&write_addr, &write_len);
		if (write_sock_fd < 0) {
			perror("Error: Can't accept socket");
			return -1;
		}

		http(write_sock_fd);
		close(write_sock_fd);
		close(read_sock_fd);
/*	}*/

	return 0;
}

