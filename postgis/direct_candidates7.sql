--DROP INDEX IF EXISTS nodes_index_lon;
--DROP INDEX IF EXISTS nodes_index_lat;
--CREATE INDEX nodes_index_lon ON nodes(lon);
--CREATE INDEX nodes_index_lat ON nodes(lat);

--DROP TABLE IF EXISTS direct_walk_in_nodes;
--CREATE TABLE direct_walk_in_nodes AS (
--	SELECT id,loc,square1,square2 FROM nodes
--	WHERE walk = true AND inside = false
--);

--CREATE INDEX ON direct_walk_in_nodes(id); 

DROP TYPE ow_cand CASCADE;
CREATE TYPE ow_cand AS (id1 BIGINT, id2 BIGINT, loc1 GEOMETRY, loc2 GEOMETRY);

CREATE OR REPLACE FUNCTION make_candidates(integer) 
RETURNS SETOF ow_cand AS $$
	DECLARE  
		squares INT;
		node1 BIGINT;
		node2 BIGINT;
		r ow_cand;
	BEGIN
		squares = $1;
		FOR s IN 0..squares LOOP
			FOR r IN SELECT n1.id AS id1, n2.id AS id2, n1.loc AS loc1, n2.loc AS loc2
				FROM direct_walk_in_nodes AS n1
				INNER JOIN direct_walk_in_nodes AS n2 ON n1.square1 = s AND n2.square1 = s
				WHERE n1.id != n2.id
			LOOP
				RETURN NEXT r;
			END LOOP;
			FOR r IN SELECT n1.id AS id1, n2.id AS id2, n1.loc AS loc1, n2.loc AS loc2
				FROM direct_walk_in_nodes AS n1
				INNER JOIN direct_walk_in_nodes AS n2 ON n1.square2 = s AND n2.square2 = s
				WHERE n1.id != n2.id
			LOOP
				RETURN NEXT r;
			END LOOP;
			IF s%100 = 0 THEN
				RAISE NOTICE 'Processed: % of %, %',s,squares,clock_timestamp();
			END IF;
		END LOOP;
		RETURN;
	END;	
$$ LANGUAGE 'plpgsql';

DROP TABLE IF EXISTS direct_precandidates;
CREATE TABLE direct_precandidates2 AS (
	SELECT * FROM make_candidates(8063)
);
