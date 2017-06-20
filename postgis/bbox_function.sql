WITH sq AS (
	SELECT  MIN(lon) AS minlon, MAX(lon) AS maxlon,
		MIN(lat) AS minlat, MAX(lat) AS maxlat,
		CEIL((MAX(lon)-MIN(lon))/100) AS lonparts,
		CEIL((MAX(lat)-MIN(lat))/100) AS latparts
	FROM nodes
) UPDATE nodes SET square1 = FLOOR((lat-sq.minlat)/100)*lonparts+FLOOR((lon-sq.minlon)/100) FROM sq;

WITH sq AS (
	SELECT  MIN(lon) AS minlon, MAX(lon) AS maxlon,
		MIN(lat) AS minlat, MAX(lat) AS maxlat,
		(MAX(lon)-MIN(lon))/100 AS lonparts,
		(MAX(lat)-MIN(lat))/100 AS latparts
	FROM nodes
) UPDATE nodes SET square2 = FLOOR((lat-sq.minlat+50)/100)*lonparts+FLOOR((lon-sq.minlon+50)/100) FROM sq;

CREATE INDEX ON nodes(square1);
CREATE INDEX ON nodes(square2);