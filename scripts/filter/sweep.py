

def nodeVectors(amap,nodeways):
	vectors = {}
	for nodeidx in range(len(nodeways)):
		 for way_id in nodeways[nodeidx]:
			 wayidx = binary_search(amap.ways,lambda way: way.id, way_id)
			 way = amap.ways[wayidx]
			 wayrefs = [amap.nodesidx[r] for r in way.refs]
			 lastindex = 0
			 refs = []
			 if wayrefs.count(nodeidx)>2:
				 print "Error in node "+str(amap.nodes[nodeidx].id)+" on way "+str(way.id)
				 continue
			 for i in range(wayrefs.count(nodeidx)):
				 idx = wayrefs.index(nodeidx,lastindex)
				 refs.append(idx)
				 lastindex = idx
			 for i in refs:
				 if i != 0:
					 node2_idx = wayrefs[i-1]
					 if node2_idx < 0:
						 continue
					 if (amap.nodes[node2_idx].lon > amap.nodes[nodeidx].lon) or ( amap.nodes[node2_idx].lon == amap.nodes[nodeidx].lon  and amap.nodes[node2_idx].lat > amap.nodes[nodeidx].lat):
						 try:
						 	vectors[nodeidx].append((node2_idx,wayidx))
						 except KeyError:
						 	vectors[nodeidx] = [(node2_idx,wayidx)]
				 if i!= len(wayrefs)-1:
					 node2_id = wayrefs[i+1]
					 if node2_idx < 0:
						 continue
					 if (amap.nodes[node2_idx].lon > amap.nodes[nodeidx].lon) or ( amap.nodes[node2_idx].lon == amap.nodes[nodeidx].lon  and amap.nodes[node2_idx].lat > amap.nodes[nodeidx].lat):
						 try:
						 	vectors[nodeidx].append((node2_idx,wayidx))
						 except KeyError:
						 	vectors[nodeidx] = [(node2_idx,wayidx)]
	return vectors



	


"""
Postup:
 - najdi všechny body na poledníku
 - seřaď je od jihu k severu
 - jdi od jihu
   - vezmi bod
   - seřaď jeho sousedy podle úhlu od jihu k severu
   - smaž všechny intervaly, které v něm končí
   - je bod uvnitř intervalu budovy b?
     - je nejnižší hrana hranou b?
       - ne - budova je broken
       - ano
        - je nejvyšší hrana hranou b?
	  - ne - budova je broken
	  - ano 
	    - rozdělit interval
	    - vyřešit vnitřek
   - je bod na kraji intervalu budovy b?
     - horní
       - je nejnižší hrana hranou b?
         - ne - budova je broken
	 - ano
	   - uprav interval
     - dolní
       - je nejvyšší hrana hranou b?
         - ne - budova je broken
	 - ano
	   - uprav interval
   - vyřešit vnitřek

Vyřešit vnitřek:
 - jdi po kružnici od jihu
 - je cesta budova?
  - ne - vezmi další
  - ano - budova b
    - je další cesta budova b?
      - ano - uprav interval
      - ne
        - b je broken
	- ber cesty, dokud nenarazíš na b
	- uprav interval

"""

def sortedNeighs(amap,vectors,nodeidx):
	lat = amap.nodes[nodeidx].lat
	lon = amap.nodes[nodeidx].lon
	try:
		neighs = vectors[nodeidx]
	except KeyError:
		#print "Key Error: "+str(amap.nodes[nodeidx].id)
		return []
	angles = []
	top = []
	bot = []
	for n in neighs:
		nlat = amap.nodes[n[0]].lat
		nlon = amap.nodes[n[0]].lon
		if nlon == lon:
			if nlat>lat:
				top.append(n)
			elif nlat<lat:
				bot.append(n)
			continue
		angle = (1.0*nlat-lat)/(1.0*nlon-lon)
		angles.append((angle,n))
	angles.sort(key=lambda a: a[0])
	sneighs = bot+[a[1] for a in angles]+top
	out = []
	lastidx = 0
	for n in sneighs:
		if n[0] == lastidx:
			if lastidx == 0:
				print "Error in "+str(amap.nodes[n[0]].id)
				continue
			out[-1].append(n)
		else:
			out.append([n])
			lastidx = n[0] 
	return out

class Interval:
	lfrom = 0
	lto = 0
	ufrom = 0
	uto = 0
	way_idx = 0


def isInside(node,intervals):
	if len(intervals) == 0:
		return -1
	idx = binary_search(intervals, lambda interval: interval.lower, node.lat)
	if idx >= 0:
		return idx
	elif intervals[-idx].upper > node.lat:
		return -idx
	else:
		return -1
def updateIntervals(nodes,intervals,lon):
	for i in range(len(intervals)):
		aint = intervals[i]
		dlat = nodes[aint.lto].lat - nodes[aint.lfrom].lat
		dlon = nodes[aint.lto].lon - nodes[aint.lfrom].lon
		scale = (lon - nodes[aint.lfrom].lon)/dlon
		intervals[i].lower = dlat*scale + nodes[aint.lfrom].lat
		dlat = nodes[aint.uto].lat - nodes[aint.ufrom].lat
		dlon = nodes[aint.uto].lon - nodes[aint.ufrom].lon
		scale = (lon - nodes[aint.ufrom].lon)/dlon
		intervals[i].upper = dlat*scale + nodes[aint.ufrom].lat
	return intervals


