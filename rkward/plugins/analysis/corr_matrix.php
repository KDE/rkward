<?
function preprocess () {
}
	
function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>rk.temp.objects <- list (<? echo ($vars); ?>)

# cor requires all object to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
rk.temp.frame <- data.frame (lapply (rk.temp.objects, eval))

# calculate correlation matrix
rk.temp <- cor (rk.temp.frame, use="<? getRK ("use"); ?>", method="<? getRK ("method"); ?>")
<?	if (getRK_val ("do_p")) { ?>
# calculate matrix of probabilities
rk.temp.p <- matrix (nrow = length (rk.temp.objects), ncol = length (rk.temp.objects))
local (
	for (i in 1:length (rk.temp.objects)) {
		for (j in i:length (rk.temp.objects)) {
			if (i != j) {
				t <- cor.test (eval (rk.temp.objects[[i]]), eval (rk.temp.objects[[j]]), use="<? getRK ("use"); ?>", method="<? getRK ("method"); ?>")
				rk.temp.p[i, j] <<- t$p.value
				rk.temp.p[j, i] <<- t$parameter["df"]
			}
		}
	}
)
<?	}
}

function printout () {
?>
rk.header ("Correlation Matrix", parameters=list ("Method", "<? getRK ("method"); ?>", "Exclusion", "<? getRK ("use"); ?>"))

rk.temp <- data.frame (I (sapply (rk.temp.objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (rk.temp))
rk.results (rk.temp, titles=c ('Coefficient', sapply (rk.temp.objects, rk.get.short.name)))

<?	if (getRK_val ("do_p")) { ?>
rk.temp.p <- data.frame (I (sapply (rk.temp.objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (rk.temp.p))
rk.results (rk.temp.p, titles=c ('df \\ p', sapply (rk.temp.objects, rk.get.short.name)))
<?	}
}

function cleanup () {
?>rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
