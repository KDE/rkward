<?
function preprocess () {
}
	
function calculate () {
	global $use;

	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
	$use = getRK_val ("use");
	if ($use == "pairwise") {
		$exclude_whole = false;
		$use = "\"pairwise.complete.obs\"";
	} else {
		$exclude_whole = true;
		$use = "\"complete.obs\"";
	}

?>rk.temp.objects <- list (<? echo ($vars); ?>)

# cor requires all object to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
rk.temp.frame <- data.frame (lapply (rk.temp.objects, function (x) eval (x, envir=globalenv ())))

# calculate correlation matrix
rk.temp <- cor (rk.temp.frame, use=<? echo ($use); ?>, method="<? getRK ("method"); ?>")
<?	if (getRK_val ("do_p")) { ?>
# calculate matrix of probabilities
rk.temp.p <- matrix (nrow = length (rk.temp.frame), ncol = length (rk.temp.frame))
local ({
<?		if ($exclude_whole) { ?>
	# as we need to do pairwise comparisons for technical reasons,
	# we need to exclude incomplete cases first to match the use="complete.obs" parameter to cor()
	rk.temp.frame <- rk.temp.frame[complete.cases (rk.temp.frame),]
<?		} ?>
	for (i in 1:length (rk.temp.frame)) {
		for (j in i:length (rk.temp.frame)) {
			if (i != j) {
				t <- cor.test (rk.temp.frame[[i]], rk.temp.frame[[j]], method="<? getRK ("method"); ?>")
				rk.temp.p[i, j] <<- t$p.value
				rk.temp.p[j, i] <<- sum (complete.cases (rk.temp.frame[[i]], rk.temp.frame[[j]]))
			}
		}
	}
})
<?	}
}

function printout () {
	global $use;
?>
rk.header ("Correlation Matrix", parameters=list ("Method", "<? getRK ("method"); ?>", "Exclusion", <? echo ($use); ?>))

rk.temp <- data.frame (I (sapply (rk.temp.objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (rk.temp))
rk.results (rk.temp, titles=c ('Coefficient', sapply (rk.temp.objects, rk.get.short.name)))

<?	if (getRK_val ("do_p")) { ?>
rk.temp.p <- data.frame (I (sapply (rk.temp.objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (rk.temp.p))
rk.results (rk.temp.p, titles=c ('n \\ p', sapply (rk.temp.objects, rk.get.short.name)))
<?	}
}

function cleanup () {
?>rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
