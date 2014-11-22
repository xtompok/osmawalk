DROP TABLE ways,ways_refs,multipols,multipols_refs,nodes;

CREATE TABLE nodes (
	id BIGINT PRIMARY KEY,
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

CREATE TABLE ways (
	id BIGINT PRIMARY KEY,
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
	id BIGINT REFERENCES ways(id),
	ref BIGINT, -- REFERENCES nodes(id),
	ord INTEGER
);

CREATE TABLE multipols (
	id BIGINT PRIMARY KEY,
	objtype INTEGER,
	geom GEOMETRY(MULTIPOLYGON,3065)
);

CREATE TABLE multipols_refs (
	id BIGINT REFERENCES multipols(id),
	ref BIGINT, -- REFERENCES ways(id),
	role INTEGER);

CREATE INDEX ON multipols_refs (ref);
CREATE INDEX ON ways_refs (ref);
CREATE INDEX ON nodes (lat, lon);

ALTER TABLE nodes OWNER TO jethro;
ALTER TABLE ways OWNER TO jethro;
ALTER TABLE ways_refs OWNER TO jethro;
ALTER TABLE multipols OWNER TO jethro;
ALTER TABLE multipols_refs OWNER TO jethro;
