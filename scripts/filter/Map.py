
import premap_pb2 as pb

class Map:
	""" Class for representing a map"""
	nodes = [] 
	ways = []
	relations = []
	multipols = []
	nodesIdx = {}
	waysIdx = {}
	lastnodeid = -1
	lastwayid = -1

	def newNodeid(self):
		""" Generate id for newly created node"""
		self.lastnodeid-=1
		return self.lastnodeid

	def newWayid(self):
		""" Generate id for newly created way"""
		self.lastwayid-=1
		return self.lastwayid
	
	def toPB(self):
		""" Make Protocol buffer from content"""
		pbMap = pb.Map()
		pbMap.nodes.extend(self.nodes)
		pbMap.ways.extend(self.ways)
		pbMap.relations.extend(self.relations)
		pbMap.multipols.extend(self.multipols)
		return pbMap
	
	def loadFromPB(self,filename):
		""" Fill map from Protocol Buffer"""
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
		""" Update mapping node id -> node index"""
		for i in range(len(self.nodes)):
			self.nodesIdx[self.nodes[i].id]=i
	def updateWaysIdx(self):
		""" Update mapping way id -> way index"""
		for i in range(len(self.ways)):
			self.waysIdx[self.ways[i].id]=i

