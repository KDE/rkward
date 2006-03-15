<?
// rename to getR
function callR_val ($call) {
	return (getInput ("requesting rcall:" . $call));
}

// rename to printR
function callR ($call) {
	echo (callR_val ($call));
}

// rename to getRK
function getRK_val ($id) {
	return (getInput ("requesting data:" . $id));
}

// rename to printRK
function getRK ($id) {
	echo (getRK_val ($id));
}

function getRVector ($call) {
	return (explode ("\t", getInput ("requesting rvector:" . $call)));
}

function getInput ($prompt) {
	fputs (STDOUT, "#RKEND#\n" . $prompt);
	fflush (STDOUT);
	while (1) {
		if (feof (STDIN)) die ();			// if the parent process exits unexpectedly, make sure the PHP-process gets killed
		$inp = fgets (STDIN, 4096);
		if ($inp != "") {
			$input .= $inp;
			$inp = "";
			if (substr ($input, -8, 8) == "#RKEND#\n") {
				return (substr ($input, 0, -8));
			}
		}
	}
}

ini_set ("error_prepend_string", "#RKEND#\nPHP-Error");

while (1) {
	eval (getInput ("requesting code"));
	fflush (STDOUT);
}
?>