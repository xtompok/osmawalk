DROP TABLE IF EXISTS walk_nodes;
CREATE TABLE walk_nodes AS
	SELECT DISTINCT n.id
	FROM ways AS w
	INNER JOIN ways_refs AS wr ON wr.id=w.id
	INNER JOIN nodes AS n ON wr.ref= n.id
	WHERE w.type != 0 AND w.type != 30
;
ALTER TABLE walk_nodes ADD UNIQUE (id);
ALTER TABLE walk_nodes ADD CONSTRAINT walk_nodesfk FOREIGN KEY (id) REFERENCES nodes(id) MATCH FULL;
CREATE INDEX ON walk_nodes(id);

UPDATE nodes SET walk=true
FROM walk_nodes
WHERE nodes.id = walk_nodes.id;


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
ALTER TABLE walk_in_nodes ADD UNIQUE (nid);
ALTER TABLE walk_in_nodes ADD CONSTRAINT walk_in_nodesfk FOREIGN KEY (nid) REFERENCES nodes(id) MATCH FULL;
CREATE INDEX ON walk_in_nodes(nid);

DROP TABLE IF EXISTS walk_tunnel_nodes;
CREATE TABLE walk_tunnel_nodes AS
	SELECT DISTINCT n.id FROM nodes AS n
	INNER JOIN ways_refs AS wr ON n.id = wr.ref
	INNER JOIN ways AS w ON w.id = wr.id
	WHERE w.tunnel = true;
ALTER TABLE walk_tunnel_nodes ADD UNIQUE (id);
ALTER TABLE walk_tunnel_nodes ADD CONSTRAINT walk_tunnel_nodesfk FOREIGN KEY (nid) REFERENCES nodes(id) MATCH FULL;
CREATE INDEX ON walk_tunnel_nodes(nid);
 



UPDATE nodes SET inside=true
FROM walk_in_nodes
WHERE nodes.id = walk_in_nodes.nid
;

UPDATE nodes SET inTunnel = true
FROM walk_tunnel_nodes
WHERE nodes.id = walk_tunnel_nodes.id;
