<!DOCTYPE html> 
<head>
<title>SafePace 650 Set Speed test</title>
<meta http-equiv="refresh" content="10" />
</head>
<body>
<?php
	$context = stream_context_create(array(
	'http' => array(
	'method' => "POST", 
	'ignore_errors' => true,
	'header' => "x-api-key: f2be074061f00df705d9fb2a540a4f1f" . "\r\n"
	)
	));
	echo "context: $context";
?>
	<p></p>
<?php
	$speed = file_get_contents('/var/www/html/php/speed.txt');
	$request_url = 'https://api.streetsoncloud.com/sv1/set-speed-limit/location/3068/speed-limit/'.$speed;
?>
<p></p>
<?php
	echo "request_url: $request_url";
?>
<p></p>
<?php
	//Try using file_get_contents to get location information
	$xml_string = file_get_contents($request_url, false, $context);
	var_dump($http_response_header);
?>
<p></p>
<?php
	echo "Try var_dump of xml_string: ";  
	var_dump($xml_string);
?>
<p></p>
<?php
	echo "Try var_dump of context: ";  
	var_dump($context);
?>
<p></p>
<?php
	echo "Try var_dump of : request_url";  
	var_dump($request_url);
?>
<p></p>
<?php
	phpinfo();
?>
</body>
