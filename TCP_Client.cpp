

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
#include <syslog.h>

#include "TCP_Client.hpp"

int connect(const char *address, int port)
{
  int sockfd = 0;
  struct sockaddr_in serv_addr;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_CRIT, "Error : Could not create socket: %s:%d", address, port);
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0) {
    syslog(LOG_CRIT, "inet_pton error occured");
    return -1;
  }

  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    syslog(LOG_CRIT, "Error : Connect Failed");
    return -1;
  }

  return sockfd;
}
