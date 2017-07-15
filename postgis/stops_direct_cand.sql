
DROP TABLE IF EXISTS stops_direct_cand;
CREATE TABLE stops_direct_cand AS ((
	SELECT s.id AS sid, n.id AS nid, ST_MakeLine(s.loc,n.loc) AS geom, 51 AS objtype 
	FROM direct_walk_in_nodes AS n
	INNER JOIN stops AS s ON
		s.square00 = n.square00 OR
		s.square01 = n.square01 OR
		s.square10 = n.square10 OR
		s.square11 = n.square11 
	WHERE ST_Distance(s.loc,n.loc) < 30 AND NOT s.underground)

	UNION

	(SELECT s.id AS sid, n.id AS nid, ST_MakeLine(s.loc,n.loc) AS geom, 52 AS objtype
	FROM direct_underground_nodes AS n
	CROSS JOIN stops AS s
	WHERE ST_Distance(s.loc,n.loc) < 70 AND s.underground AND NOT s.osm)
);
CREATE INDEX ON stops_direct_cand USING GIST(geom);
