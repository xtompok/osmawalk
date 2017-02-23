DROP TABLE IF EXISTS direct;
CREATE TABLE direct AS
	WITH degs AS (
		SELECT id1 AS id, COUNT(id2) AS cnt
		FROM direct_ok 
		GROUP BY id1
	)
	SELECT id1, id2, geom FROM
	direct_ok AS dir
	INNER JOIN degs AS d1 ON d1.id = dir.id1
	INNER JOIN degs AS d2 ON d2.id = dir.id2
	WHERE d1.cnt > d2.cnt AND random() < (5.0/d1.cnt)
;
