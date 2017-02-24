ANALYZE barriers;
ANALYZE direct_candidates;

DROP TABLE IF EXISTS stops_cand_col;
CREATE TABLE stops_cand_col AS
	SELECT dc.sid AS sid, dc.nid AS nid, dc.geom AS geom, b.geom AS cgeom
	FROM stops_direct_cand AS dc
	CROSS JOIN barriers AS b
	WHERE ST_IsValid(b.geom)  AND (dc.geom && b.geom) AND (ST_Intersects(ST_MakeValid(dc.geom),b.geom))
;

DROP TABLE IF EXISTS stops_direct_ok;
CREATE TABLE stops_direct_ok AS
	SELECT * FROM stops_direct_cand
	EXCEPT
	SELECT sid, nid, geom 
	FROM stops_cand_col
;
