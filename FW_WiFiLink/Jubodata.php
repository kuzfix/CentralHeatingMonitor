<?php
$J_update = false;		//set a jubomer to update mode (a binary file must be available at http://212.235.187.34/jubomer.bin)
$J_config = false;		//set a jubomer into a configuration mode - stops measuring, connects to wifi and serves a config web page
$J_ID = "";	//blank (""): set all yubomers into update mode, a number ("15" - no leading zeros): set jubomer with ID into update mode 
$result = "Error";
//parse received data
$countParams = 0;

if ($J_update) exit("update".$J_ID);
if ($J_config) exit("config".$J_ID);

function test_input($data) {
  //filter_input(INPUT_GET, 'timestamp', FILTER_SANITIZE_SPECIAL_CHARS);
  //is the above command better?
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

//echo '<pre>' . var_export($_REQUEST, true) . '</pre>';
$comment="";
foreach($_REQUEST as $name => $value) 
{
	//necessary parameters
	if ($name == 'private_key')		{$private_key = $value; $countParams++;	}
	if ($name == 'jubomerid')		{$jubomerid = $value; $countParams++;	}
	if ($name == 'napetost')		{$Vtotal = $value['V']; $countParams++;	}
	if ($name == 'niz1')			{$Vstring1 = $value['V']; $countParams++;	}
	if ($name == 'niz2')			{$Vstring2 = $value['V']; $countParams++;	}
	if ($name == 'niz3')			{$Vstring3 = $value['V']; $countParams++;	}
	if ($name == 'tempadc')			{$Tadc = $value['C']; $countParams++;	}
	if ($name == 'temp1w')			{$Tds18b20 = $value['C']; $countParams++;	}
	if ($name == 'tempi2c')			{$Tsht21 = $value['C']; $countParams++;	}
	if ($name == 'humidityi2c')		{$RHsht21 = $value['%RH'];	$countParams++;	}
	if ($name == 'timestamp')		{$timestamp = $value['s']; $countParams++;	}
	if ($name == 'MeasurmentNr')	{$measurmentNr = $value; $countParams++;	}
	if ($name == 'RSSi')			{$RSSi = $value['dBm']; $countParams++;	}
	if ($name == 'synced')			{$synced = $value['s']; $countParams++;	}
	if ($name == 'Name')			{$unitName = $value; $countParams++;	}
	//optional parameters
	if ($name == 'Calibration')		{$comment .= $value;}	
	if ($name == 'comment')			{$comment .= $value;}
}

//All parameters accounted for. Submit to database.
if ($countParams == 15)
{
	if ($private_key == "JB00B000")
	{
		date_default_timezone_set('UTC');
		$datetime = date("Y-m-d H:i:s", $timestamp+3600); //UTC+1

		$sqlQuery = "INSERT INTO results 
			(jubomerid,DateTime, napetost,niz1,niz2,niz3,tempadc,temp1w,tempi2c,
			humidityi2c,MeasurementNr,RSSi,synced,Name,Calibration) 
			Values ('"
			.test_input($jubomerid)."', '"
			.test_input($datetime)."', '"
			.test_input($Vtotal)."', '"
			.test_input($Vstring1)."', '"
			.test_input($Vstring2)."', '"
			.test_input($Vstring3)."', '"
			.test_input($Tadc)."', '"
			.test_input($Tds18b20)."', '"
			.test_input($Tsht21)."', '"
			.test_input($RHsht21)."', '"
			.test_input($measurmentNr)."', '"
			.test_input($RSSi)."', '"
			.test_input($synced)."', '"
			.test_input($unitName)."', '"
			.test_input($comment)."');";

		$servername = "212.235.187.62";
		$username = "jubo";
		$password = "SE17jubo";
		$dbname = "jubomer";

		//create DB connection
		$conn = new mysqli($servername, $username, $password, $dbname);
		// Check connection
		if ($conn->connect_error) {
			//die("Connection failed: " . $conn->connect_error);
		} 
		else if ($conn->query($sqlQuery) === TRUE) {
			$result = "OK";
		}
		$conn->close();
	}
}
echo $result
?> 
