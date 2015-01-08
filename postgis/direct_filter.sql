DROP TABLE IF EXISTS direct;
CREATE TABLE direct AS
	WITH degs AS (
		SELECT id1 AS id, COUNT(id2) AS cnt
		FROM direct_ok 
		GROUP BY id1
	)
	SELECT id1, id2, geom FROM
	direct_ok AS dir
	INNER JOIN degs ON degs.id = dir.id1
	WHERE random() < (6/degs.cnt)
;