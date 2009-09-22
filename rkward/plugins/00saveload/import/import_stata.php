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

	if (getRK_val ("convert_dates")) {
	       $convert_dates_opt = ", convert.dates=TRUE" ;
	} else {
	       $convert_dates_opt = ", convert.dates=FALSE" ;
	}

	if (getRK_val ("convert_factors")) {
	       $convert_factors_opt = ", convert.factors=TRUE" ;
	} else {
	       $convert_factors_opt = ", convert.factors=FALSE" ;
	}

	if (getRK_val ("missing_type")) {
	       $missing_type_opt = ", missing.type=TRUE" ;
	} else {
	       $missing_type_opt = ", missing.type=FALSE" ;
	}

	if (getRK_val ("convert_underscore")) {
	       $convert_underscore_opt = ", convert.underscore=TRUE" ;
	} else {
	       $convert_underscore_opt = ", convert.underscore=FALSE" ;
	}

	if (getRK_val ("warn_missing_labels")) {
	       $warn_missing_labels_opt = ", warn.missing.labels=TRUE" ;
	} else {
	       $warn_missing_labels_opt = ", warn.missing.lables=FALSE" ;
	}


	$object = getRK_val ("saveto");
?>
data <- read.dta ("<? getRK ("file"); ?>"<? echo ($convert_dates_opt); echo ($convert_factors_opt); echo ($missing_type_opt); echo ($convert_underscore_opt); echo ($warn_missing_labels);  ?>)
<?	if (getRK_val ("do_locale_conversion")) {
		$from_locale = getRK_val ("encoding");
		if ($from_locale == "other") {
			$from_locale = getRK_val ("user_encoding");
		} ?>

# convert all strings to the current encoding
data <- iconv.recursive (data, from="<? echo ($from_locale); ?>")
<?	}
	if (getRK_val ("convert_var_labels")) { ?>

# set variable labels for use in RKWard
labels <- attr (data, "variable.labels");
if (!is.null (labels)) {
	for (i in 1:length (labels)) {
		col <- make.names (names (labels[i]))
		if (!is.null (col)) {
			rk.set.label (data[[col]], labels[i])
		}
	}
}
<?	} ?>

<? echo ($object); ?> <<- data		# assign to globalenv()
<?
	if (getRK_val ("doedit") ) { ?>
rk.edit (<? echo ($object); ?>)
<?	}
}

function printout () {
}
?>
