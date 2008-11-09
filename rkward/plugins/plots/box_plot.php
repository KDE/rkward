<?
function preprocess () {
}

function calculate () {
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout ($final) {
	$xvarsstring = join (", ", split ("\n", getRK_val ("x")));
	$names_mode = getRK_val ("names_mode");
	$mean = getRK_val ("mean");
	$sd = getRK_val ("sd");
	$horizontal = getRK_val ("orientation");
	$plot_adds = getRK_val ("plotoptions.code.calculate"); //add grid and alike

?>
	data_list <- list (<?echo ($xvarsstring); ?>)		#convert single sample variables to list
<?
	if ($names_mode == "rexp") {
		echo ("names(data_list) <- " . getRK_val ("names_exp") . "\n");
	} else if ($names_mode == "custom") {
		echo ("names(data_list) <- c (\"" . str_replace (";", "\", \"", trim (getRK_val ("names_custom"))) . "\")\n");
	}

	if ($final) {
?>
rk.header ("Boxplot", list ("Variable(s)", rk.get.description (<? echo ($xvarsstring); ?>, paste.sep=", ")))
rk.graph.on()
<?	} ?>
try (boxplot (data_list, notch = <? getRK ("notch") ?>, outline = <? getRK("outline")?>, horizontal = <? getRK("orientation"); ?><? getRK ("plotoptions.code.printout"); ?>)) #actuall boxplot function
<?	if (($mean == "TRUE") && ($horizontal == "TRUE")) {?>
	try (points(1:length(data_list) ~ apply(data.frame(<? echo ($xvarsstring); ?>),2,mean,na.rm = TRUE),pch=19, cex = <? getRK ("cex_sd_mean"); ?><? getRK ("sd_mean_color.code.printout"); ?>)) #calculates the mean for all data and adds a point at the corresponding position
<?	} if (($mean == "TRUE") && ($horizontal == "FALSE")) {?>
	try (points(apply(data.frame(<? echo ($xvarsstring); ?>),2,mean,na.rm = TRUE),pch=19, cex = <? getRK ("cex_sd_mean"); ?><? getRK ("sd_mean_color.code.printout"); ?>)) #calculates the mean for all data and adds a point at the corresponding position
<?	}
?>
<?	if (($sd == "TRUE") && ($horizontal == "FALSE")) {?>
	sd_low <- (apply(data.frame(<? echo ($xvarsstring); ?>),2,mean,na.rm = TRUE)) - (apply(data.frame(<? echo ($xvarsstring); ?>),2,sd,na.rm = TRUE))
	sd_high <- (apply(data.frame(<? echo ($xvarsstring); ?>),2,mean,na.rm = TRUE)) + (apply(data.frame(<? echo ($xvarsstring); ?>),2,sd,na.rm = TRUE))
	points(sd_low,pch=3, cex = <? getRK ("cex_sd_mean"); ?><? getRK ("sd_mean_color.code.printout"); ?>)
	points(sd_high,pch=3, cex = <? getRK ("cex_sd_mean"); ?><? getRK ("sd_mean_color.code.printout"); ?>)
<?	} if (($sd == "TRUE") && ($horizontal == "TRUE")) {?>
	sd_low <- (apply(data.frame(<? echo ($xvarsstring); ?>),2,mean,na.rm = TRUE)) - (apply(data.frame(<? echo ($xvarsstring); ?>),2,sd,na.rm = TRUE))
	sd_high <- (apply(data.frame(<? echo ($xvarsstring); ?>),2,mean,na.rm = TRUE)) + (apply(data.frame(<? echo ($xvarsstring); ?>),2,sd,na.rm = TRUE))
	points(1:length(data_list) ~ sd_low,pch=3, cex = <? getRK ("cex_sd_mean"); ?><? getRK ("sd_mean_color.code.printout"); ?>)
	points(1:length(data_list) ~ sd_high,pch=3, cex = <? getRK ("cex_sd_mean"); ?><? getRK ("sd_mean_color.code.printout"); ?>)
<?	}
?>
<?	if (!empty ($plot_adds)) { ?>

<?		// print the grid() related code
		printIndented ("\t", $plot_adds);
	}
?>
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}
?>
