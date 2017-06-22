#!/usr/bin/python

from ctypes import *

libsearch = cdll.LoadLibrary("../compiled/libsearch.so")
class PBFResult(Structure):
	_fields_ = [("len",c_long),
		("data",POINTER(c_uint8))]

class Point(Structure):
	_fields_ = [("lat",c_double),
		("lon",c_double),
		("height",c_int),
		("departure",c_uint64),
		("arrival",c_uint64),
		("vertType",c_int),
		("stopIdx",c_int),
		("wayid",c_ulonglong)]

class SearchResult(Structure):
	_fields_ = [("n_points",c_int),
		("points",POINTER(Point)),
		("dist",c_double),
		("time",c_double)]

class BBox(Structure):
	_fields_ = [("minlon",c_double),
		("minlat",c_double),
		("maxlon",c_double),
		("maxlat",c_double)]

prepareData = libsearch.prepareData
prepareData.argtypes = (c_char_p,c_char_p,c_char_p)
prepareData.restype = c_void_p

findPath = libsearch.findPath
findPath.restype = PBFResult
findPath.argtypes = (c_void_p,c_double,c_double,c_double,c_double) 


freePackedPBF = libsearch.freePackedPBF
freePackedPBF.argtypes = (PBFResult,)

#findTransfer = libsearch.findTransfer
#findTransfer.restype = SearchResult
#findTransfer.argtypes = (c_void_p,c_char_p,c_char_p)

getMapBBox = libsearch.getMapBBox
getMapBBox.restype = BBox
getMapBBox.argtypes = (c_void_p,)



#
#struct point_t {
#        double lat;
#        double lon;
#        int height;
#        int type;
#        uint64_t wayid;
#};
#
#/* @struct search_result_t
# * @abstract Struct for handling found path
# * @field n_points Number of points on path
# * @field points Points on path
# * @field dist Travelled distance
# * @field time Time needed for it
# */
#struct search_result_t {
#        int n_points;
#        struct point_t * points;
#        double dist;
#        double time;
#
#};
