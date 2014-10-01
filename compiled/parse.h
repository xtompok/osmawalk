
struct value_t {
	char * name;
	int prio;
	int objtype;	
};

struct tag_t {
	char * key;
	struct value_t * values;
};	

struct mapconfig_t {
	ProtobufCEnumDescriptor desc;
	struct tag_t * type;
	struct tag_t * tunnel;
	struct tag_t * bridge;
	struct tag_t * area;
};

struct mapConfItem_t {
	char * key;
	char * value;
	int priority;
	char * name;
};
