DROP TABLE IF EXISTS direct_candidates;
CREATE TABLE direct_candidates AS
	SELECT n1.id AS n1id, n2.id AS n2id
	FROM nodes AS n1
	INNER JOIN walk_nodes AS wn1 ON wn1.id = n1.id
	CROSS JOIN nodes AS n2
	INNER JOIN walk_nodes AS wn2 ON wn2.id = n2.id
	WHERE n1.inside = false AND n2.inside = false AND ST_Distance(n1.loc,n2.loc) < 30
;