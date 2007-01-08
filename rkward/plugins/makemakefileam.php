#!/usr/bin/php
<?

# filthy script to generate a Makefile.am for the plugins, and pages directories, installing all plugins and help pages as found.
# usage: (in plugins dir)
# ./makemakefileam.php > Makefile.am
# usage: (in pages dir)
# ../plugins/makemakefileam.php pages > Makefile.am

if ($argc < 2) {
	$base_dir = "";
	readsubs ("", "plugins");
} else {
	$base_dir = $argv[1];
	readsubs ("", $argv[1]);
}

function readsubs ($dir, $prefix) {
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
	
	echo (strtr ($prefix, "/", "X") . "dir = $(kde_datadir)/rkward/" . $base_dir . $dir . "\n");
	
	while (false !== ($file = readdir($thisdir))) {
		if (!is_dir ($ndir . $file)) {
			if ((substr ($file, -4) == ".xml") || (substr ($file, -4) == ".php") || (substr ($file, -10) == ".pluginmap") || (substr ($file, -4) == ".rkh") || (substr ($file, -4) == ".png")) {
				if ($file != "makemakefileam.php") {
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
		echo ("dist_" . strtr ($prefix, "/", "X") . "_DATA =");
		
		foreach ($files as $item) {
			echo (" \\\n\t" . $ndir . $item);
		}
		echo ("\n\n");
	}
	
	foreach ($subdirs as $sub) {
		readsubs ($ndir . $sub, $prefix . "/" . strtr ($sub, "_.", "UD"));
	}
	
	closedir ($thisdir);
}
?>