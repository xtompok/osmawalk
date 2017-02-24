DROP TABLE IF EXISTS stops;
CREATE TABLE stops AS
	SELECT stop_id, 
		ST_X(ST_Transform(ST_SetSRID(ST_Makepoint(stop_lon,stop_lat),4326),3065))::INTEGER AS lon,
		ST_Y(ST_Transform(ST_SetSRID(ST_Makepoint(stop_lon,stop_lat),4326),3065))::INTEGER AS lat,
		ST_Transform(ST_SetSRID(ST_MakePoint(stop_lon, stop_lat),4326),3065) AS loc,
		-1 AS square1,
		-1 AS square2
	FROM gtfs_stops
	WHERE stop_id NOT IN (
		SELECT DISTINCT ref 
		FROM nodes
		WHERE ref IS NOT NULL)
	
	UNION
	
	SELECT gs.stop_id,
       		n.lon,
		n.lat,
		n.loc,
	       	-1 AS square1,
	       	-1 AS square2
	FROM gtfs_stops AS gs
	INNER JOIN nodes AS n ON gs.stop_id = n.ref
;
