<?
function preprocess () {
}

function calculate () {
?>
write (x=<? getRK("data"); ?>, file="<? getRK("file"); ?>", ncolumns=<? getRK("ncolumns"); ?>, append=<? getRK("append"); ?>)
<?
}

function printout () {
	makeHeaderCode ("Write Variables", array ("File" => getRK_val ("file"), "Data" => getRK_val ("data")));
}
?>
