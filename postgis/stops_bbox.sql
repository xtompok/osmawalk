WITH sq AS (
        SELECT  MIN(lon) AS minlon, MAX(lon) AS maxlon,
                MIN(lat) AS minlat, MAX(lat) AS maxlat,
                CEIL((MAX(lon)-MIN(lon))/100) AS lonparts,
                CEIL((MAX(lat)-MIN(lat))/100) AS latparts
        FROM nodes
) UPDATE stops SET square1 = FLOOR((lat-sq.minlat)/100)*lonparts+FLOOR((lon-sq.minlon)/100) FROM sq WHERE square1 = -1;

WITH sq AS (
        SELECT  MIN(lon) AS minlon, MAX(lon) AS maxlon,
                MIN(lat) AS minlat, MAX(lat) AS maxlat,
                CEIL((MAX(lon)-MIN(lon))/100) AS lonparts,
                CEIL((MAX(lat)-MIN(lat))/100) AS latparts
        FROM nodes
) UPDATE stops SET square2 = FLOOR((lat-sq.minlat+50)/100)*lonparts+FLOOR((lon-sq.minlon+50)/100) FROM sq WHERE square2 = -1;

CREATE INDEX ON stops(square1);
CREATE INDEX ON stops(square2);

