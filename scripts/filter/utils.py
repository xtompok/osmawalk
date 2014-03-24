import math
import pyproj

scale = 1000000
geod = pyproj.Geod(ellps="WGS84")

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

def binary_search(alist,func,item):
    lo = 0
    hi = len(alist)
    mid = 1
    while lo < hi:
        mid = (lo+hi)//2
        midval = alist[mid]
        if func(midval) < item:
            lo = mid+1
        elif func(midval) > item: 
            hi = mid
        else:
            return mid
    return -(mid-1)

def int2deg(intdeg):
	return 1.0*intdeg/scale

def deg2int(deg):
	return int(deg*scale)

def distance(node1,node2):
	return geod.inv(int2deg(node1.lat),int2deg(node1.lon),int2deg(node2.lat),int2deg(node2.lon))[2]
