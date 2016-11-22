ANALYZE barriers;
ANALYZE direct_candidates;

DROP TABLE IF EXISTS direct_cand_col;
CREATE TABLE direct_cand_col AS
	SELECT dc.id1 AS id1, dc.id2 AS id2, dc.geom AS geom, b.geom AS cgeom
	FROM direct_candidates AS dc
	CROSS JOIN barriers AS b
	WHERE ST_IsValid(b.geom) AND (dc.geom && b.geom) AND (ST_Intersects(dc.geom,b.geom))
;

DROP TABLE IF EXISTS direct_ok;
CREATE TABLE direct_ok AS
	SELECT * FROM direct_candidates
	EXCEPT
	SELECT id1, id2, geom 
	FROM direct_cand_col
;
