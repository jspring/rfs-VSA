#!/usr/bin/php

<?php
	$LDSID=$argv[1];
	$SPEED=intval($argv[2]);

	$context = stream_context_create(array(
	'http' => array(
	'method' => "POST", 
	'ignore_errors' => true,
	'header' => "x-api-key: f2be074061f00df705d9fb2a540a4f1f" . "\r\n"
	)
	));
//	echo "context: $context";
	$location = $LDSID;
	$myspeed = $SPEED/0.62;
	echo "Location ID ".$location." speed ".$myspeed." kph ".$SPEED." mph\n";
	$request_url = 'https://api.streetsoncloud.com/sv1/set-speed-limit/location/'.$location.'/speed-limit/'.$myspeed;
	$xml_string = file_get_contents($request_url, false, $context);
	var_dump($xml_string);
?>
