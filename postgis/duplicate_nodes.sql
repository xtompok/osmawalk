CREATE TABLE dedup_nodes AS 
SELECT n1.lat AS lat, n1.lon AS lon, MIN(n1.id) AS id
FROM nodes AS n1 
INNER JOIN nodes AS n2 
ON (n1.lat = n2.lat AND n1.lon = n2.lon AND n1.id < n2.id)
GROUP BY n1.lat, n1.lon;

CREATE INDEX ON dedup_nodes (lat, lon);