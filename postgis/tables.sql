DROP TABLE IF EXISTS ways,ways_refs,multipols,multipols_refs,nodes,gtfs_stops CASCADE;

CREATE TABLE nodes (
	id BIGSERIAL PRIMARY KEY,
	lat DOUBLE PRECISION,
	lon DOUBLE PRECISION,
	height INTEGER,
	objtype INTEGER DEFAULT 51,
	inside BOOLEAN DEFAULT FALSE,
	inTunnel BOOLEAN DEFAULT FALSE,
	onBridge BOOLEAN DEFAULT FALSE,
	stopPos BOOLEAN DEFAULT FALSE,
	ref VARCHAR(15),
	nodeidx INTEGER,
	loc GEOMETRY(POINTZ,3065),
	square00 INTEGER,
	square01 INTEGER,
	square10 INTEGER,
	square11 INTEGER,
	walk BOOLEAN DEFAULT FALSE
);

CREATE TABLE ways (
	id BIGSERIAL PRIMARY KEY,
--	refs INTEGER[],
	area BOOLEAN,
	"type" INTEGER,
	bordertype INTEGER,
	crossing INTEGER,
	bridge BOOLEAN,
	tunnel BOOLEAN,
	geom GEOMETRY(LINESTRINGZ,3065)
);

CREATE TABLE ways_refs (
	id BIGSERIAL REFERENCES ways(id),
	ref BIGSERIAL, -- REFERENCES nodes(id),
	ord INTEGER
);

CREATE TABLE multipols (
	id BIGSERIAL PRIMARY KEY,
	objtype INTEGER,
	geom GEOMETRY(MULTILINESTRINGZ,3065)
);

CREATE TABLE multipols_refs (
	id BIGSERIAL REFERENCES multipols(id),
	ref BIGSERIAL, -- REFERENCES ways(id),
	role INTEGER
);

CREATE TABLE gtfs_stops (
	stop_id	VARCHAR(15),
	raptor_id INTEGER,
	stop_name VARCHAR(100),
	stop_lon DOUBLE PRECISION,
	stop_lat DOUBLE PRECISION,
	underground BOOLEAN
);

CREATE INDEX ON multipols_refs (ref);
CREATE INDEX ON ways_refs(ref);
CREATE INDEX ON ways_refs(id);
CREATE INDEX ON ways_refs(ord);
CREATE INDEX ON nodes(id);
CREATE INDEX ON nodes(inside);
CREATE INDEX ON nodes(lat);
CREATE INDEX ON nodes(lat, lon);
CREATE INDEX ON nodes(lon);
CREATE INDEX ON nodes(square00);
CREATE INDEX ON nodes(square01);
CREATE INDEX ON nodes(square10);
CREATE INDEX ON nodes(square11);
CREATE INDEX ON nodes(walk);
CREATE INDEX ON nodes USING GIST(loc);
CREATE INDEX ON ways(id);


ALTER TABLE nodes OWNER TO jethro;
ALTER TABLE ways OWNER TO jethro;
ALTER TABLE ways_refs OWNER TO jethro;
ALTER TABLE multipols OWNER TO jethro;
ALTER TABLE multipols_refs OWNER TO jethro;
