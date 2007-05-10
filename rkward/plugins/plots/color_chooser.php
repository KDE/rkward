<?
/* NOTE: This file is currently not used by the color_chooser plugin. It remains here, as it illustrates the functionality better than the hack that is actually in effect.

*/
function preprocess () {
}

function calculate () {
}

function printout () {
	$col = getRK_val ("color");
	if (empty ($col)) $col = getRK_val ("default_color");
	if (!empty ($col)) $col = getRK_val ("argument") . "\"" . $col . "\"";

	echo ($col);
}
?>