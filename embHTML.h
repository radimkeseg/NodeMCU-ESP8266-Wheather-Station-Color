const char PAGE_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<title>Meteo Station - Color</title>
<body>
<h1>Meteo Station - Color</h1>
<p>
Meteo Station with an NodeMCU ESP8266 IoT device and a DIGOLE display reading WunderGround Wheather API
<p/>
<form method='post' action='loc'>
<label>Country<br><input id='_city' name='_country' length=64 pattern='^[a-zA-Z][a-zA-Z0-9-_\\.]{1,64}$' required value='{country}'></label><br>
<label>City<br><input id='_country' name='_city' length=64 pattern='^[a-zA-Z][a-zA-Z0-9-_\\.]{1,64}$' required value='{city}'></label><br>
<label>UTC TimeOffset<br><input id='_timeoffset' name='_timeoffset' length=5 pattern='^[0-9-\\.]{1,5}$' required value='{timeoffset}'></label><br>
<input type='submit' value='Store and Restart'></form>

</body>
</html>
)=====";
