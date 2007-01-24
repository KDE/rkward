<?php
function preprocess () {
}

function calculate () {
	$type = getRK_val ("format");
	if ($type == "other") {
		$type = getRK_val ("specifiedformat");
	}

	$options = "";
	if (!getRK_val ("autowidth")) $options .= ", width=" . getRK_val ("width");
	if (!getRK_val ("autoheight")) $options .= ", height=" . getRK_val ("height");
	$options = ", res=" . getRK_val ("resolution");
	if (!getRK_val ("autopointsize")) $options = ", pointsize=" . getRK_val ("pointsize");
?>
dev.set (<? getRK ("devnum"); ?>)
dev2bitmap ("<? getRK ("file"); ?>", type="<? echo ($type); ?>"<? echo ($options); ?>)
<?
}
	
function printout () {
}
	
function cleanup () {
?><?
}
?>