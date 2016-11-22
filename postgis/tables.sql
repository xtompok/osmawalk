DROP TABLE IF EXISTS ways,ways_refs,multipols,multipols_refs,nodes CASCADE;

CREATE TABLE nodes (
	id BIGSERIAL PRIMARY KEY,
	lat INTEGER,
	lon INTEGER,
	height INTEGER,
	objtype INTEGER DEFAULT 51,
	inside BOOLEAN DEFAULT FALSE,
	inTunnel BOOLEAN DEFAULT FALSE,
	onBridge BOOLEAN DEFAULT FALSE,
	nodeidx INTEGER,
	loc GEOMETRY(POINTZ,3065),
	square1 INTEGER,
	square2 INTEGER,
	walk BOOLEAN DEFAULT FALSE
);

CREATE TABLE ways (
	id BIGSERIAL PRIMARY KEY,
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
	role INTEGER);

CREATE INDEX ON multipols_refs (ref);
CREATE INDEX ON ways_refs(ref);
CREATE INDEX ON ways_refs(id);
CREATE INDEX ON ways_refs(ord);
CREATE INDEX ON nodes (lat, lon);
CREATE INDEX ON nodes (lat);
CREATE INDEX ON nodes (lon);
CREATE INDEX ON nodes(square1);
CREATE INDEX ON nodes(square2);
CREATE INDEX ON nodes(walk);
CREATE INDEX ON nodes(inside);
CREATE INDEX ON nodes(id);
CREATE INDEX ON nodes USING GIST(loc);
CREATE INDEX ON ways(id);


ALTER TABLE nodes OWNER TO jethro;
ALTER TABLE ways OWNER TO jethro;
ALTER TABLE ways_refs OWNER TO jethro;
ALTER TABLE multipols OWNER TO jethro;
ALTER TABLE multipols_refs OWNER TO jethro;
