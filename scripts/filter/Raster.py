
import pyproj
import math
from utils import int2deg, deg2int

scale = 10

class Raster:
	""" Class for representing raster over a map"""	
	dimension = 20
	minlon = 0
	minlat = 0
	raster = [[]]
	lonparts = 0
	latparts = 0

	def __init__(self,amap):
		""" Creates new raster from given map"""
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

		londist = dlon/scale
		latdist = dlat/scale
		
		print "Lon:",self.minlon," -- ",maxlon
		print "Lat:",self.minlat," -- ",maxlat

		self.lonparts = int(math.ceil(londist/self.dimension))+10
		self.latparts = int(math.ceil(latdist/self.dimension))+10

		print "Creating raster",self.lonparts,"x",self.latparts

		self.steplon = int(math.ceil(1.0*dlon/self.lonparts))
		self.steplat = int(math.ceil(1.0*dlat/self.latparts))
		self.raster = [[[] for j in range(self.latparts)] for i in range(self.lonparts)]
		for node in amap.nodes:
			self.raster[(node.lon-self.minlon)/self.steplon][(node.lat-self.minlat)/self.steplat].append(node.id)

	def getBox(self,lon,lat):
		""" Get coordinates of a raster box in which given coordinates lies"""
		return ((lon-self.minlon)/self.steplon,(lat-self.minlat)/self.steplat)

