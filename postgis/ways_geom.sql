UPDATE ways SET geom = ways_geom.geom
FROM (
	SELECT w.id AS id,ST_MakeLine(n.loc ORDER BY r.ord) AS geom
	FROM ways AS w 
	INNER JOIN ways_refs AS r ON w.id = r.id
	INNER JOIN nodes AS n ON r.ref=n.id
	GROUP BY w.id
) AS ways_geom
WHERE ways.id = ways_geom.id
;