--UPDATE multipols SET geom=mp_geom.geom
--FROM (
	SELECT m.id AS id, ST_AsText(ST_LineMerge(ST_Multi(ST_Collect(w.geom)))) AS geom
	FROM multipols AS m
	INNER JOIN multipols_refs AS mr ON mr.id = m.id
	INNER JOIN ways AS w ON w.id = mr.ref
	GROUP BY m.id
--) AS mp_geom
--WHERE multipols.id = mp_geom.id
;