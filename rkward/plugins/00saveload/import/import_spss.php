<?php
function preprocess () { ?>
require (foreign)
<?	if (getRK_val ("do_locale_conversion")) { ?>

# helper function to convert all strings to the current encoding
iconv.recursive <- function (x, from) {
	attribs <- attributes (x);
	if (is.character (x)) {
		x <- iconv (x, from=from, to="", sub="")
	} else if (is.list (x)) {
		x <- lapply (x, function (sub) iconv.recursive (sub, from))
	}
	# convert factor levels and all other attributes
	attributes (x) <- lapply (attribs, function (sub) iconv.recursive (sub, from))
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
<? echo ($object); ?> <- iconv.recursive (<? echo ($object); ?>, from="<? echo ($from_locale); ?>")
<?	}
	if (getRK_val ("convert_var_labels")) { ?>

# set variable labels for use in RKWard
labels <- attr (<? echo ($object); ?>, "variable.labels");
if (!is.null (labels)) {
	for (i in 1:length (labels)) {
		col <- make.names (names (labels[i]))
		if (!is.null (col)) {
			rk.set.label (<? echo ($object); ?>[[col]], labels[i])
		}
	}
}
<?	}
	if (getRK_val ("doedit") && $data_frame) { ?>

<? echo ($object); ?> <<- <? echo ($object); ?>		# assign to globalenv()
rk.edit (<? echo ($object); ?>)
<?	}
}

function printout () {
}
?>
