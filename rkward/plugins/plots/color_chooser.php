<?
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

function cleanup () {
}
?>