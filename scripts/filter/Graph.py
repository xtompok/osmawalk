
import graph_pb2 as pb

class Graph:
	vertices = []
	edges = []
	
	def toPB(self):
		pbGraph = pb.Graph()
		pbGraph.vertices.extend(self.vertices)
		pbGraph.edges.extend(self.edges)
		return pbGraph
	
	def loadFromPB(self,filename):
		pbGraph = pb.Graph()
		with open(filename,"r") as f:
			pbGraph.ParseFromString(f.read())
		self.vertices = list(pbGraph.vertices)
		self.edges = list(pbGraph.edges)
