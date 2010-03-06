<?
function preprocess () {
}

function calculate () {
	// most options should only be shown, if they differ from the default.
	$options = "";
	if (getRK_val ("echo")) {
		$options .= ", echo=TRUE";
		$prompt = getRK_val ("promptecho");
		if (!empty ($prompt)) {
			$options .= ", prompt.echo=\"" . $prompt . "\"";
		}
		$options .= ", max.deparse.length=" . getRK_val ("maxdeparselength");
		$options .= ", verbose=" . getRK_val ("verbose");
	} else {
		$options .= ", verbose=FALSE";
	}
	$options .= ", print.eval=" . getRK_val ("printeval");
?>
source (file="<? getRK("file"); ?>", local=<? getRK("local"); echo ($options); ?>, chdir=<? getRK("chdir"); ?>)
<?
}

function printout () {
	makeHeadercode ("Source R file", array ("File" => getRK_val ("file")));
}

?>
