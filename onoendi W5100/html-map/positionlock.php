<?php
$username="kreatifi_arduser";
$password="adventure";
$database="kreatifi_onoendi";
$val_deviceid=$_POST['deviceid'];
$val_lat=$_POST['latitude'];
$val_lon=$_POST['longitude'];
$val_gpstime=$_POST['gpstime'];
$noww = date("Y-m-d H:i:s",time());
if ($val_deviceid!= ''){
mysql_connect(localhost,$username,$password);
@mysql_select_db($database) or die( "Unable to select database");
// get existing registered device!
$query= mysql_query("SELECT device_id FROM device_location WHERE device_id = '". $val_deviceid ."'");
$identity = mysql_num_rows($query);
if ($identity == 1){
	$query = "UPDATE `device_location` SET `latitude_pos`= '". $val_lat ."',`longitude_pos`= '". $val_lon ."', `gpstime`= '". $val_gpstime ."', `logged_time`= '". $noww ."' WHERE device_id = '". $val_deviceid ."'";
	if (mysql_query($query)){ echo "#OK"; } else {};
} else {
	echo ":Device isn't registered!";
	};
mysql_close();
}
?>