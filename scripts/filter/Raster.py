
class Raster:
	
	parts = 200
	minlon = 0
	minlat = 0
	raster = [[]]

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
		print "Lon:",self.minlon," -- ",maxlon
		print "Lat:",self.minlon," -- ",maxlon

		self.steplon = dlon/self.parts
		self.steplat = dlat/self.parts
		self.raster = [[[] for j in range(self.parts+1)] for i in range(self.parts+10)]
		for node in amap.nodes:
			self.raster[(node.lon-self.minlon)/self.steplon][(node.lat-self.minlat)/self.steplat].append(node.id)

	def getBox(self,lon,lat):
		return ((lon-self.minlon)/self.steplon,(lat-self.minlat)/self.steplat)

