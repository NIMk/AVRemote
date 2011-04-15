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

// our exit codes are shell style: 1 is error, 0 is success
#define ERR 1

// uncomment to debug
#define DEBUG 1

char filename[512];
char command[64];
char server[512];
int port = 0;
int dry_run = 0;

// we use only getopt, no _long
static const char *short_options = "-hvs:p:t";

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

    case 't':
      // test dry run
      dry_run = 1;
      fprintf(stderr,"dry run: printint out rendered upnp message without establishing connection\n");
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
	  fprintf(stderr,"executing command '%s'\n",command);
	}
      }
      break;
    default:
      break;
    }

  } while(res != -1);

  if(!dry_run) {
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
  
}

int main(int argc, char **argv) {
  int sock, port, n;

  cmdline(argc, argv);

  upnp_t *upnp;
  upnp = create_upnp();

  if(!dry_run) {

    if ( connect_upnp(upnp, server, port) < 0 ) {
      fprintf(stderr,"can't connect to server %s: operation aborted.\n", server);
      exit(ERR);
    }  

  } else {

    sprintf(upnp->hostname,"%s","dry run");
    upnp->port = 0;

  }

  // command parsing is a cascade switch on single letters
  // this is supposedly faster than strcmp
  switch(command[0]) {
  case 'p': 

    if(command[1]=='l') { // 'pl*' is play

      render_upnp(upnp,"Play","<Speed>1</Speed>");
      
    } else if(command[1]=='a') { // 'pa*' is pause

      render_upnp(upnp,"Pause","");

    }
    break;

  case 's':
    render_upnp(upnp,"Stop","");
    break;
  case 'g':
    render_upnp(upnp,"GetTransportInfo","");
    break;
  default:
    fprintf(stderr,"error: command not understood.\n");
    free_upnp(upnp);
    exit(1);
  }

  if(dry_run)
    print_upnp(upnp);
  else
    send_upnp(upnp);

  // TODO recv when needed

  free_upnp(upnp);

  exit(0);
}
