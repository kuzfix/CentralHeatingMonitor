<?php
header('Content-type: text/plain; charset=utf8', true);

$localBinary = "FW_JubomerV2.ino";
//echo md5_file($localBinary.".bin")."\n";
//echo md5_file($localBinary.".bin.signed")."\r\n";
$FWversion = "2.0.8";

$dbgStr = "";
/*
foreach($_SERVER as $name => $value) 
{
 $dbgStr.= $name." = ".$value."\r\n";
}*/

function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}

function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}

//Check if all the necessary header values present, abort if something missing.
if (!check_header('HTTP_X_ESP8266_STA_MAC') ||
    !check_header('HTTP_X_ESP8266_AP_MAC') ||
    !check_header('HTTP_X_ESP8266_FREE_SPACE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
    !check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
    !check_header('HTTP_X_ESP8266_SDK_VERSION')
	)
{
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP8266 updater! (header)\n";
    exit();
}

//Check memory size, abort if wrong size.
if (!check_header('HTTP_X_ESP8266_CHIP_SIZE','4194304'))
{
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "wrong chip size\n";
    exit();
}

if (!check_header('HTTP_X_ESP8266_VERSION'))	//if no version info available, only check if the new binary is different
{
	if ($_SERVER["HTTP_X_ESP8266_SKETCH_MD5"] != md5_file($localBinary.".bin"))
	{
		sendFile($localBinary.".bin");
	}
}
else
{
	$ver=$_SERVER['HTTP_X_ESP8266_VERSION'];
	if ($ver != $FWversion)
	{
		sendFile($localBinary.".bin.signed");
	}
	else 
	{
		header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
	}
}

//DEBUG - save dbgStr to database
function test_input($data) {
  //filter_input(INPUT_GET, 'timestamp', FILTER_SANITIZE_SPECIAL_CHARS);
  //is the above command better?
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

if ($dbgStr != "")
{
	$sqlQuery = "INSERT INTO debug(text) Values ('".test_input($dbgStr)."');";
	$servername = "212.235.187.62";
	$username = "jubo";
	$password = "SE17jubo";
	$dbname = "jubomer";

	//create DB connection
	$conn = new mysqli($servername, $username, $password, $dbname);
	// Check connection
	if (!$conn->connect_error)
	{
		echo "con...";
		echo $sqlQuery;
		if ($conn->query($sqlQuery) === TRUE) echo "done";
		$conn->close();
	}
}
?> 
