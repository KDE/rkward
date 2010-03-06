<?
function preprocess () {
}

function calculate () {
	$sep = getRK_val ("sep");
	if ($sep == "other") $sep = getRK_val ("custom_sep");
	if ($sep == " ") $sep = "";
	else $sep = ", sep=" . quote ($sep);
?>
write (x=<? getRK("data"); ?>, file="<? getRK("file"); ?>", ncolumns=<? getRK("ncolumns"); ?>, append=<? getRK("append"); echo ($sep); ?>)
<?
}

function printout () {
	makeHeaderCode ("Write Variables", array ("File" => getRK_val ("file"), "Data" => getRK_val ("data")));
}
?>
