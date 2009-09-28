<?php
function preprocess () { ?>
require (foreign)
<?	}

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
}
?>
