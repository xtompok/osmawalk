#include <stdio.h>
#include "writegpx.h"

void writeGpxHeader(FILE * OUT){
	fprintf(OUT,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n\
<gpx version=\"1.1\"\n\
    creator=\"Osmawalk - https://github.com/xtompok/osmawalk\"\n\
    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n\
    xmlns=\"http://www.topografix.com/GPX/1/1\"\n\
    xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n\
");
}
void writeGpxFooter(FILE * OUT){
	fprintf(OUT,"</gpx>");
}
void writeGpxMetadata(FILE * OUT){
	fprintf(OUT,"\n\
<metadata>\n\
<name>Osmawalk track</name>\n\
<desc>Walk track found by Osmawalk</desc>\n\
<author>\n\
<name>Tomas Pokorny</name>\n\
<email id=\"xtompok\" domain=\"gmail.com\" />\n\
<link href=\"https://github.com/xtompok/osmawalk\"><text>Osmawalk on Github</text></link>\n\
</author>\n\
</metadata>\n");
}

void writeGpxStartTrack(FILE * OUT,int num,char * arr,double len, double penalty){
	fprintf(OUT,"<trk>\n\
<name>%d</name>\n\
<extensions>\n\
<arrival>%s</arrival>\n<len>%f</len>\n<penalty>%f</penalty>\n\
</extensions>\n",num,arr,len,penalty);
}
void writeGpxStartTrkSeg(FILE * OUT, int type){
	fprintf(OUT,"<trkseg>\n<extensions>\n<type>%d</type>\n</extensions>\n",type);
}

void writeGpxEndTrack(FILE * OUT){
	fprintf(OUT,"</trk>\n");
}

void writeGpxEndTrkSeg(FILE * OUT){
	fprintf(OUT,"</trkseg>\n");
}

void writeWaypoint(FILE * OUT, double lat, double lon, double ele, char * route, char * arr, char * dep){
	fprintf(OUT,"<wpt lat=%f lon=%f>\n<ele>%f</ele>\n<name>%s (%s)</name>\n<desc>Arrival at %s</desc>\n\
</wpt>\n",lat,lon,ele,route,dep,arr);
	
}

void writeGpxTrkpt(FILE * OUT, double lat, double lon, double ele){
	fprintf(OUT,"\
    <trkpt lat=\"%f\" lon=\"%f\">\n\
        <ele>%f</ele>\n\
    </trkpt>\n",lat,lon,ele);

}
