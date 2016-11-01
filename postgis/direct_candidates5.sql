DROP INDEX IF EXISTS nodes_index_lon;
DROP INDEX IF EXISTS nodes_index_lat;
CREATE INDEX nodes_index_lon ON nodes(lon);
CREATE INDEX nodes_index_lat ON nodes(lat);


DROP TABLE IF EXISTS direct_candidates;
CREATE TABLE direct_candidates AS (
	WITH sq AS(
	 SELECT DISTINCT n1.id AS id1, n2.id AS id2, n1.loc AS loc1, n2.loc AS loc2
		FROM nodes AS n1
		INNER JOIN walk_nodes AS wn1 ON wn1.id = n1.id
		CROSS JOIN nodes AS n2
		INNER JOIN walk_nodes AS wn2 ON wn2.id = n2.id
		WHERE n1.id != n2.id AND 
			n1.inside = false AND 
			n2.inside = false AND 
--			(
--				n1.square1 = n2.square1 OR
--				n1.square2 = n2.square2
--			)AND
			ST_Distance(n1.loc,n2.loc) < 30

--  SELECT DISTINCT (make_candidates(minx,miny,maxx,maxy)).* FROM bboxes_100
   ) 
   SELECT sq.id1,sq.id2,ST_MakeLine(sq.loc1,sq.loc2) AS geom FROM sq
--   LIMIT 10;
 );

--SELECT * FROM make_candidates(444358,5532215,445458,5535Ļ315);
CREATE INDEX ON direct_candidates USING GIST(geom);
