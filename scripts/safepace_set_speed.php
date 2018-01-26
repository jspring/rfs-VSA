#!/usr/bin/php

<?php
    while(1) {
	$context = stream_context_create(array(
	'http' => array(
	'method' => "POST", 
	'ignore_errors' => true,
	'header' => "x-api-key: f2be074061f00df705d9fb2a540a4f1f" . "\r\n"
	)
	));
//	echo "context: $context";
	$speed = file_get_contents('/var/www/html/VSA/webdata/speed.txt');
	$speed_array = explode(" ", $speed);
	for($i=0; $i<14; $i+=2) {
		if( $speed_array[$i] != NULL) {
			$location = $speed_array[$i];
			$j = $i + 1;
			$myspeed = $speed_array[$j];
			$mymph = $myspeed * 0.62;
			echo "Location ID ".$location." speed ".$myspeed." kph ".$mymph." mph\n";
			$request_url = 'https://api.streetsoncloud.com/sv1/set-speed-limit/location/'.$location.'/speed-limit/'.$myspeed;
//			echo "request_url: $request_url";
			//Try using file_get_contents to get location information
			$xml_string = file_get_contents($request_url, false, $context);
//			var_dump($http_response_header);
//			echo "Try var_dump of xml_string: ";  
			var_dump($xml_string);
//			echo "Try var_dump of context: ";  
//			var_dump($context);
//			echo "Try var_dump of : request_url";  
//			var_dump($request_url);
//			phpinfo();
			}
	}
	echo "\n";
	sleep(30);
    }
?>
