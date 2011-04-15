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

#include <libgen.h>

// our exit codes are shell style: 1 is error, 0 is success
#define ERR 1

// uncomment to debug
#define DEBUG 1


// format (printf style) to be used when generating xml markup
#define UPNP_MSG_FORMAT "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
  "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"" \
  "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n<s:Body>\r\n" \
  "<u:%s xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\r\n"	\
  "<InstanceID>0</InstanceID>\r\n%s\r\n</u:%s>\r\n</s:Body>\r\n</s:Envelope>\r\n"

#define UPNP_HDR_FORMAT "POST /MediaRenderer_AVTransport/control HTTP/1.0\r\n" \
	   "SOAPACTION: \"urn:schemas-upnp-org:service:AVTransport:1#%s\"\r\n" \
	   "CONTENT-TYPE: text/xml ; charset=\"utf-8\"\r\n" \
	   "HOST: %s:%u\r\n" \
	   "Connection: close\r\n" \
	   "Content-Length: %u\r\n" \
	   "\r\n"

char filename[512];
char command[64];
char server[512];
int port = 0;

// messages get rendered in this structure
// allocated and freed with create/free_upnp
typedef struct {
  char *hostname;
  int port;
  int sockfd;

  char *msg;
  char *hdr;
  char *res;

  size_t size;
} upnp_t;




upnp_t *create_upnp() {
  upnp_t *upnp;
  upnp = calloc(1,sizeof(upnp_t));
  upnp->hostname = calloc(256,sizeof(char));
  upnp->port = -1;
  upnp->sockfd = -1;

  upnp->msg = (char*) calloc(1024,sizeof(char));
  upnp->hdr = (char*) calloc(512,sizeof(char));
  upnp->res = (char*) calloc(1401,sizeof(char));

  upnp->size = -1;

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
    error("error: can't open socket (%s)\n", strerror(errno));
    return(-1);
  }

  /* gethostbyname: get the server's DNS entry */
  host = gethostbyname(hostname);
  if (host == NULL) {
    error("error: no such host as %s (%s)\n", hostname, strerror(errno));
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
    error("error: can't connect (%s)\n",strerror(errno));
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

  upnp->size = strlen(upnp->msg);

  snprintf(upnp->hdr,1023,UPNP_HDR_FORMAT,
	   action, upnp->hostname, upnp->port, upnp->size);
}

int send_upnp(upnp_t *upnp) {
  int res;
  int hdrlen = strlen(upnp->hdr);
  res = write(upnp->sockfd,upnp->hdr,hdrlen);
  if(res != hdrlen)
    fprintf(stderr,"send upnp header wrote only %u of %u bytes",res, hdrlen);
  // TODO: check success
  res = write(upnp->sockfd,upnp->msg,upnp->size);
  if(res != upnp->size)
    fprintf(stderr,"send upnp message wrote only %u of %u bytes",res, upnp->size);

#ifdef DEBUG
  fprintf(stderr,"sent %u bytes header, %u bytes message\n",hdrlen, res);
  fprintf(stderr,"header:\n\n%s\n\n",upnp->hdr);
  fprintf(stderr,"message:\n\n%s\n\n",upnp->msg);
#endif

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

int load(upnp_t *upnp, char *file) {
  char meta[1024];
  if(!check_upnp(upnp, __PRETTY_FUNCTION__)) return(0);
  //  render_file_meta(file, meta);
  // TODO
}

int play(upnp_t *upnp) {
  if(!check_upnp(upnp, __PRETTY_FUNCTION__)) return(0);
  render_upnp(upnp,"Play","<Speed>1</Speed>");
  send_upnp(upnp);
  return(1);
}

int stop(upnp_t *upnp) {
  if(!check_upnp(upnp, __PRETTY_FUNCTION__)) return(0);
  render_upnp(upnp,"Stop","");
  send_upnp(upnp);
  return(1);
}

int get_trans_info(upnp_t *upnp) {
  if(!check_upnp(upnp, __PRETTY_FUNCTION__)) return(0);
  render_upnp(upnp,"GetTransportInfo","");
  send_upnp(upnp);
  recv_upnp(upnp);
  return(1);
}


// we use only getopt, no _long
static const char *short_options = "-hvD:s:p:";
void cmdline(int argc, char **argv) {
  command[0] = 0;
  filename[0] = 0;
  server[0] = 0;

  int res, optlen;
  do {
    res = getopt(argc, argv, short_options);
    switch(res) {
    case 'h':
      fprintf(stderr,"usage: %s [options] command [file]\n",argv[0]);
      break;
    case 'v':
      fprintf(stderr,"AVRemote - simple commandline tool to send AVTransport commands over UPNP\n"
	      "version 0.1 (Apr/2011) by Jaromil - Netherlands Media Art Institute\n"
	      "Copyright (C) 2011 NIMk Artlab, License GNU AGPL v3+\n"
	      "This is free software: you are free to change and redistribute it\n");
      exit(0);
      
    case 's':
      snprintf(server,511,"%s",optarg);
      break;

    case 'p':
      sscanf (optarg, "%u", &port);
      break;

    case '?':
      fprintf(stderr,"unrecognized option: %s\n",optarg);
      break;
      
    case 1:
      {
	struct stat filestatus;
	if( stat(optarg,&filestatus) >= 0 ) { // its a file
	  snprintf(filename,511,"%s",optarg);
	} else {
	  snprintf(command,63,"%s",optarg);
	}
      }
      break;
    default:
      break;
    }

  } while(res != -1);

  // check requires args
  if(!command[0]) {
    fprintf(stderr,"command not specified, see %s -h for help\n",argv[0]);
    exit(1);
  }

  if(!port) {
    fprintf(stderr,"port not specified, use -p\n");
    exit(1);
  }

  if(!server[0]) {
    fprintf(stderr,"server not specified, using localhost\n");
    sprintf(server,"%s","localhost");
  }

  
}

int main(int argc, char **argv) {
  int sock, port, n;

  cmdline(argc, argv);

  upnp_t *upnp;
  upnp = create_upnp();

  if ( connect_upnp(upnp, server, port) < 0 ) {
    fprintf(stderr,"error: connection failed\n");
    exit(ERR);
  }  

  fprintf(stderr,"socket: %u\n",upnp->sockfd);

  switch(command[0]) {
  case 'p': // play
    play(upnp);
    break;
  case 's':
    stop(upnp);
    break;
  case 'g':
    get_trans_info(upnp);
    break;
  default:
    fprintf(stderr,"error: command not understood.\n");
    break;
  }

  free_upnp(upnp);

  exit(0);
}
