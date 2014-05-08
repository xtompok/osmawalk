
import premap_pb2 as pb

class Map:
	nodes = []
	ways = []
	relations = []
	multipols = []
	nodesIdx = {}
	waysIdx = {}
	lastnodeid = -1
	lastwayid = -1

	def newNodeid(self):
		self.lastnodeid-=1
		return self.lastnodeid

	def newWayid(self):
		self.lastwayid-=1
		return self.lastwayid
	
	def toPB(self):
		pbMap = pb.Map()
		pbMap.nodes.extend(self.nodes)
		pbMap.ways.extend(self.ways)
		pbMap.relations.extend(self.relations)
		pbMap.multipols.extend(self.multipols)
		return pbMap
	
	def loadFromPB(self,filename):
		f = open(filename,"r")
		pbMap = pb.Map()
		pbMap.ParseFromString(f.read())
		f.close()
		self.nodes = list(pbMap.nodes)
		self.ways = list(pbMap.ways)
		self.relations = list(pbMap.relations)
		self.multipols = list(pbMap.multipols)
		self.updateNodesIdx()
		self.updateWaysIdx()

	def updateNodesIdx(self):
		for i in range(len(self.nodes)):
			self.nodesIdx[self.nodes[i].id]=i
	def updateWaysIdx(self):
		for i in range(len(self.ways)):
			self.waysIdx[self.ways[i].id]=i
