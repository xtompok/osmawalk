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
<name>GPS Receiver track log</name>\n\
<desc>Tallinn (car)</desc>\n\
<author>\n\
<name>Michael Collinson</name>\n\
<email id=\"mikes\" domain=\"ayeltd.biz\" />\n\
<link href=\"http://www.ayeltd.biz\"><text>AYE Ltd.</text></link>\n\
</author>\n\
<time>2007-10-02T09:22:06Z</time>\n\
<bounds minlat=\"59.4367664166667\" maxlat=\"59.4440920666666\" minlon=\"24.74394385\" maxlon=\"24.7971432\"/>\n\
</metadata>\n");
}

void writeGpxStartTrack(FILE * OUT){
	fprintf(OUT,"<trk>\n\
<src>Logged by Michael Collinson using EMTAC BTGPS Trine II</src>\n\
<link href=\"http://www.ayeltd.biz\"><text>AYE Ltd.</text></link>\n\
<trkseg>\n\
");
}
void writeGpxEndTrack(FILE * OUT){
	fprintf(OUT,"</trkseg>\n\
</trk>\n");
}
void writeGpxTrkpt(FILE * OUT, double lat, double lon, double ele){
	fprintf(OUT,"\
    <trkpt lat=\"%f\" lon=\"%f\">\n\
        <ele>%f</ele>\n\
        <time>2007-10-02T07:54:30Z</time>\n\
        <fix>3d</fix>\n\
        <hdop>300</hdop><vdop>300</vdop><pdop>300</pdop>\n\
    </trkpt>\n",lat,lon,ele);

}
