<?php
function preprocess () {
}

function calculate () {
	$type = getRK_val ("format");
	$file = getRK_val ("file");

	if ($type == "gs" ) {
		$gstype = getRK_val ("gsformat");
		if ($gstype == "other") $gstype = getRK_val ("gs_specifiedformat");
	} else {
		$jpegpng = (($type == "jpeg") | ($type == "png"));

		// Does the filename end with .ps/.eps or .pdf or .png or .jpeg/.jpg?
		// If not, add the appropriate extension.
		if (getRK_val ("autoextension")) {
			if ($type == "jpeg") {
				if (!( ereg("\.jpeg$",$file)|ereg("\.jpg$",$file) )) $file .= ".jpg";
			} elseif ($type == "postscript") {
				if (!( ereg("\.ps$",$file)|ereg("\.eps$",$file) )) $file .= ".eps";
			} else {
				$ext = "." . $type;
				if (!ereg($ext."$",$file)) $file .= $ext;
			}
		}
	}
	$options = "";

	// set $resolution appropriately:
	if ($jpegpng || ($type == "gs")) {
		$autores = getRK_val ("autores");
		if ($autores) {
			if ($jpegpng) $resolution = 96;
			else $resolution = 72;
		}	else $resolution = getRK_val ("resolution");
	}

	$autoW = getRK_val ("autowidth");	$autoH = getRK_val ("autoheight");

	// jpeg()/png() need at least one of width/height. For jpeg()/png() the width/height parameter (in pixels)
	// is calculated using width/height (in inches) times the resolution. For postscript()/pdf() $resolution is set to 1.
	if ($jpegpng && $autoW && $autoH) $options .= ", width=par(\"din\")[1]*" . $resolution;
	elseif ($jpegpng) {
		if(!$autoW) $options .= ", width=" . round(getRK_val ("width")*$resolution);
		if(!$autoH) $options .= ", height=" . round(getRK_val ("height")*$resolution);
	}	else {
		if(!$autoW) $options .= ", width=" . getRK_val ("width");
		if(!$autoH) $options .= ", height=" . getRK_val ("height");
	}

	// pointsize, resolution and quality parameters:
	if (!getRK_val ("autopointsize")) $options .= ", pointsize=" . getRK_val ("pointsize");
	if (($jpegpng && !$autores)	|| ($type == "gs"))  $options .= ", res=" . $resolution;
	if (($type == "jpeg") && (!getRK_val ("autoquality"))) $options .= ", quality=" . getRK_val ("quality");

	// For ps/pdf: page, pagecentre, horizontal, family, encoding and title parameters:
	if (!$jpegpng) {
		$paper = getRK_val ("paper");
		if (!empty ($paper)) $options .= ", paper=" . "\"" . $paper . "\"";
		$pagecentre = getRK_val ("pagecentre");
		if (!$pagecentre) $options .= ", pagecentre=FALSE";
		$pshoriz = getRK_val ("ps_horiz");
		if (!$pshoriz) $options .= ", horizontal=FALSE";
		$family = getRK_val ("family");
		if (!empty($family)) $options .= ", family=" . "\"" . $family . "\"";
		$enc = getRK_val ("encoding");
		if (!empty($enc)) $options .= ", encoding=" . "\"" . $enc . "\"";
		if (!getRK_val("autotitle")) $options .= ", title=" . "\"" . getRK_val("title") . "\"";
	}
?>
dev.set (<? getRK ("devnum"); ?>)
<? if ($type == "gs") {?>
dev2bitmap ("<? echo($file); ?>", type="<? echo ($gstype); ?>"<? echo ($options); ?>);
<? } else {?>
dev.print (device=<? echo ($type); ?>, file="<? echo ($file); ?>"<? echo ($options); ?>);
<? }
}

function printout () {
}

function cleanup () {
?><?
}
?>