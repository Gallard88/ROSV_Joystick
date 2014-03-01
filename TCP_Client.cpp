

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "TCP_Client.hpp"

int connect(const char *address, int port)
{
  int sockfd = 0;
  char recvBuff[1024];
  struct sockaddr_in serv_addr;

  printf("Connect %s:%d\n", address, port);
  memset(recvBuff, '0',sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Error : Could not create socket \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0) {
    printf("\n inet_pton error occured\n");
    return -1;
  }

  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\n Error : Connect Failed \n");
    return -1;
  }

  return sockfd;
}
