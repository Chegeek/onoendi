<?php 
$username="kreatifi_arduser";$password="adventure";$database="kreatifi_onoendi";
mysql_connect(localhost,$username,$password);
@mysql_select_db($database) or die( "Unable to select database");
$query="SELECT * FROM device_location WHERE device_id='".$_POST['device_id']."'";
$result=mysql_query($query);
$row=mysql_fetch_row($result);
$geoloc = array(
    "device_id" => "$row[0]",
    "latitude" => "$row[1]",
    "longitude" => "$row[2]",
    "lastmove" => "$row[3]"
);
echo json_encode($geoloc);
mysql_close(); 
?>