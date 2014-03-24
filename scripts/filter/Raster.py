
import pyproj
import math
from utils import int2deg, deg2int


class Raster:
	
	dimension = 20
	minlon = 0
	minlat = 0
	raster = [[]]
	lonparts = 0
	latparts = 0

	def __init__(self,amap):
		self.minlon = 10**10
		self.minlat = 10**10
		maxlon = 0
		maxlat = 0
		for node in amap.nodes:
			self.minlon = min(self.minlon,node.lon)
			self.minlat = min(self.minlat,node.lat)
			maxlon = max(maxlon,node.lon)
			maxlat = max(maxlat,node.lat)
		
		dlon = maxlon-self.minlon
		dlat = maxlat-self.minlat

		geod = pyproj.Geod(ellps="WGS84")
		londist = geod.inv(int2deg(self.minlon),int2deg(self.minlat+dlat/2),
				int2deg(maxlon),int2deg(self.minlat+dlat/2))[2]
		latdist = geod.inv(int2deg(self.minlon+dlon/2),int2deg(self.minlat),
				int2deg(maxlon+dlon/2),int2deg(maxlat))[2]
		
		print "Lon:",self.minlon," -- ",maxlon
		print "Lat:",self.minlon," -- ",maxlon

		self.lonparts = int(math.ceil(londist/self.dimension))+10
		self.latparts = int(math.ceil(latdist/self.dimension))+10

		print "Creating raster",self.lonparts,"x",self.latparts

		self.steplon = int(math.ceil(1.0*dlon/self.lonparts))
		self.steplat = int(math.ceil(1.0*dlat/self.latparts))
		self.raster = [[[] for j in range(self.latparts)] for i in range(self.lonparts)]
		for node in amap.nodes:
			self.raster[(node.lon-self.minlon)/self.steplon][(node.lat-self.minlat)/self.steplat].append(node.id)

	def getBox(self,lon,lat):
		return ((lon-self.minlon)/self.steplon,(lat-self.minlat)/self.steplat)

