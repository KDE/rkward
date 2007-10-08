#!/usr/bin/php
<?

# filthy script to generate a CMakeLists.txt for the plugins, and pages directories, installing all plugins and help pages as found.
# usage: (in plugins dir)
# ./makecmakelists.php > CMakeLists.txt
# usage: (in pages dir)
# ../plugins/makecmakelists.php pages > CMakeLists.txt

if ($argc < 2) {
	$base_dir = "";
	readsubs ("");
} else {
	$base_dir = $argv[1];
	readsubs ("");
}

function readsubs ($dir) {
	global $base_dir;

	if ($dir == "") {
		$thisdir = opendir (".");
		$ndir = "";
	} else {
		$thisdir = opendir ($dir);
		$ndir = $dir . "/";
	}
	$subdirs = array ();
	$files = array ();

	$destination = "share/apps/rkward/" . $base_dir . $dir;
	
	while (false !== ($file = readdir($thisdir))) {
		if (!is_dir ($ndir . $file)) {
			if ((substr ($file, -4) == ".xml") || (substr ($file, -4) == ".php") || (substr ($file, -10) == ".pluginmap") || (substr ($file, -4) == ".rkh") || (substr ($file, -4) == ".png") || (substr ($file, -4) == ".css")) {
				if ($file != "makecmakelists.php") {
					array_push ($files, $file);
				}
			}
		}  else {
			if (($file != ".") && ($file != "..") && ($file != ".svn")) {
				array_push ($subdirs, $file);
			} 
		}
	}

	if (count ($files)) {
		echo ("INSTALL( FILES");
		
		foreach ($files as $item) {
			echo("\n\t" . $ndir . $item);
		}
		echo ("\n\tDESTINATION " . $destination . " )\n\n");
	}
	
	foreach ($subdirs as $sub) {
		readsubs ($ndir . $sub);
	}
	
	closedir ($thisdir);
}
?>