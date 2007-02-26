<?
// rename to getRK
function getRK_val ($id) {
	return (getInput ("requesting data:" . $id));
}

// rename to printRK
function getRK ($id) {
	echo (getRK_val ($id));
}

function getPreview () {
	if (function_exists ("preview")) {
		preview ();
	}
}

function getInput ($prompt) {
	fputs (STDOUT, "#RKEND#\n" . $prompt . "#RKQEND#\n");
	fflush (STDOUT);
	while (1) {
		if (feof (STDIN)) die ("STDIN is at eof");			// if the parent process exits unexpectedly, make sure the PHP-process gets killed
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

ini_set ("display_errors", "1");
ini_set ("error_prepend_string", "#RKEND#\nPHP-Error");
ini_set ("error_append_string", "#RKQEND#\n");

while (1) {
	eval (getInput ("requesting code"));
	fflush (STDOUT);
}
?>