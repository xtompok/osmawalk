CREATE INDEX ON direct_candidates USING GIST(geom);

ANALYZE barriers;
ANALYZE direct_candidates;

DROP TABLE IF EXISTS direct_ok;
CREATE TABLE direct_ok AS
	SELECT DISTINCT dc.*
	FROM direct_candidates AS dc
	CROSS JOIN barriers AS b
	WHERE (NOT (dc.geom && b.geom)) OR ( NOT ST_Intersects(dc.geom,b.geom))
;