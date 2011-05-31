/* AVRemote
   
 (c) 2011 Nederlands Instituut voor Mediakunst (NIMk)
     2011 Denis Roio <jaromil@nimk.nl>

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU Affero General Public License as
     published by the Free Software Foundation, either version 3 of the
     License, or (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU Affero General Public License for more details.
     
     You should have received a copy of the GNU Affero General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>



     
     
     This code is written in C and is reentrant.
     Our main goals are simplicity and speed.

*/

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <poll.h>

#include <libgen.h>
 
#include <errno.h>

#include <avremote.h>



upnp_t *create_upnp() {
  upnp_t *upnp;
  upnp = calloc(1,sizeof(upnp_t));
  upnp->hostname = calloc(MAX_HOSTNAME_SIZE,sizeof(char));
  upnp->port = -1;
  upnp->sockfd = -1;

  upnp->msg = (char*) calloc(MAX_MSG_SIZE,sizeof(char));
  upnp->msglen = 0;
  upnp->hdr = (char*) calloc(MAX_HDR_SIZE,sizeof(char));
  upnp->hdrlen = 0;
  upnp->res = (char*) calloc(MAX_RES_SIZE,sizeof(char));
  upnp->meta = (char*) calloc(MAX_META_SIZE,sizeof(char));

  return(upnp);
} 

void free_upnp(upnp_t *upnp) {
  if(!upnp) {
    fprintf(stderr,"error: upnp object is NULL (%s)",__PRETTY_FUNCTION__);
    return;
  }
  if(upnp->sockfd > 0) close(upnp->sockfd);
  if(upnp->hostname) free(upnp->hostname);
  if(upnp->msg) free(upnp->msg);
  if(upnp->hdr) free(upnp->hdr);
  if(upnp->res) free(upnp->res);
  if(upnp->meta) free(upnp->meta);

  free(upnp);
}

int connect_upnp(upnp_t *upnp) {
  struct sockaddr_in serveraddr;
  //  const struct sockaddr *serveraddr;
  struct hostent *host;
  int sockfd;

  if( upnp->sockfd > 0 ) {
    fprintf(stderr,"error: upnp connection already open on socket %u\n",upnp->sockfd);
    return(-1);
  }

  /* socket: create the socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr,"error: can't open socket (%s)\n", strerror(errno));
    return(-1);
  }

  /* gethostbyname: get the server's DNS entry */
  host = gethostbyname(upnp->hostname);
  if (host == NULL) {
    fprintf(stderr,"error: no such host as %s (%s)\n", upnp->hostname, strerror(errno));
    return(-1);
  }
  
  /* build the server's Internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)host->h_addr, 
	(char *)&serveraddr.sin_addr.s_addr, host->h_length);
  serveraddr.sin_port = htons(upnp->port);
  
  /* connect: create a connection with the server */
  if (connect(sockfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    fprintf(stderr,"error: can't connect (%s)\n",strerror(errno));
    return(-1);
  }

  upnp->sockfd = sockfd;

  return(sockfd);
}

void render_uri_meta(upnp_t *upnp, char *path) {
  char dir[1024], *pdir;
  char file[1024], *pfile;
  char url[1024];
  size_t filesize;

  // TODO: streams

  struct stat fs;
  if( stat(path,&fs) < 0 ) {
    //    fprintf(stderr,"error: cannot load file %s (%s)\n", path, strerror(errno));
    filesize = 0;
  } else
    filesize = fs.st_size;

  strncpy(dir,path,1023);
  pdir = dirname(dir);
  strncpy(file,path,1023);
  pfile = basename(file);
  if( strncmp(path,"http://",7)==0 )
    snprintf(url,1023,"%s",path);
  else
    snprintf(url,1023,"file://%s",path);
  
  
  snprintf(upnp->meta,MAX_META_SIZE-1,UPNP_META_FORMAT, url,
	   pfile, pdir, pfile, filesize, url);
	   

}

void render_upnp(upnp_t *upnp, char *action, char *arg) {
  // blank message first
  snprintf(upnp->msg, MAX_MSG_SIZE-1,UPNP_MSG_FORMAT, 
	   action, arg, action);

  upnp->msglen = strlen(upnp->msg);

  snprintf(upnp->hdr, MAX_HDR_SIZE-1, UPNP_HDR_FORMAT,
	   action, upnp->hostname, upnp->port, upnp->msglen);

  upnp->hdrlen = strlen(upnp->hdr);

}

int send_upnp(upnp_t *upnp) {
  int res;
  res = write(upnp->sockfd,upnp->hdr,upnp->hdrlen);
  if(res != upnp->hdrlen)
    fprintf(stderr,"send upnp header wrote only %u of %u bytes",res, upnp->hdrlen);
  // TODO: check success
  res = write(upnp->sockfd,upnp->msg,upnp->msglen);
  if(res != upnp->msglen)
    fprintf(stderr,"send upnp message wrote only %u of %u bytes",res, upnp->msglen);

  return(1);
}

int recv_upnp(upnp_t *upnp, int timeout) {
  int res;
  struct pollfd fds[1]; /* for the poll */
  fd_set socketSet;
  struct timeval timeval;

  fds[0].fd = upnp->sockfd;
  fds[0].events = POLLRDHUP; // needs _GNU_SOURCE
  res = poll(fds, 1, timeout);
  if (res < 0) {
      fprintf(stderr,"error polling reply: %s\n",strerror(errno));
      return(0);
  } else if (!res) return(0);

  FD_ZERO(&socketSet);
  FD_SET(upnp->sockfd, &socketSet);
  timeval.tv_sec = timeout / 1000;
  timeval.tv_usec = (timeout % 1000) * 1000;
  res = select(FD_SETSIZE, &socketSet, NULL, NULL, &timeval);
  if (res < 0) {
      fprintf(stderr,"error on socket select: %s\n",strerror(errno));
      return(0);
  } else if (!res) return(0);
  

  res = recv(upnp->sockfd, upnp->res, MAX_RES_SIZE-1, 0);
  return(res);
}

int print_upnp(upnp_t *upnp) {
  fprintf(stderr,"header (%u bytes):\n",upnp->hdrlen);
  fprintf(stdout,"%s\n\n", upnp->hdr);
  fprintf(stderr,"message (%u bytes):\n",upnp->msglen);
  fprintf(stdout,"%s\n\n", upnp->msg);
}

