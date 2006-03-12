#!/usr/bin/php
<?

# filthy script to generate a Makefile.am for the plugins-directory, installing all plugins as found.
# usage: (in plugins dir)
# ./makemakefileam.php > Makefile.am

readsubs ("", "plugins");

function readsubs ($dir, $prefix) {
	if ($dir == "") {
		$thisdir = opendir (".");
		$ndir = "";
	} else {
		$thisdir = opendir ($dir);
		$ndir = $dir . "/";
	}
	$subdirs = array ();
	$files = array ();
	
	echo (strtr ($prefix, "/", "X") . "dir = $(kde_datadir)/rkward/" . $dir . "\n");
	
	while (false !== ($file = readdir($thisdir))) {
		if (!is_dir ($ndir . $file)) {
			if ((substr ($file, -4) == ".xml") || (substr ($file, -4) == ".php") || (substr ($file, -10) == ".pluginmap")) {
				if ($file != "makemakefileam.php") {
					array_push ($files, $file);
				}
			}
		}  else {
			if (($file != ".") && ($file != "..") && ($file != "CVS")) {
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