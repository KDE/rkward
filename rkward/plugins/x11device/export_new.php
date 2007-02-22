<?php
function preprocess () {
}

function calculate () {
	$type = getRK_val ("format");
	$jpegpng = (($type == "jpeg") | ($type == "png"));

	$file = getRK_val ("file");
	if (getRK_val ("autoextension")) {
		if ($type == "jpeg") {
			if (!( ereg("\.jpeg$",$file)|ereg("\.jpg$",$file) )) $file .= ".jpg";
		} elseif ($type == "postscript") {
			if (!( ereg("\.ps$",$file)|ereg("\.eps$",$file) )) $file .= ".eps";
		} else {
			$ext = "." . $type;
			if (!ereg($ext."$",$file)) $file .= $ext;
		}
/*		$possible_ext = substr($file, -4);
		if (strcasecmp($ext, $possible_ext) != 0) $file .= $ext;*/
// 		if (!ereg($ext."$",$file)) $file .= $ext;
	}
	$options = "";
	if ($jpegpng) {
		$width = getRK_val ("width_px");
		$height = getRK_val ("height_px");
	} else {
		$width = getRK_val ("width_in");
		$height = getRK_val ("height_in");
	}

	if (!getRK_val ("autowidth")) $options .= ", width=" . $width;
	if (!getRK_val ("autoheight")) $options .= ", height=" . $height;
	if (!getRK_val ("autopointsize")) $options .= ", pointsize=" . getRK_val ("pointsize");
	if (($type == "jpeg") & (!getRK_val ("autoquality"))) $options .= ", quality=" . getRK_val ("quality");
/*	if (!getRK_val ("color_transparent")) $options .= getRK_val ("bg.code.printout");
	else $options .= ", bg=\"transparent\"";*/
	if ($jpegpng & !getRK_val ("autores")) $options .= ", res=" . getRK_val ("resolution");
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
dev.print (device=<? echo ($type); ?>, file="<? echo ($file); ?>"<? echo ($options); ?>)
<?
}

function printout () {
}

function cleanup () {
?><?
}
?>