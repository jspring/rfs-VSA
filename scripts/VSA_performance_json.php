

<?php

$csv= file_get_contents("VSA_performance_plot.txt") or die("Error: Cannot create object");
$array = array_map("str_getcsv", explode("\n", $csv));
    

//$array = array_map("str_getcsv",  $json);
$json1 = json_encode(array_filter($array), JSON_NUMERIC_CHECK);

//echo json_encode($data);
print_r($json1).PHP_EOL
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


?>
