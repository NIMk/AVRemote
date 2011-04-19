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

#include <config.h>

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
#include <discover.h>

// our exit codes are shell style: 1 is error, 0 is success
#define ERR 1

// uncomment to debug
#define DEBUG 1

char filename[512];
char command[64];
char server[512];
int port = 0;
int dry_run = 0;
int discover = 0;

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
      fprintf(stderr,
	      "%s %s - send AVTransport commands to UPNP media services\n"
	      "\n"
	      " Copyright (C) 2011 Jaromil @ NIMk.nl Artlab , License GNU AGPL v3+\n"
	      " This is free software: you are free to change and redistribute it.\n"
	      " The latest AVTransport sourcecode is published on <%s>\n"
	      "\n"
	      "Syntax: avremote [options] command [file]\n"
	      "\n"
	      "Commands:\n"
	      "\n"
	      " load        load a file and prepare it for playback\n"
	      " play        start playing the selected file\n"
	      " pause       pause currently running playback\n"
	      " stop        stop playback and return to menu\n"
#ifdef USE_UPNP
	      " discover    search for upnp devices on the network\n"
#endif
	      "\n"
	      "Options:\n"
	      "\n"
	      " -s          network address or hostname of the media server\n"
	      " -p          port on which the UPNP AVTransport daemon is listening\n"
	      " -t          dry run to test without a server (print out rendered xml)\n"
	      "\n"
	      " -h          print this help\n"
	      " -v          version information for this tool\n"
	      "\n"
	      "For more informations on AVRemote read the manual: man avremote\n"
	      "Please report bugs on <http://bugs.dyne.org>.\n",
	      PACKAGE, VERSION, PACKAGE_URL);
      exit(0);

    case 'v':
      fprintf(stderr,"%s - simple commandline tool to send AVTransport commands over UPNP\n"
	      "version %s (Apr/2011) by Jaromil - Netherlands Media Art Institute\n"
	      "Copyright (C) 2011 NIMk Artlab, License GNU AGPL v3+\n"
	      "This is free software: you are free to change and redistribute it\n",
	      PACKAGE, VERSION);
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
      if(!command[0]) {
	snprintf(command,63,"%s",optarg);
      } else
	snprintf(filename,511,"%s",optarg);
      break;
    default:
      break;
    }

  } while(res != -1);

  if(command[0] == 'd') { // discover
    discover = 1;
  } else if(!dry_run) {
    // check requires args
    if(!command[0]) {
      fprintf(stderr,"command not specified, see %s -h for help\n",argv[0]);
      exit(1);
    }
    
    
    // not in dry run nor discovery, check for necessary options
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
  upnp_t *upnp;

  cmdline(argc, argv);


#ifdef USE_UPNP
  if (discover)
    {
      fprintf(stderr,"Performing upnp autodiscovery...\n");
      upnp_discover();
      exit(0);
    }
#endif
  
  upnp = create_upnp();

  if(!dry_run)
    {
      
      if ( connect_upnp (upnp, server, port) < 0 )
	{
	  fprintf(stderr,"can't connect to %s:%u: operation aborted.\n", server, port);
	  exit(ERR);
	}  
      
    } 
  else
    {
      
      sprintf(upnp->hostname,"%s","dry run");
      upnp->port = 0;
      
    }
  
  // command parsing is a cascade switch on single letters
  // this is supposedly faster than strcmp
  switch(command[0]) {

  case 'l': // load url
    render_uri_meta(upnp,filename);
    render_upnp(upnp,"SetAVTransportURI", upnp->meta);
    send_upnp(upnp);
    break;

  case 'p': 

    if(command[1]=='l')
      { // 'pl*' is play
	
	render_upnp(upnp,"Play","<Speed>1</Speed>");
	
      }
    else if (command[1]=='a')
      { // 'pa*' is pause
	
	render_upnp(upnp,"Pause","");
	
      }
    break;

  case 's': // stop
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

  if (dry_run)
    print_upnp(upnp);
  else
    {
      send_upnp(upnp);
      recv_upnp(upnp);
      fprintf(stderr,"%s\n",upnp->res);
    }
  
  free_upnp(upnp);

  exit(0);
}
