import sys
sys.path.append("../ext-lib/mmpf/raptor")
import data_pb2

class Timetable:
	def __init__(self,filename):
		tt = data_pb2.Timetable()
		with open(filename,"rb") as f:
			tt.ParseFromString(f.read())
		self.routes = tt.routes
		self.stops = tt.stops
		self.route_stops = tt.route_stops
		del(tt)

	def get_route_details(self,route_idx):
		return self.routes[route_idx]

	def get_stop_details(self,stop_idx):
		return self.stops[stop_idx]
