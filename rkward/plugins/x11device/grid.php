<?php
function preprocess () {
}
function calculate () {
	global $gridoptions;
	global $is_embed;

	$is_embed = getRK_val ("is_embed");

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
	if ($is_embed=="false") {
?>
dev.set (<? getRK ("devnum"); ?>)
grid(<? echo ($gridoptions); ?>);
<?
	}
}
function printout () {
	global $gridoptions;
	global $is_embed;
	if ($is_embed=="true") echo ("grid(" . $gridoptions . ")");
}
?>