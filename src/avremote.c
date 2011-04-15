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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
 
#include <errno.h>

#include <avremote.h>


upnp_t *create_upnp() {
  upnp_t *upnp;
  upnp = calloc(1,sizeof(upnp_t));
  upnp->hostname = calloc(256,sizeof(char));
  upnp->port = -1;
  upnp->sockfd = -1;

  upnp->msg = (char*) calloc(1024,sizeof(char));
  upnp->msglen = 0;
  upnp->hdr = (char*) calloc(512,sizeof(char));
  upnp->hdrlen = 0;
  upnp->res = (char*) calloc(1401,sizeof(char));


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

  free(upnp);
}

int check_upnp(upnp_t *upnp, const char *caller) {
  int res = 1;
  if(!upnp) {
    fprintf(stderr,"error: upnp object is NULL (%s)",caller);
    res = 0;
  }
  if(!upnp) {
    fprintf(stderr,"error: upnp is not connected (%s)",caller);
    res = 0;
  }
  return(res);
}

int connect_upnp(upnp_t *upnp, char *hostname, int port) {
  struct sockaddr_in serveraddr;
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
  host = gethostbyname(hostname);
  if (host == NULL) {
    fprintf(stderr,"error: no such host as %s (%s)\n", hostname, strerror(errno));
    return(-1);
  }
  
  /* build the server's Internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)host->h_addr, 
	(char *)&serveraddr.sin_addr.s_addr, host->h_length);
  serveraddr.sin_port = htons(port);
  
  /* connect: create a connection with the server */
  if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) {
    fprintf(stderr,"error: can't connect (%s)\n",strerror(errno));
    return(-1);
  }

  snprintf(upnp->hostname, 255, "%s",hostname);
  upnp->port = port;
  upnp->sockfd = sockfd;

  return(sockfd);
}

void render_file_meta() {
  /*
    "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite\""
	   "xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
	   "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">"
	   "<item id=\"2$file\" parentID=\"2$parentDir\" restricted=\"0\">"
	   "<dc:title>$fileName</dc:title><dc:date></dc:date><upnp:class>object.item.imageItem</upnp:class><dc:creator></dc:creator><upnp:genre></upnp:genre><upnp:artist></upnp:artist><upnp:album></upnp:album><res protocolInfo=\"file-get:*:*:*:DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00000000001000000000000000000000\" protection=\"\" tokenType=\"0\" bitrate=\"0\" duration=\"\" size=\"$fileSize\" colorDepth=\"0\" ifoFileURI=\"\" resolution=\"\">$uri</res></item></DIDL-Lite>"
  */
}

void render_upnp(upnp_t *upnp, char *action, char *arg) {
  // blank message first
  snprintf(upnp->msg,1023,UPNP_MSG_FORMAT, 
	   action, arg, action);

  upnp->msglen = strlen(upnp->msg);

  snprintf(upnp->hdr,1023,UPNP_HDR_FORMAT,
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

int recv_upnp(upnp_t *upnp) {
  int res;
  res = read(upnp->sockfd, upnp->res,1400);
#ifdef DEBUG
  fprintf(stderr,"response:\n\n%s\n",upnp->res);
#endif
  return(1);
}

int print_upnp(upnp_t *upnp) {
  fprintf(stderr,"header (%u bytes):\n\n%s\n\n",upnp->hdrlen, upnp->hdr);
  fprintf(stderr,"message (%u bytes):\n\n%s\n\n",upnp->msglen, upnp->msg);
}

int load(upnp_t *upnp, char *file) {
  char meta[1024];
  if(!check_upnp(upnp, __PRETTY_FUNCTION__)) return(0);
  //  render_file_meta(file, meta);
  // TODO
}

