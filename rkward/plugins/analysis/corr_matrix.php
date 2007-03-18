<?
function preprocess () {
}
	
function calculate () {
	global $use;
	global $method;
	global $do_p;

	$do_p = getRK_val ("do_p");

	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
	$use = getRK_val ("use");
	if ($use == "pairwise") {
		$exclude_whole = false;
		$use = "\"pairwise.complete.obs\"";
	} else {
		$exclude_whole = true;
		$use = "\"complete.obs\"";
	}
	$method = "\"" . getRK_val ("method") . "\"";

?>objects <- list (<? echo ($vars); ?>)

# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data <- data.frame (lapply (objects, function (x) eval (x, envir=globalenv ())))

# calculate correlation matrix
result <- cor (data, use=<? echo ($use); ?>, method=<? echo ($method); ?>)
<?	if ($do_p) { ?>
# calculate matrix of probabilities
result.p <- matrix (nrow = length (data), ncol = length (data))
<?		if ($exclude_whole) { ?>
# as we need to do pairwise comparisons for technical reasons,
# we need to exclude incomplete cases first to match the use="complete.obs" parameter to cor()
data <- data[complete.cases (data),]
<?		} ?>
for (i in 1:length (data)) {
	for (j in i:length (data)) {
		if (i != j) {
			t <- cor.test (data[[i]], data[[j]], method=<? echo ($method); ?>)
			result.p[i, j] <- t$p.value
			result.p[j, i] <- sum (complete.cases (data[[i]], data[[j]]))
		}
	}
}
<?	}
}

function printout () {
	global $use;
	global $method;
	global $do_p;
?>
rk.header ("Correlation Matrix", parameters=list ("Method", <? echo ($method); ?>, "Exclusion", <? echo ($use); ?>))

result <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result))
rk.results (result, titles=c ('Coefficient', sapply (objects, rk.get.short.name)))

<?	if ($do_p) { ?>
result.p <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result.p))
rk.results (result.p, titles=c ('n \\ p', sapply (objects, rk.get.short.name)))
<?	}
}

?>
