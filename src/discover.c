
#include <config.h>

#include <stdio.h>
#include <string.h>

#ifdef USE_UPNP

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
    int r;

    devlist = upnpDiscover(1000, multicastif, minissdpdpath, 0);
    
    r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
    if (!r) {
      fprintf(stderr,"no valid UPnP devices found\n");

    } else if (r == 3) { // 3 = an UPnP root device has been found (not an IGD)

      dev = devlist;
      while(dev) {

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
	dev = dev->pNext;
      }

      /* fprintf(stderr,
	      " controlURL: %s\n"
	      " ipcondescURL: %s\n"
	      " controlURL_CIF: %s\n",
	      urls.controlURL, urls.ipcondescURL, urls.controlURL_CIF); */
      
      FreeUPNPUrls(&urls);
    }
    freeUPNPDevlist(devlist); devlist = 0;

    return(r);
}

#endif
