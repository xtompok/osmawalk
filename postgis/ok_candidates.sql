DROP TABLE IF EXISTS direct_ok;
CREATE TABLE direct_ok AS
	SELECT dc.* FROM direct_candidates AS dc
	CROSS JOIN barriers AS bar
	WHERE NOT ST_Intersects(dc.geom,bar.geom)
;