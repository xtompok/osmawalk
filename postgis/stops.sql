DROP TABLE IF EXISTS stops;
CREATE TABLE stops AS
	WITH sq AS (
		SELECT MAX(id) AS max FROM nodes
	), gtfscnt AS (
		SELECT COUNT(*) AS cnt	
		FROM gtfs_stops
		WHERE stop_id NOT IN (
			SELECT DISTINCT ref 
			FROM nodes
			WHERE ref IS NOT NULL)
	)
	SELECT 	row_number() OVER () + sq.max AS id,
		stop_id,
		raptor_id, 
		ST_X(ST_Transform(ST_SetSRID(ST_Makepoint(stop_lon,stop_lat),4326),3065)) AS lon,
		ST_Y(ST_Transform(ST_SetSRID(ST_Makepoint(stop_lon,stop_lat),4326),3065)) AS lat,
		ST_Transform(ST_SetSRID(ST_MakePoint(stop_lon, stop_lat),4326),3065) AS loc,
		-1 AS square1,
		-1 AS square2
	FROM gtfs_stops,sq
	WHERE stop_id NOT IN (
		SELECT DISTINCT ref 
		FROM nodes
		WHERE ref IS NOT NULL)
	
	UNION
	
	SELECT row_number() OVER () + sq.max + gtfscnt.cnt AS id,
		gs.stop_id,
		gs.raptor_id,
       		n.lon,
		n.lat,
		n.loc,
	       	-1 AS square1,
	       	-1 AS square2
	FROM sq,gtfscnt,gtfs_stops AS gs
	INNER JOIN nodes AS n ON gs.stop_id = n.ref
;

