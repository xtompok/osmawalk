DROP TABLE IF EXISTS stops;
CREATE TABLE stops AS
	WITH sq AS (
		SELECT MAX(id) AS max FROM nodes
	)
	SELECT 	row_number() OVER () + sq.max AS id,
		stop_id, 
		ST_X(ST_Transform(ST_SetSRID(ST_Makepoint(stop_lon,stop_lat),4326),3065))::INTEGER AS lon,
		ST_Y(ST_Transform(ST_SetSRID(ST_Makepoint(stop_lon,stop_lat),4326),3065))::INTEGER AS lat,
		ST_Transform(ST_SetSRID(ST_MakePoint(stop_lon, stop_lat),4326),3065) AS loc,
		-1 AS square1,
		-1 AS square2
	FROM gtfs_stops,sq
	WHERE stop_id NOT IN (
		SELECT DISTINCT ref 
		FROM nodes
		WHERE ref IS NOT NULL)
	
	UNION
	
	SELECT row_number() OVER () AS id,
		gs.stop_id,
       		n.lon,
		n.lat,
		n.loc,
	       	-1 AS square1,
	       	-1 AS square2
	FROM sq,gtfs_stops AS gs
	INNER JOIN nodes AS n ON gs.stop_id = n.ref
;

