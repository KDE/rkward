<?
	function preprocess () {
	}
	
	function calculate () {
		$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>rk.temp.objects <- list (<? echo ($vars); ?>)

# cor requires all object to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
rk.temp.frame <- data.frame (lapply (rk.temp.objects, eval))

rk.temp <- cor (rk.temp.frame, use="<? getRK ("use"); ?>", method="<? getRK ("method"); ?>")
<?
	}
	
	function printout () {
?>
rk.header ("Correlation Matrix", parameters=list ("Method", "<? getRK ("method"); ?>", "Exclusion", "<? getRK ("use"); ?>"))
rk.temp <- data.frame (I (sapply (rk.temp.objects, FUN=function (x) { rk.get.description (x, is.substitute=TRUE)})), as.data.frame (rk.temp))

rk.results (rk.temp, titles=c ('Variable Name', sapply (rk.temp.objects, rk.get.short.name)))
<?
	}
	
	function cleanup () {
?>rm (rk.temp)
<?
	}
?>
