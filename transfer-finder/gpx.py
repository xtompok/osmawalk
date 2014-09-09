

class GPX:
    def __init__(self,filename):
        self.fd = open(filename,"w")

        self.fd.write('<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>\n')
        self.fd.write('<gpx version="1.1"\n')
        self.fd.write('    creator="Osmawalk - https://github.com/xtompok/osmawalk"\n')
        self.fd.write('    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"\n')
        self.fd.write('    xmlns="http://www.topografix.com/GPX/1/1"\n')
        self.fd.write('    xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd">\n')

    def close(self):
        self.fd.write('</gpx>\n');
        self.fd.close()

    def writeTrack(self,track):
        self.startTrack()
        for point in track:
            self.writeTrkpt(point[0],point[1],point[2])
        self.endTrack()

    def startTrack(self):
        self.fd.write('<trk>\n<trkseg>\n')

    def endTrack(self):
        self.fd.write('</trkseg>\n</trk>\n')

    def writeTrkpt(self,lat,lon,ele):
        self.fd.write('    <trkpt lat="'+str(lat)+'" lon="'+str(lon)+'">\n')
        self.fd.write('        <ele>'+str(ele)+'</ele>\n')
        self.fd.write('    </trkpt>\n')

