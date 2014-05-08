import math

scale = 10

def angle(ux,uy,vx,vy):
	try:
		cos = 1.0*(ux*vx + uy*vy)/(math.sqrt(ux*ux+uy*uy)*math.sqrt(vx*vx+vy*vy))
	except ZeroDivisionError:
		print "ZDE"
		return 0
	try:
		angle = math.acos(cos)
	except ValueError:
		if cos>0.99:
			angle=0
		if cos<0.01:
			angle=math.pi
	d = ux*vy-uy*vx
	if d>=0:
		angle=2*math.pi-angle
	return angle

def int2deg(intdeg):
	return 1.0*intdeg/scale

def deg2int(deg):
	return int(deg*scale)

def distance(node1,node2):
        return math.sqrt((node1.lat-node2.lat)*(node1.lat-node2.lat)+(node1.lon-node2.lon)*(node1.lon-node2.lon))/scale;

def nodeWays(amap):
	missingcnt=0
	nodeways = [ [] for i in amap.nodes]
	for way in amap.ways:
		missing = []
		for node in way.refs:
			try:
				nodeways[amap.nodesIdx[node]].append(way.id)
			except KeyError:
				missingcnt+=1
				missing.append(node)
		for node in missing:
			way.refs.remove(node)
	print "Missing "+str(missingcnt)+" nodes"

	return nodeways	

def deleteAloneNodes(amap,nodeways):
	toidx = 0
	for fromidx in range(len(amap.nodes)):
		if len(nodeways[fromidx])==0:
			continue
		amap.nodes[toidx]=amap.nodes[fromidx]
		toidx+=1
	for i in range(len(amap.nodes)-1,toidx,-1):
		del amap.nodes[i]
	return amap
