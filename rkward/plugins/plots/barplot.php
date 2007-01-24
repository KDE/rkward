<?
	function preprocess () {
}
	
	function calculate () {
}

	function printout () {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ; 
	?>

rk.temp.x2 <- (<? echo ($vars); ?>)
if(is.matrix(rk.temp.x2)) rk.temp.x=rk.temp.x2
if(is.factor(rk.temp.x2)) rk.temp.x=summary(rk.temp.x2)

rk.header ("Barplot", parameters=list ("Rainbow colors", "<? getRK ("rainbow"); ?>", "Beside", "<? getRK ("beside"); ?>", "Legend", "<? getRK ("legend"); ?>"))

rk.graph.on ()
try ({
	if(<? getRK ("beside"); ?>){
		if(<? getRK ("rainbow"); ?>) {		
		rk.temp.barplot <- barplot((rk.temp.x), col=rainbow(rk.temp.x), beside=<? getRK ("beside"); ?>, 	legend.text=<? getRK ("legend"); ?>,  ylim = range(rk.temp.x) * c(0, 1.2))
		}
		else {
		rk.temp.barplot <- barplot((rk.temp.x), beside=<? getRK ("beside"); ?>, legend.text=<? getRK ("legend"); ?>,  ylim = range(rk.temp.x) * c(0, 1.2))
	}
	if(<? getRK ("labels"); ?>) text(rk.temp.barplot, rk.temp.x, labels=rk.temp.x, pos=<? getRK ("place"); ?>, offset=.5)
	}
	else {
	if(<? getRK ("rainbow"); ?>) barplot((rk.temp.x), col=rainbow(rk.temp.x), legend.text=<? getRK ("legend"); ?>)
	else barplot((rk.temp.x), legend.text=<? getRK ("legend"); ?>)
	}
})
rk.graph.off ()


<?
	}
	
	function cleanup () {
	?>
rm(rk.temp.barplot, rk.temp.x, rk.temp.x2, rk.temp.rainbow)
	<?
	}
?>
