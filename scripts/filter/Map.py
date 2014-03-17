
import premap_pb2 as pb

class Map:
	nodes = []
	ways = []
	relations = []
	nodesidx = {}
	waysidx = {}
	
	def toPB(self):
		pbMap = pb.Map()
		pbMap.nodes.extend(self.nodes)
		pbMap.ways.extend(self.ways)
		pbMap.relations.extend(self.relations)
		return pbMap
	
	def loadFromPB(self,filename):
		f = open(filename,"r")
		pbMap = pb.Map()
		pbMap.ParseFromString(f.read())
		f.close()
		self.nodes = list(pbMap.nodes)
		self.ways = list(pbMap.ways)
		self.relations = list(pbMap.relations)
		for i in range(len(self.nodes)):
			self.nodesidx[self.nodes[i].id]=i
		for i in range(len(self.ways)):
			self.waysidx[self.ways[i].id]=i
