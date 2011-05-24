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
#include <string.h>

#include <miniwget.h>
#include <miniupnpc.h>
#include <upnpcommands.h>
#include <upnperrors.h>


int upnp_discover()
{
    const char * rootdescurl = 0;
    const char * multicastif = 0;
    const char * minissdpdpath = 0;
    char lanaddr[64];
    struct UPNPDev *devlist = 0;
    struct UPNPDev *dev;
    struct UPNPUrls urls;
    struct IGDdatas data;
    int r, err;

    // damn programmers who change API prototypes in headers
    // without versioning.

/* #ifdef UPNPDISCOVER_SUCCESS */
    devlist = upnpDiscover(1000, multicastif, minissdpdpath, 0, 0, &err);
/* #else */
/*     devlist = upnpDiscover(1000, multicastif, minissdpdpath, 0); */
/* #endif */

    r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
    if (!r) {
      fprintf(stderr,"no valid UPnP devices found\n");

    } else if (r == 3) { // 3 = an UPnP root device has been found (not an IGD)

      dev = devlist;
      for( dev = devlist; dev; dev = dev->pNext) {

	// parse out ip and port from url
	char ip[256];
	char port[64];
	char tmp[512];
	char *p, *pp;

	memcpy(tmp,dev->descURL,511);
	p = tmp;

	// ip
	do p+=2; while(*p != '/'); p++;
	pp = p; do pp++; while(*pp != ':'); *pp = 0;
	snprintf(ip,255,"%s",p);

	// port
	p = pp+1; pp = p;
	do pp++; while(*pp != '/'); *pp = 0;
	snprintf(port,63,"%s",p);

	fprintf(stderr,"%s\t%s\t%s\t%s\n", dev->st, dev->descURL, ip, port);

      }

      FreeUPNPUrls(&urls);
    }
    freeUPNPDevlist(devlist); devlist = 0;

    return(r);
}

