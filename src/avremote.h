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

#ifndef __AVREMOTE_H__
#define __AVREMOTE_H__

/* Buffer Boundaries
   the following defines set the maximum size we allow for buffers used */
#define MAX_HOSTNAME_SIZE 256
#define MAX_MSG_SIZE 2048
#define MAX_HDR_SIZE 512
#define MAX_RES_SIZE 1401
#define MAX_META_SIZE 2048

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
  size_t reslen;

  char *meta;

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

#define UPNP_META_FORMAT "<CurrentURI>%s</CurrentURI>" \
  "<CurrentURIMetaData><DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite\"" \
  "xmlns:dc=\"http://purl.org/dc/elements/1.1/\"" \
  "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\">" \
  "<item id=\"2%s\" parentID=\"2%s\" restricted=\"0\">" \
  "<dc:title>%s</dc:title><dc:date></dc:date>" \
  "<upnp:class>object.item.imageItem</upnp:class><dc:creator></dc:creator>" \
  "<upnp:genre></upnp:genre><upnp:artist></upnp:artist><upnp:album></upnp:album>" \
  "<res protocolInfo=\"file-get:*:*:*:DLNA.ORG_OP=01;DLNA.ORG_CI=0;" \
  "DLNA.ORG_FLAGS=00000000001000000000000000000000\" protection=\"\" tokenType=\"0\" " \
  "bitrate=\"0\" duration=\"\" size=\"%u\" colorDepth=\"0\" ifoFileURI=\"\" " \
  "resolution=\"\">%s</res></item></DIDL-Lite></CurrentURIMetaData>"

upnp_t *create_upnp();
void free_upnp(upnp_t *upnp);

int upnp_discover(upnp_t *upnp);

// should set upnp-> hostname and port before calling this
int connect_upnp(upnp_t *upnp);

/*
Available AVTransport actions:

GetCurrentTransportActions
GetDeviceCapabilities
GetMediaInfo
GetPositionInfo
GetTransportInfo
GetTransportSettings
Next
Pause
Play
Previous
Seek <SeekMode> <SeekTarget> (allowed SeekMode: "X_DLNA_REL_BYTE", "REL_TIME", "TRACK_NR")
SetAVTransportURI <URI> <URIMetaData> (allowed URI: "http://server/file", "file:///folder/file"
SetPlayMode <NewPlayMode> (allowed NewPlayMode = "NORMAL", "REPEAT_ONE", "REPEAT_ALL", "RANDOM")
Stop
X_DLNA_GetBytePositionInfo

Available RenderingControl actions:

GetMute
GetVolume
SetMute <DesiredMute> (allowed DesiredMute = 0 or 1)
SetVolume <DesiredVolume> (allowed DesiredVolume = 1 to 100)
*/
   
void render_upnp(upnp_t *upnp, char *action, char *arg);
void render_uri_meta(upnp_t *upnp, char *path);

int send_upnp(upnp_t *upnp);
int recv_upnp(upnp_t *upnp, int timeout);
int print_upnp(upnp_t *upnp);

#endif
