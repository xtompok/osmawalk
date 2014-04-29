#ifndef WRITEGPX_H
#define WRITEGPX_H

void writeGpxHeader(FILE * OUT);
void writeGpxFooter(FILE * OUT);
void writeGpxMetadata(FILE * OUT);
void writeGpxStartTrack(FILE * OUT);
void writeGpxEndTrack(FILE * OUT);
void writeGpxTrkpt(FILE * OUT, double lat, double lon, double ele);

#endif
