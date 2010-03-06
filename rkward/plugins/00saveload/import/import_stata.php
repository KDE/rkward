<?php
function preprocess () { ?>
require (foreign)
<?
}

function calculate () {
	$options = "";

	if (getRK_val ("convert_dates")) {
	       $options .= ", convert.dates=TRUE" ;
	} else {
	       $options .= ", convert.dates=FALSE" ;
	}

	if (getRK_val ("convert_factors")) {
	       $options .= ", convert.factors=TRUE" ;
	} else {
	       $options .= ", convert.factors=FALSE" ;
	}

	if (getRK_val ("missing_type")) {
	       $options .= ", missing.type=TRUE" ;
	} else {
	       $options .= ", missing.type=FALSE" ;
	}

	if (getRK_val ("convert_underscore")) {
	       $options .= ", convert.underscore=TRUE" ;
	} else {
	       $options .= ", convert.underscore=FALSE" ;
	}

	$object = getRK_val ("saveto");
?>
data <- read.dta ("<? getRK ("file"); ?>"<? echo ($options); ?>)

# set variable labels for use in RKWard
labels <- attr (data, "var.labels")
if (!is.null (labels)) {
        for (i in 1:length (labels)) {
                col <- make.names (attr (data, "names")[i] )
                if (!is.null (col)) {
                        rk.set.label (data[[col]], labels[i])
                }
        }
}

<? echo ($object); ?> <<- data		# assign to globalenv()
<?
	if (getRK_val ("doedit") ) { ?>
rk.edit (<? echo ($object); ?>)
<?	}
}

function printout () {
	makeHeaderCode ("Import Stata File", array ("File" => getRK_val ("file"), "Imported to" => getRK_val ("saveto")));
}
?>
