<?php
	$servername = "delta.local";
	$username = "root";
	$password = "password";
	$dbname = "ssfp";
	$conn = mysqli_connect($servername, $username, $password, $dbname);
	if (!$conn) {die("!Connection failed: " . mysqli_connect_error());}
	
	if($_GET["SUBMIT"] == "back") {
		echo "+CONTEXT list\n";
	}
	
	if(isset($_GET["message"])) {
		$message = $_GET["message"];
		mysqli_query($conn, "INSERT INTO `messages` (`message`, `time`) VALUES ('$message', current_timestamp());");
	}
    
	echo "!Below is the most recent message\n";
	$sql = "SELECT * FROM `messages` ORDER BY `time` DESC LIMIT 10;";
	$result = mysqli_query($conn, $sql);
	while($row = mysqli_fetch_array($result)) {
		extract($row);
		echo "!";
		echo "$message";
		echo "\n";
	}
?>

&back Go back
submit back Go Back