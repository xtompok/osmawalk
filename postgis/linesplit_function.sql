DROP TYPE ow_linesplit CASCADE;
CREATE TYPE ow_linesplit AS (partid INT, geom geometry, posfrom DOUBLE PRECISION, posto DOUBLE PRECISION );

CREATE OR REPLACE FUNCTION linesplit(integer,bigserial) 
RETURNS SETOF ow_linesplit AS '
  DECLARE  
	length DOUBLE PRECISION; 
	parts INT;
	position DOUBLE PRECISION;
	newline geometry;
	result ow_linesplit;
	way ways%ŘOWTYPE; 
	series setof INT;
  BEGIN
	SELECT * INTO way FROM ways WHERE id = $1;
	WITH line AS
	(SELECT
		ST_AddMeasure(way.geometry, 0, ST_Length(way.geometry)) AS linem,
		generate_series(0, ST_Length(way.geometry)::int, 10) AS i
	FROM way),
	geometries AS (
	SELECT
		i,
		(ST_Dump(ST_GeometryN(ST_LocateAlong(linem, i), 1))).geom AS geom 
	FROM linemeasure)
	SELECT
		i,
		ST_SetSRID(ST_MakePoint(ST_X(geom), ST_Y(geom)), 31468) AS geom
	FROM geometries



	SELECT s INTO series 
	length := ST_Length(way.geometry);
	parts := length / $1; 
	position := 0;
	  
	FOR i IN 1..parts LOOP
		result.geom = ST_LineSubstring($2,position,LEAST(1,position+1.0/parts));
	result.partid = i;
	result.posfrom = position;
	result.posto = position + 1.0/parts;
	position = position + 1.0/parts;
			
	RETURN NEXT result;
	END LOOP;
	RETURN;
  END;
' LANGUAGE 'plpgsql';

WITH line AS 
	(SELECT
		your_polylinestring_id,
		(ST_Dump(geom)).geom AS geom
	FROM your_polylinestring_table),
linemeasure AS
	(SELECT
		ST_AddMeasure(line.geom, 0, ST_Length(line.geom)) AS linem,
		generate_series(0, ST_Length(line.geom)::int, 10) AS i
	FROM line),
geometries AS (
	SELECT
		i,
		(ST_Dump(ST_GeometryN(ST_LocateAlong(linem, i), 1))).geom AS geom 
	FROM linemeasure)

SELECT
	i,
	ST_SetSRID(ST_MakePoint(ST_X(geom), ST_Y(geom)), 31468) AS geom
FROM geometries



SELECT ST_MakeLine(ARRAY[dp1.geom, dp2.geom]) AS seg
FROM gm,st_dumppoints(gm.geom) AS dp1,st_dumppoints(gm.geom) AS dp2
WHERE dp2.path[1] - dp1.path[1] = 1

