SELECT MIN(lon),MIN(lat),MAX(lon),MAX(lat)
FROM nodes;

DROP TYPE ow_bbox CASCADE;
CREATE TYPE ow_bbox AS (minx INT, miny INT, maxx INT, maxy INT);

CREATE OR REPLACE FUNCTION make_boxes(integer) 
RETURNS SETOF ow_bbox AS '
  DECLARE  
	minx INT; 
	miny INT; 
	maxx INT; 
	maxy INT; 
	dx INT; 
	dy INT;
	line ow_bbox ;
  BEGIN
    SELECT INTO minx MIN(lon) FROM nodes;
    SELECT INTO miny MIN(lat) FROM nodes;
    SELECT INTO maxx MAX(lon) FROM nodes;
    SELECT INTO maxy MAX(lat) FROM nodes;
    dx := $1/2;
    dy := $1/2;
      
    FOR i IN 1..((maxx-minx)/dx) LOOP
      FOR j IN 1..((maxy-miny)/dx) LOOP
	line.minx = minx;
	line.maxx = minx+2*dx;
	line.miny = miny;
	line.maxy = miny+2*dy;
	minx := minx + dx;
	miny := miny + dy;
	RETURN NEXT line;
      END LOOP;
    END LOOP;
    RETURN;
  END;
' LANGUAGE 'plpgsql';

DROP TABLE IF EXISTS bboxes_100;
CREATE TABLE bboxes_100 AS
	SELECT * FROM make_boxes(100);
