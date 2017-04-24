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
		    maxZoom: 20,
		    id: 'xtompok.04nai95j',
		    accessToken: 'pk.eyJ1IjoieHRvbXBvayIsImEiOiJjaW43OTNwaGYwMDEyeDJsemRobDl3eHNnIn0.pPKNYlQbkNJLWw9k9xSFXw'
	}).addTo(map);
	return map;
}

function findPath(from,to){
	$.ajax("/search?flon="+from.lng+"&flat="+from.lat+"&tlon="+to.lng+"&tlat="+to.lat,{success: function(data,stat,req){
		pathFound(JSON.parse(data));		
	}});
}
function pointFeature(feature,layer){
	switch (feature.properties.type){
		case 60:
			addStop(feature.properties)
			switch (feature.properties.subtype){
				case "departure":
					layer.bindPopup("D:"+feature.properties.name+"<br>"+
						formatStopTime(feature.properties.arrival)+
						" --> "+formatStopTime(feature.properties.departure));
					break;
				case "arrival":
					layer.bindPopup("A:"+feature.properties.name+"<br>"+
						formatStopTime(feature.properties.arrival)+
						" --> "+formatStopTime(feature.properties.departure));
					break;
			}
		break;
	}
	
}
function linestringFeature(feature,layer){
	switch (feature.properties.type){
		case 60:
			layer.bindPopup("Line "+feature.properties.name);
			addLine(feature.properties);
		break;
	}
	
}

function addStop(data){
	var div;
	div = document.createElement('div')
	div.classList.add("stop")
	var stopstr;
	switch(data.subtype){
		case "departure":
			stopstr = "Departure <br>"+data.name+"<br>"+
				formatStopTime(data.arrival)+
				" --> "+formatStopTime(data.departure)+
				"<br>"
			break;
		case "arrival":
			stopstr = "Arrival<br>"+data.name+"<br>"+
				formatStopTime(data.arrival)+
				"<br>"
			
			break;	
	}
	div.innerHTML = stopstr;
	leftcol.appendChild(div);
}
function addLine(data){
	var div;
	div = document.createElement('div')
	div.classList.add("line")
	div.innerHTML = data.name;
	leftcol.appendChild(div);
	
}

function clearConnection(){
	while (leftcol.firstChild){
		leftcol.removeChild(leftcol.firstChild);	
	}	
}
function pathFound(data){
	dist.innerHTML=formatDist(data.properties.dist.toFixed(0));
	time.innerHTML=formatTotalTime(data.properties.time.toFixed(0));
	clearConnection();



	pathLayer =new  L.GeoJSON(data,{
		style: function(feature){
			switch (feature.properties.type){
				case 7: return {color: "#ff0000"};
				case 60: return {color: "#0000ff"};
				default: return {color: "#00ff00"};
			}
		},
		/*style: arcStyle,*/
		onEachFeature: function(feature,layer){
			switch (feature.geometry.type){
				case "Point":
					pointFeature(feature,layer);
				break;
				case "LineString":
					linestringFeature(feature,layer);
				break;
			}
		}
	});
	gpxbut.disabled = false
	gpxbut.onclick = function(){downloadGPX(from.getLatLng(),to.getLatLng())}
	pathLayer.addTo(map);
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
function formatStopTime(time){
	time = Math.floor(time) % (3600*24);
	var h;
	var m;
	var s;
	h = Math.floor(time/3600);
	time = time % 3600;
	m = Math.floor(time/60);
	time = time % 60;
	s = time;

	m = Intl.NumberFormat("cs-CZ",{"minimumIntegerDigits": 2}).format(m)
	s = Intl.NumberFormat("cs-CZ",{"minimumIntegerDigits": 2}).format(s)

	return (h+"."+m+":"+s);
}

function formatTotalTime(time){
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

		


