
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
	struct tag_t * tags;
};

struct mapConfItem_t {
	char * key;
	char * value;
	int priority;
	int enum_val;
};
