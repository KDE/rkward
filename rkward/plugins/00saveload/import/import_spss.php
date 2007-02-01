<?php
function preprocess () { ?>
require (foreign)
<?
}

function calculate () {
	if (getRK_val ("data_frame")) {
		$data_frame = true;
		$data_frame_opt = ", to.data.frame=TRUE";
	}

	if (getRK_val ("use_labels")) {
		$labels_opt = ", use.value.labels=TRUE";
		$labels_opt .= ", max.value.labels=" . getRK_val ("labels_limit");
		if (getRK_val ("trim_labels")) $labels_opt .= ", trim.factor.names=TRUE";
	}

	$object = getRK_val ("saveto");
?>
<? echo ($object); ?> <- read.spss ("<? getRK ("file"); ?>"<? echo ($data_frame_opt); echo ($labels_opt); ?>)

<?	if ($data_frame) { 
// actually, this should not only happen for a data.frame (the alternative is a list), but maybe according to an option (or always)
?>
# set variable labels for use in RKWard
rk.temp.labels <- attr (<? echo ($object); ?>, "variable.labels");
if (!is.null (rk.temp.labels)) {
	for (rk.temp.i in 1:length (rk.temp.labels)) {
		rk.temp.col <- make.names (names (rk.temp.labels[rk.temp.i]))
		if (!is.null (rk.temp.col)) {
			rk.set.label (<? echo ($object); ?>[[rk.temp.col]], rk.temp.labels[rk.temp.i])
		}
	}
}
<?	} ?>
<?
}
	
function printout () {
}
	
function cleanup () { ?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
