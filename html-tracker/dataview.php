<html>
<head>
<meta http-equiv="refresh" content="5">
</head>
<body>
<?php 
$username="kreatifi_arduser";$password="adventure";$database="kreatifi_onoendi";
mysql_connect(localhost,$username,$password);
@mysql_select_db($database) or die( "Unable to select database");
$query="SELECT * FROM device_location";
$result=mysql_query($query);
$num=mysql_numrows($result); 
mysql_close(); 
?>
<table border="0" cellspacing="2" cellpadding="2">
<tr>
<td>
<font face="Arial, Helvetica, sans-serif">Device Id</font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif">Latitude</font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif">Longitude</font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif">Logged Time</font>
</td>
</tr>
<tr>
<td>
<font face="Arial, Helvetica, sans-serif">-------------</font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif">----------</font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif">-------------</font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif">-----------------</font>
</td>
</tr>
<?php
$i=0;while ($i < $num) {
$f1=mysql_result($result,$i,"device_id");$f2=mysql_result($result,$i,"latitude_pos");
$f3=mysql_result($result,$i,"longitude_pos");$f4=mysql_result($result,$i,"logged_time");
?>
<tr>
<td>
<font face="Arial, Helvetica, sans-serif"><?php echo $f1; ?></font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif"><?php echo $f2; ?></font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif"><?php echo $f3; ?></font>
</td>
<td>
<font face="Arial, Helvetica, sans-serif"><?php echo $f4; ?></font>
</td>
</tr>
<?php $i++;} ?>
</body>
</html>