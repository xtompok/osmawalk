struct obj_attr {
	int objtype;
	int area;
	int tunnel;
	int bridge;
	int height;
	int square1;
	int square2;

};

struct height_map_t {
	int minlon;
	int minlat;
	int maxlon;
	int maxlat;
	int * map;
};
