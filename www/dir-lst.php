<!DOCTYPE html>
<html>
<head>
	<title>directory listing</title>
</head>
<body>
	<span style="color:#AAFFAA;text-align:center;"><h1>WEBSERV</h1></span>
	<span style="color:#AAFFAA;text-align:center;"><h2>directory listing</h2></span>
<style>
    body
	{
        background-color: rgb(42,42, 42);
    }
</style>
</body>
</html>

<?php
$path = $_ENV["TARGET"];
$cwd = getcwd() . "/";
$path = $cwd . "../" . basename($_ENV["TARGET"]);
$dir = dir($path);

while (false !== ($entry = $dir->read())) {
	if ($entry !== '.' && $entry != '..' && $entry[0] != '.')
	{
		if (is_dir($path . $entry))
		{
			echo '<span style="color:#AAFFAA; left: 10%; top: 40px; position: relative;">';
			echo $entry . "/" . "<br/>";
			echo '</span>';
		}
		else
		{
			echo '<span style="color:#AAFFAA; left: 10%; top: 40px; position: relative;">';
			print($entry . "<br/>");
			echo '</span>';
		}
	}
}
?>