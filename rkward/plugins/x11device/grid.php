<?php
function preprocess () {
	if (getRK_val ("is_embed")) { ?>
dev.set (<? getRK ("devnum"); ?>)
<?	}
}

function calculate () {
}

function printout () {
	$nx = getRK_val ("nx");
	if ($nx == "other") $gridoptions = 'nx=' . getRK_val ("nx_cells");
	else $gridoptions = 'nx=' . $nx;

	$ny = getRK_val ("ny");
	if ($ny == "other") $gridoptions .= ', ny=' . getRK_val ("ny_cells");
	else $gridoptions .= ', ny=' . $ny;

	$gridoptions .= getRK_val ("col.code.printout");

	if (getRK_val("custlwd")) $gridoptions .= ', lwd=' . round(getRK_val ("lwd"),1);

	$lty = getRK_val("linetype");
	if ($lty != "") $gridoptions .= ", lty=\"{$lty}\"";

	if (!getRK_val("equilogs")) $gridoptions .= ', equilogs=FALSE';
?>
grid(<? echo ($gridoptions); ?>);
<?
}
?>