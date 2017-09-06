#!/usr/bin/php

<?php
	$context = stream_context_create(array(
	'http' => array(
	'method' => "GET", 
	'ignore_errors' => true,
	'header' => "x-api-key: f2be074061f00df705d9fb2a540a4f1f" . "\r\n"
	)
	));
//	echo "context: $context";
	$request_url = 'https://api.streetsoncloud.com/sv1/location-info/3068/period/2';
//	echo "request_url: $request_url";
	//Try using file_get_contents to get location information
	$xml_string = file_get_contents($request_url, false, $context);
//	echo "Try var_dump of http_response_header: ";  
//	var_dump($http_response_header);
//	echo "Try var_dump of xml_string: ";  
	var_dump($xml_string);
//	echo "Try var_dump of context: ";  
//	var_dump($context);
//	echo "Try var_dump of : request_url";  
//	var_dump($request_url);
//	phpinfo();
?>
