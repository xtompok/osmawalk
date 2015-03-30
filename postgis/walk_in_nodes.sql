DROP TABLE IF EXISTS walk_nodes;
CREATE TABLE walk_nodes AS
	SELECT DISTINCT n.id
	FROM ways AS w
	INNER JOIN ways_refs AS wr ON wr.id=w.id
	INNER JOIN nodes AS n ON wr.ref= n.id
	WHERE w.type != 0 AND w.type != 30
;
CREATE INDEX ON walk_nodes(id);
ALTER TABLE walk_nodes ADD UNIQUE (id);
--RAISE NOTICE 'Creating indexes';

CREATE INDEX ON nodes USING GIST(loc);
CREATE INDEX ON barriers USING GIST(geom);

--RAISE NOTICE 'Marking inside nodes';

DROP TABLE IF EXISTS walk_in_nodes;
CREATE TABLE walk_in_nodes AS
	SELECT n.id AS nid ,b.id AS bid
	FROM nodes AS n
	INNER JOIN walk_nodes AS wn ON wn.id = n.id
	CROSS JOIN barriers AS b
	WHERE ST_GeometryType(b.geom) = 'ST_Polygon'
		AND ST_Within(n.loc,b.geom) 
		AND NOT ST_Within(n.loc,ST_ExteriorRing(b.geom))
;

CREATE INDEX ON walk_in_nodes(nid);


UPDATE nodes SET inside=true
FROM walk_in_nodes
WHERE nodes.id = walk_in_nodes.nid
;
