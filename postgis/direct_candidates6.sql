DROP INDEX IF EXISTS nodes_index_lon;
DROP INDEX IF EXISTS nodes_index_lat;
CREATE INDEX nodes_index_lon ON nodes(lon);
CREATE INDEX nodes_index_lat ON nodes(lat);

DROP TABLE IF EXISTS direct_walk_in_nodes;
CREATE TABLE direct_walk_in_nodes AS (
	SELECT id,loc,square1,square2 FROM nodes
	WHERE walk = true AND inside = false
);

CREATE INDEX ON direct_walk_in_nodes(id); 
CREATE INDEX ON direct_walk_in_nodes(square1); 
CREATE INDEX ON direct_walk_in_nodes(square2); 


DROP TABLE IF EXISTS direct_candidates;
CREATE TABLE direct_candidates AS (
	SELECT n1.id AS id1, n2.id AS id2, ST_MakeLine(n1.loc,n2.loc) AS geom
	FROM direct_walk_in_nodes AS n1
	INNER JOIN direct_walk_in_nodes AS n2 ON
		n1.square1 = n2.square1 OR
		n1.square2 = n2.square2
	WHERE n1.id != n2.id AND ST_Distance(n1.loc,n2.loc) < 30
);
--CREATE INDEX ON direct_precandidates USING GIST(loc1);
--CREATE INDEX ON direct_precandidates USING GIST(loc2);


--DROP TABLE IF EXISTS direct_candidates;
--CREATE TABLE direct_candidates AS (
--   SELECT id1,id2,ST_MakeLine(loc1,loc2) AS geom 
--   FROM direct_precandidates
--  WHERE ST_Distance(loc1,loc2) < 30
--);

--SELECT * FROM make_candidates(444358,5532215,445458,5535Ļ315);
CREATE INDEX ON direct_candidates USING GIST(geom);
