ANALYZE barriers;
ANALYZE direct_candidates;

DROP TABLE IF EXISTS stops_cand_col;
CREATE TABLE stops_cand_col AS
	SELECT dc.sid AS sid, dc.nid AS nid, dc.geom AS geom, b.geom AS cgeomm, dc.objtype AS objtype
	FROM stops_direct_cand AS dc
	INNER JOIN stops AS s ON sid = s.id
	CROSS JOIN barriers AS b
	WHERE (NOT s.underground) AND ST_IsValid(b.geom)  AND (dc.geom && b.geom) AND (ST_Intersects(ST_MakeValid(dc.geom),b.geom))
;

DROP TABLE IF EXISTS stops_direct_ok;
CREATE TABLE stops_direct_ok AS
	(SELECT * FROM stops_direct_cand
	EXCEPT
	SELECT sid, nid, geom, objtype 
	FROM stops_cand_col)
	
	UNION
	
	(SELECT s.id AS sid,n.id AS nid,ST_MakeLine(s.loc,n.loc) AS geom, 54 AS objtype 
	FROM stops AS s
	INNER JOIN nodes AS n
	ON s.id = n.id
	)
;
