"use strict";
var greenIcon = new L.Icon({
  iconUrl: 'https://cdn.rawgit.com/pointhi/leaflet-color-markers/master/img/marker-icon-green.png',
  shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
  iconSize: [25, 41],
  iconAnchor: [12, 41],
  popupAnchor: [1, -34],
  shadowSize: [41, 41]
});

var redIcon = new L.Icon({
  iconUrl: 'https://cdn.rawgit.com/pointhi/leaflet-color-markers/master/img/marker-icon-red.png',
  shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/0.7.7/images/marker-shadow.png',
  iconSize: [25, 41],
  iconAnchor: [12, 41],
  popupAnchor: [1, -34],
  shadowSize: [41, 41]
});

// Create the base map with
function createBaseMap(){
	var map = L.map('map').setView([50.0770, 14.4322], 15);

	L.tileLayer('https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token={accessToken}', {
		    attribution: 'Map data &copy; <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, Imagery © <a href="http://mapbox.com">Mapbox</a>',
		    maxZoom: 18,
		    id: 'xtompok.04nai95j',
		    accessToken: 'pk.eyJ1IjoieHRvbXBvayIsImEiOiJjaW43OTNwaGYwMDEyeDJsemRobDl3eHNnIn0.pPKNYlQbkNJLWw9k9xSFXw'
	}).addTo(map);
	return map;
}

function findPath(from,to){
	var pathLayer =new  L.GeoJSON.AJAX("/search?flon="+from.lng+"&flat="+from.lat+"&tlon="+to.lng+"&tlat="+to.lat,{
		/*style: arcStyle,*/
		onEachFeature: function(feature,layer){
				dist.innerHTML=formatDist(feature.properties.dist.toFixed(0));
				time.innerHTML=formatTime(feature.properties.time.toFixed(0));
		}
	});
	return pathLayer;
}

function formatDist(dist){
	var km;
	var m;
	dist = Math.floor(dist);
	km = Math.floor(dist/1000);
	dist = dist % 1000;	
	m = dist;
	if (km != 0){
		return km+" km "+m+" m"	
	}
	return m+" m"
}
function formatTime(time){
	var h;
	var m;
	var s;
	time = Math.floor(time);
	h = Math.floor(time/3600);
	time = time % 3600;
	m = Math.floor(time/60);
	time = time % 60;
	s = time;
	if (h!=0){
		return h+" hod "+m+" min "+s+" s"
	}
	if (m!=0){
		return m+" min "+s+" s"
	
	}
	return s+" s"
		
}

function downloadGPX(from,to){
	window.open("/gpx?flon="+from.lng+"&flat="+from.lat+"&tlon="+to.lng+"&tlat="+to.lat)
		
}

		


