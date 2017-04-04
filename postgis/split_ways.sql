DROP TABLE IF EXISTS ways_refs_new;
CREATE TABLE ways_refs_new AS
	SELECT id,linesplit as ref,row_number() OVER (PARTITION BY id) AS ord 
	FROM ways,linesplit(30,ways.id)
	WHERE type != 30 AND type != 0; 

DELETE FROM ways_refs WHERE id IN 
(
	SELECT id FROM ways_refs_new
);

INSERT INTO ways_refs(id,ref,ord)
(
	SELECT id,ref,ord FROM ways_refs_new
)
