ANALYZE nodes;

DROP TABLE IF EXISTS direct_walk_in_nodes;
CREATE TABLE direct_walk_in_nodes AS (
	SELECT id,loc,square1,square2 FROM nodes
	WHERE walk = true AND inside = false AND inTunnel = false
);

DROP TABLE IF EXISTS direct_underground_nodes;
CREATE TABLE direct_underground_nodes AS (
	SELECT id,loc,square1,square2 FROM nodes
	WHERE walk = true  AND inTunnel = true
);

CREATE INDEX ON direct_walk_in_nodes(id); 
CREATE INDEX ON direct_walk_in_nodes(square1); 
CREATE INDEX ON direct_walk_in_nodes(square2); 
ANALYZE direct_walk_in_nodes;

CREATE INDEX ON direct_underground_nodes(id); 
CREATE INDEX ON direct_underground_nodes(square1); 
CREATE INDEX ON direct_underground_nodes(square2); 
CREATE INDEX ON direct_underground_nodes USING GIST(loc);
ANALYZE direct_underground_nodes;


DROP TABLE IF EXISTS direct_candidates;
CREATE TABLE direct_candidates AS (
	SELECT n1.id AS id1, n2.id AS id2, ST_MakeLine(n1.loc,n2.loc) AS geom, 50 AS objtype
	FROM direct_walk_in_nodes AS n1
	INNER JOIN direct_walk_in_nodes AS n2 ON
		n1.square1 = n2.square1 OR
		n1.square2 = n2.square2
	WHERE n1.id != n2.id AND ST_Distance(n1.loc,n2.loc) < 30
);

CREATE INDEX ON direct_candidates USING GIST(geom);