class Intervals:
	nodes = None
	points = []
	intervals = []
	

	def calcLon(idx1, idx2, lon):
		dlat = nodes[idx2].lat - nodes[idx1].lat
		dlon = nodes[idx2].lon - nodes[idx1].lon
		scale = (lon - nodes[idx1].lon)/dlon
		return dlat*scale + nodes[aint.lfrom].lat
		

	def __init__(self,nodes):
		self.nodes = nodes

	def deleteEnding(self,lon):
		remove = []
		for i in range(len(self.intervals)):
			if self.intervals[i] == None:
				continue
			if self.intervals[i].lto == self.intervals[i].uto and nodes[self.intervals[i].lto].lon == lon:
				remove.append(i)
		i=0
		toidx = 0
		for i in range(len(self.intervals)):
			if i in remove:
				continue
			self.intervals[toidx] = self.intervals[i]
			self.points[toidx] = self.points[i]
			self.points[toidx+1] = self.points[i+1]
			toidx+=1
		for i in range(len(self.intervals)-1,toidx,-1):
			del self.intervals[i]
			del self.points[i]
						
	def isInside(self,nodeidx):
		return binary_search(self.points,int,nodes[nodeidx].lon)
	def update(self,lon):
		if len(self.points)==0:
			return
		self.points[0] = self.calcLon(self.intervals[0].lfrom,self.intervals[0].lto,lon)	
		for i in range(len(self.intervals)):
			self.points[i+1] = self.calcLon(self.intervals[i].ufrom,self.intervals[i].uto,lon)

	def add(self,interval):
		llat = nodes[interval.lfrom].lat
		pos = binary_search(self.points,int,llat)
		if pos>0:
			while i+1<len(self.intervals) and self.points[i+1]==llat:
				pos+=1
			self.intervals.insert(pos,interval)
			self.points.instert(pos,llat)
		if pos<0:
			pos = -pos-1
			if pos!=len(self.intervals):
				self.intervals.insert(pos,None)
				self.points.insert(pos,llat)

				self.intervals.insert(pos,interval)
				self.points.insert(pos,llat)
			if pos!=0:
				self.intervals.insert(pos,None)
				self.points.insert(pos,llat)


def zametani(amap,vectors):
	nodes = sorted(amap.nodes,key=lambda node: node.lon)
	broken = {}
	intervals = Intervals(amap.nodes)
	graph = []
	ends = {}
	nodeidx=0
	nextlon=0
	while nodeidx<len(nodes):
		intervals.deleteEnding(nextlon)
		actual=[nodes[nodeidx]]
		while (nodeidx<len(nodes)-1) and (nodes[nodeidx].lon == nodes[nodeidx+1].lon):
			actual.append(nodes[nodeidx+1])
			nodeidx+=1
		nodeidx+=1
		if nodeidx<len(nodes)-1:
			nextlon = nodes[nodeidx].lon
		else:
			 return graph
		 
		#print nextlon
		#print nodeidx

		actual.sort(key=lambda node: node.lat)
		for node in actual:
			neighs = sortedNeighs(amap,vectors,amap.nodesidx[node.id])
			for ns in neighs:
				for n in ns:
					way = amap.ways[binary_search(amap.ways,lambda way: way.id,n[1])]
					if way.type == pb.Way.BARRIER and way.area == true:
						pass



		intervals.update(nextlon)
"""
			inside = isInside(node,intervals)
			buildings = []
			isbroken = False
			for n in neighs:
				if amap.ways[n[1]].area == False:
					isbroken = True
					continue
				if amap.ways[n[1]].type != pb.Way.BARRIER:
					isbroken = True
					continue
				buildings.append(n)
			if inside == -1:
				buildings.sort(key=lambda b: amap.nodes[b[0]].lat)
				i = 0
				while i+1 < len(buildings):
					if buildings[i][1]!=buildings[i+1][1]:
						print "Error in buildings!"
					aint = Interval()
					aint.lfrom = amap.nodesidx[node.id]
					aint.lto = buildings[i][0]
					aint.ufrom = aint.lfrom
					aint.uto = buildings[i+1][0]
					aint.way_idx = buildings[i][1]
					idx = binary_search(intervals,lambda aint: aint.upper,node.lat)
					intervals.insert(idx+1,aint)

					broken[aint.way_idx] = isbroken

					i+=2
			else:
				buildings.sort(key=lambda b: b[1])
				remove = []
				iidx = 0
				for inter in intervals:
					if inter.uto == node.id and inter.lto == node.id:
						remove.append(i)
					
					bidx = binary_search(buildings,lambda b:b[1],inter.way_idx)
					if inter.uto == node.id and bidx > 0:
						inter.ufrom = amap.nodesidx[node.id]
						inter.uto = buildings[bidx][0]
						buildings.remove(buildings[bidx])

					if inter.lto == node.id and bidx > 0:
						inter.lfrom = amap.nodesidx[node.id]
						i.lto = buildings[bidx][0]
						buildings.remove(buildings[bidx])
					
					if iidx+1 < len(intervals) and inter.uto == intervals[iidx+1].lto and inter.uto == node.id:
						intervals[iidx+1].lfrom = inter.lfrom
						intervals[iidx+1].lto = inter.lto
						remove.append(i)

				buildings.sort(key=lambda b: amap.nodes[b[0]].lat)
				i = 0
				while i+1 < len(buildings):
					if buildings[i][1]!=buildings[i+1][1]:
						print "Error in buildings!"
					aint = Interval()
					aint.lfrom = amap.nodesidx[node.id]
					aint.lto = buildings[i][0]
					aint.ufrom = aint.lfrom
					aint.uto = buildings[i+1][0]
					aint.way_idx = buildings[i][1]
					idx = binary_search(intervals,lambda aint: aint.upper,node.lat)
					intervals.insert(idx+1,aint)

					broken[aint.way_idx] = isbroken

					i+=2

					iidx +=1 
				for inter in remove:
					intervals.remove(inter)

				 
				 
				
		intervals = updateIntervals(amap.nodes,intervals,nextlon)

		"""
