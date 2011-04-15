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

// messages get rendered in this structure
// allocated and freed with create/free_upnp
typedef struct {
  char *hostname;
  int port;
  int sockfd;

  char *hdr;
  size_t hdrlen;

  char *msg;
  size_t msglen;

  char *res;

} upnp_t;

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

upnp_t *create_upnp();
void free_upnp(upnp_t *upnp);

int connect_upnp(upnp_t *upnp, char *hostname, int port);
void render_upnp(upnp_t *upnp, char *action, char *arg);

int send_upnp(upnp_t *upnp);
int recv_upnp(upnp_t *upnp);
int print_upnp(upnp_t *upnp);






int check_upnp(upnp_t *upnp, const char *caller);
