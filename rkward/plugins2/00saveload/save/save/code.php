<?
function preprocess () {
}

function calculate () {
	global $vars;
	$vars = str_replace ("\n", ",", trim (getRK_val ("data")));
?>
save (<? echo ($vars); ?>, file="<? getRK("file"); ?>", ascii=<? getRK("ascii"); ?>, compress=<? getRK("compress"); ?>)
<?
}

function printout () {
	global $vars;
	makeHeaderCode ("Save R objects", array ("File" => getRK_val ("file"), "Variables" => $vars));
}
?>
