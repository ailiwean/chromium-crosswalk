<?php
header("Access-Control-Allow-Origin: http://127.0.0.1:8000");
header("Access-Control-Allow-Credentials: true");

$name = 'green-background.css';
$fp = fopen($name, 'rb');
header("Content-Type: text/css");
header("Content-Length: " . filesize($name));

fpassthru($fp);
exit;
?>
