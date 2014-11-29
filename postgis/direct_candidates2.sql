DROP FUNCTION make_candidates(integer,integer,integer,integer);
CREATE OR REPLACE FUNCTION make_candidates(minx int,miny int,maxx int,maxy int)
RETURNS TABLE (id1 BIGINT,id2 BIGINT,loc1 GEOMETRY,loc2 GEOMETRY) AS '
 SELECT n1.id AS id1, n2.id AS id2, n1.loc AS loc1, n2.loc AS loc2
	FROM nodes AS n1
	INNER JOIN walk_nodes AS wn1 ON wn1.id = n1.id
	CROSS JOIN nodes AS n2
	INNER JOIN walk_nodes AS wn2 ON wn2.id = n2.id
	WHERE n1.id != n2.id AND 
		n1.inside = false AND 
		n2.inside = false AND
		(n1.lon BETWEEN minx AND maxx) AND 
		(n2.lon BETWEEN minx AND maxx) AND
		(n1.lat BETWEEN miny AND maxy) AND
		(n2.lat BETWEEN miny AND maxy) AND
		ST_Distance(n1.loc,n2.loc) < 30;
' LANGUAGE 'sql';

DROP TABLE IF EXISTS direct_candidates;
CREATE TABLE direct_candidates AS (
  WITH sq AS (
   SELECT DISTINCT (make_candidates(minx,miny,maxx,maxy)).* FROM bboxes_100
   ) 
   SELECT sq.id1,sq.id2,ST_MakeLine(sq.loc1,sq.loc2) AS geom FROM sq
--   LIMIT 10;
 );

--SELECT * FROM make_candidates(444358,5532215,445458,5535Ļ315);