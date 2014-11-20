DROP TABLE ways,ways_refs,multipols,multipols_refs,nodes;
CREATE TABLE ways (
	id INTEGER PRIMARY KEY,
--	refs INTEGER[],
	area BOOLEAN,
	barrier BOOLEAN,
	"type" INTEGER,
	bordertype INTEGER,
	crossing INTEGER,
	bridge BOOLEAN,
	tunnel BOOLEAN,
	wayidx INTEGER,
	render BOOLEAN,
	geom GEOMETRY(LINESTRING,3065)
);

CREATE TABLE ways_refs (
	id INTEGER,
	ref INTEGER
);

CREATE TABLE multipols (
	id INTEGER PRIMARY KEY,
	objtype INTEGER,
	geom GEOMETRY(MULTIPOLYGON,3065)
);

CREATE TABLE multipols_refs (
	id INTEGER,
	ref INTEGER,
	role INTEGER);

CREATE TABLE nodes (
	id INTEGER PRIMARY KEY,
	lat INTEGER,
	lon INTEGER,
	height INTEGER,
	objtype INTEGER,
	inside BOOLEAN DEFAULT FALSE,
	inTunnel BOOLEAN,
	onBridge BOOLEAN,
	nodeidx INTEGER,
	loc GEOMETRY(POINTZ,3065)
);
ALTER TABLE nodes OWNER TO jethro;
ALTER TABLE ways OWNER TO jethro;
ALTER TABLE ways_refs OWNER TO jethro;
ALTER TABLE multipols OWNER TO jethro;
ALTER TABLE multipols_refs OWNER TO jethro;
