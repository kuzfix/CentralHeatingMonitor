<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

$J_update = true;		//set a jubomer to update mode (a binary file must be available at http://212.235.187.34/jubomer.bin)
$J_config = true;		//set a jubomer into a configuration mode - stops measuring, connects to wifi and serves a config web page
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
	if ($name == 'private_key')		{$v=$value; 		$private_key = $v; $countParams++;	}
	if ($name == 'jubomerid')		{$v=$value; 		$jubomerid = $v; $countParams++;	}
	if ($name == 'napetost')		{$v=$value['V']; 	$Vtotal = $v; $countParams++;	}
	if ($name == 'niz1')			{$v=$value['V']; 	$Vstring1 = $v; $countParams++;	}
	if ($name == 'niz2')			{$v=$value['V']; 	$Vstring2 = $v; $countParams++;	}
	if ($name == 'niz3')			{$v=$value['V']; 	$Vstring3 = $v; $countParams++;	}
	if ($name == 'tempadc')			{$v=$value['C']; 	$Tadc = $value['C']; $countParams++;	}
	if ($name == 'temp1w')			{$v=$value['C']; 	$Tds18b20 = $v; $countParams++;	}
	if ($name == 'tempi2c')			{$v=$value['C']; 	$Tsht21 = $v; $countParams++;	}
	if ($name == 'humidityi2c')		{$v=$value['%RH'];	$RHsht21 = $v;	$countParams++;	}
	if ($name == 'timestamp')		{$v=$value['s']; 	$timestamp = $v; $countParams++;	}
	if ($name == 'MeasurmentNr')	{$v=$value; 		$measurmentNr = $v; $countParams++;	}
	if ($name == 'RSSi')			{$v=$value['dBm']; 	$RSSi = $v; $countParams++;	}
	if ($name == 'synced')			{$v=$value['s']; 	$synced = $v; $countParams++;	}
	if ($name == 'Name')			{$v=$value; 		$unitName = $v; $countParams++;	}
	//optional parameters
	if ($name == 'Calibration')		{$v=$value; $comment .= $v;}	
	if ($name == 'comment')			{$v=$value; $comment .= $v;}
	//echo "<br>".$name." = ".$v;
}

//All parameters accounted for. Submit to database.
if ($countParams == 15)
{
	if ($private_key == "JB00B000")
	{
		date_default_timezone_set('UTC');
		$datetime = date("Y-m-d H:i:s", $timestamp+3600); //UTC+1

		//echo "<br>".$datetime."<br>";
		$sqlQuery = "INSERT INTO results_tmp 
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
	
		//echo "<br>".$sqlQuery."<br>";

		$servername = "212.235.187.62";
		$username = "jubo";
		$password = "SE17jubo";
		$dbname = "jubomer";

		//create DB connection
		$conn = new mysqli($servername, $username, $password, $dbname);
		// Check connection
		if ($conn->connect_error) {
			die("Connection failed: " . $conn->connect_error);
		} 
		else
		{
			if ($conn->query($sqlQuery) === TRUE) $result = "Jubomer data submission: OK";
/*
			$sqlQuery = "SELECT * FROM jubomer.results_tmp ORDER BY DateTime DESC LIMIT 3";
			$sqlResults = $conn->query($sqlQuery);
			echo "<table border=1>";
			$i=0;
			foreach ($sqlResults as $oneRecord)
			{
				if ($i == 0) 
				{
					echo "<tr>";
					foreach ($oneRecord as $sqlField => $sqlData)
					{
						echo "<th>".$sqlField."</th>";
					}
					echo "</tr>";
					$i++;
				}
				//var_dump($oneRecord);
				echo "<tr>";
				foreach ($oneRecord as $sqlField => $sqlData)
				{
					echo "<td>".$sqlData."</td>";
				}
				echo "</tr>";
			}
			echo "</table><br>";

			$sqlQuery = "SELECT * FROM jubomer.results_tmp WHERE jubomerid = '".$jubomerid."' ORDER BY DateTime DESC LIMIT 3";
			$sqlResults = $conn->query($sqlQuery);
			echo "<table border=1>";
			$i=0;
			foreach ($sqlResults as $oneRecord)
			{
				if ($i == 0) 
				{
					echo "<tr>";
					foreach ($oneRecord as $sqlField => $sqlData)
					{
						echo "<th>".$sqlField."</th>";
					}
					echo "</tr>";
					$i++;
				}
				//var_dump($oneRecord);
				echo "<tr>";
				foreach ($oneRecord as $sqlField => $sqlData)
				{
					echo "<td>".$sqlData."</td>";
				}
				echo "</tr>";
			}
			echo "</table>";*/
		}
		
		
		$conn->close();
	}
	else
	{
		echo "Wrong private key.";
	}
}
else
{
	echo "Some data still missing. Only ".string($countParams)." parameters received";
}
echo $result;
?> 
