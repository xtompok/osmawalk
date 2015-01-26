DROP TABLE IF EXISTS coord_nodes, dedup_nodes;

CREATE TABLE coord_nodes AS 
	SELECT n1.lat AS lat, n1.lon AS lon, MIN(n1.id) AS id
	FROM nodes AS n1 
	INNER JOIN nodes AS n2 
	ON (n1.lat = n2.lat AND n1.lon = n2.lon AND n1.id < n2.id)
	GROUP BY n1.lat, n1.lon
;

CREATE INDEX ON coord_nodes (lat, lon);

CREATE TABLE dedup_nodes AS
	SELECT n.id AS old_id, c.id AS new_id
	FROM coord_nodes AS c
	INNER JOIN nodes AS n
	ON (n.lat = c.lat AND n.lon = c.lon AND n.id != c.id)
;

CREATE INDEX ON dedup_nodes (old_id);

UPDATE ways_refs SET ref = dn.new_id
FROM(
	SELECT * FROM dedup_nodes
) AS dn
WHERE ways_refs.ref = dn.old_id
;
