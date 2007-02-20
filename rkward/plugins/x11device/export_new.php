<?php
function preprocess () {
}

function calculate () {
	$type = getRK_val ("format");
  $file = getRK_val ("file");
  if (getRK_val ("autoextension")) {
    if ($type == "jpeg") $ext = ".jpg";
    elseif ($type == "postscript") $ext = ".ps";
    else $ext = "." . $type;
    $possible_ext = substr($file, -4);
    if (strcasecmp($ext, $possible_ext) != 0) $file .= $ext;
  }
	$options = "";
	if (!getRK_val ("autowidth")) $options .= ", width=" . getRK_val ("width");
	if (!getRK_val ("autoheight")) $options .= ", height=" . getRK_val ("height");
	if (!getRK_val ("autopointsize")) $options .= ", pointsize=" . getRK_val ("pointsize");
	if (($type == "jpeg") & (!getRK_val ("autoquality"))) $options .= ", quality=" . getRK_val ("quality");
	if (!getRK_val ("autobg")) $options .= ", bg=\"" . getRK_val ("bg") . "\"";
	if (!getRK_val ("autores")) $options .= ", res=" . getRK_val ("resolution");
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