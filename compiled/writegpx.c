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

void writeGpxStartTrack(FILE * OUT){
	fprintf(OUT,"<trk>\n\
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
    </trkpt>\n",lat,lon,ele);

}
