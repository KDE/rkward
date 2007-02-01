<?php
function preprocess () { ?>
require (foreign)
<?	if (getRK_val ("do_locale_conversion")) { ?>

# helper function to convert all strings to the current encoding
rk.temp.convert <- function (x, from) {
	attribs <- attributes (x);
	if (is.character (x)) {
		x <- iconv (x, from=from, to="", sub="")
	} else if (is.list (x)) {
		x <- lapply (x, function (sub) rk.temp.convert (sub, from))
	}
	# convert factor levels and all other attributes
	attributes (x) <- lapply (attribs, function (sub) rk.temp.convert (sub, from))
	x
}
<?	}
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
<?	if (getRK_val ("do_locale_conversion")) {
		$from_locale = getRK_val ("encoding");
		if ($from_locale == "other") {
			$from_locale = getRK_val ("user_encoding");
		} ?>

# convert all strings to the current encoding
<? echo ($object); ?> <- rk.temp.convert (<? echo ($object); ?>, from="<? echo ($from_locale); ?>")
<?	}
	if (getRK_val ("convert_var_labels")) { ?>

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
