DROP TABLE IF EXISTS barriers;

CREATE TABLE barriers AS

	SELECT pd.id, pd.geom
	FROM (
		SELECT  id,
			(ST_DumpRings((ST_Dump(sq.multi)).geom)).path[1] AS ring_num,
			(ST_DumpRings((ST_Dump(sq.multi)).geom)).geom AS geom  
		FROM (
			SELECT mp.id AS id,ST_BuildArea(ST_Collect(w.geom)) AS multi
			FROM multipols AS mp
			INNER JOIN multipols_refs AS mpr ON mp.id = mpr.id
			INNER JOIN ways AS w ON w.id = mpr.ref
			WHERE mp.objtype = 30 AND
				ST_IsValid(w.geom)
			GROUP BY mp.id
		) AS sq
	) AS pd
	WHERE pd.ring_num = 0

	UNION

	SELECT id, ST_Polygon(geom,3065)
	FROM ways
	WHERE type=30 
		AND area=true 
		AND ST_NPoints(geom)>=4 
		AND ST_IsClosed(geom)

	UNION

	SELECT id, geom
	FROM ways
	WHERE type=30 
		AND NOT (
			area=true 
			AND ST_NPoints(geom)>=4 
			AND ST_IsClosed(geom)
			)
;

CREATE INDEX ON barriers USING GIST(geom);
