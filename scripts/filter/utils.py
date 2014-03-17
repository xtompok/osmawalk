import math

def angle(ux,uy,vx,vy):
	try:
		cos = (ux*vx + uy*vy)/(math.sqrt(ux*ux+uy*uy)*math.sqrt(vx*vx+vy*vy))
	except ZeroDivisionError:
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
