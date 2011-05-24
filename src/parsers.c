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
#include <string.h>


#define MAX 256 // max XML string size

// returns a pointer to the current position in haystack
// for subsequent calls
char *extract_xml(char *dest, char *haystack, const char *needle) {
  char *p, *pp;

  p = strstr(haystack, needle);
  if (!p) {
    fprintf(stderr, "parser cannot find \"%s\" in:\n%s\n",needle, haystack);
    return NULL;
  }
  // as you can see, i had a lot of fun with pointers when i was a kid
  pp = p; do pp+=1; while (*pp != '>'); pp++;
  p = pp; do p+=1; while (*p != '<'); *p = 0;

  snprintf(dest,MAX-1,"%s",pp);

  return(p+1);

}

void GetCurrentTransportActions(char *res) {
  char actions[MAX];
  fprintf(stderr,"#\tactions\n");
  extract_xml(actions, res, "Actions");
  fprintf(stderr,"Act: %s\n");
}

void GetDeviceCapabilities(char *res) {
  char play[MAX];
  char rec[MAX];
  char qual[MAX];
  char *p;

  fprintf(stderr,"#\tPlay\tRec\tQuality\n");
  p = extract_xml(play, res, "PlayMedia");
  p = extract_xml(rec,  p, "RecMedia");
  p = extract_xml(qual, p, "RecQualityModes");
  
  fprintf(stderr,"Caps:\t%s\t%s\t%s\n",play,rec,qual);
}

void GetTransportInfo(char *res) {
  char state[MAX];
  char status[MAX];
  char speed[MAX];
  char *p;
  fprintf(stderr,"#\tstate\tstatus\tspeed\n");

  p = extract_xml(state, res, "CurrentTransportState");

  p = extract_xml(status, p, "CurrentTransportStatus");

  p = extract_xml(speed, p, "CurrentSpeed");

  fprintf(stderr,"TInfo:\t%s\t%s\t%s\n", state, status, speed);
}
  
