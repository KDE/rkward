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
	fflush (STDOUT);
	fputs (STDOUT, "#RKEND#\n" . $prompt);
	$stdin = fopen ('php://stdin', 'r');
	while (1) {
		$inp = fgets ($stdin, 4094);
		if ($inp != "") {
			$input .= $inp;
			$inp = "";
			if (substr ($input, -8, 8) != "#RKEND#\n") {
				usleep (100);
			} else {
				fclose ($stdin);
				return (substr ($input, 0, -8));
			}
		} else {
			usleep (1000);
		}
	}
}

fclose (STDIN);

ini_set ("error_prepend_string", "#RKEND#\nPHP-Error");

while (1) {
	include (getInput ("requesting code"));
}
?>