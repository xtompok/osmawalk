
DROP TABLE IF EXISTS stops_direct_cand;
CREATE TABLE stops_direct_cand AS (
	SELECT s.stop_id AS sid, n.id AS nid, ST_MakeLine(s.loc,n.loc) AS geom
	FROM direct_walk_in_nodes AS n
	INNER JOIN stops AS s ON
		s.square1 = n.square1 OR
		s.square2 = n.square2
	WHERE ST_Distance(s.loc,n.loc) < 30
);
CREATE INDEX ON stops_direct_cand USING GIST(geom);
