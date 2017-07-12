#ifndef WRITEGPX_H
#define WRITEGPX_H
/*! Write GPX header to open file
 * @param OUT file where write
 */
void writeGpxHeader(FILE * OUT);

/*! Write GPX footer to open file
 * @param OUT file where write
 */
void writeGpxFooter(FILE * OUT);
/*! Write GPX metadata to open file
 * @param OUT file where write
 */
void writeGpxMetadata(FILE * OUT);
/*! Write starting of a GPX track to open file
 * @param OUT file where write
 */
//void writeGpxStartTrack(FILE * OUT);
/*! Write ending of a GPX track to open file
 * @param OUT file where write
 */
//void writeGpxEndTrack(FILE * OUT);
/*! Write GPX trackpoint to open file
 * @param OUT file where write
 * @param lat Latitude of a point
 * @param lon Longitude of a point
 * @param ele Elevation of a point
 */
void writeGpxStartTrack(FILE * OUT,int num,char * arr,double len, double penalty);
void writeGpxStartTrkSeg(FILE * OUT, int type);

void writeGpxEndTrack(FILE * OUT);

void writeGpxEndTrkSeg(FILE * OUT);

void writeWaypoint(FILE * OUT, double lat, double lon, double ele, char * route, char * arr, char * dep);

void writeGpxTrkpt(FILE * OUT, double lat, double lon, double ele);

#endif
