<?
	function preprocess () {
	?>
rk.temp.barplot2<-function(x, beside=<? getRK ("beside"); ?>, legend.text=<? getRK ("legend"); ?>, rainbow=<? getRK ("rainbow"); ?>, labels=<? getRK ("labels"); ?>, place=<? getRK ("place"); ?>){
if(beside){
		if(rainbow) {		
		rk.temp.barplot <- barplot((x), col=rainbow(x), beside=beside, legend.text=legend.text,  ylim = range(x) * c(0, 1.2))
		}
		else {
		rk.temp.barplot <- barplot((x), beside=beside, legend.text=legend.text,  ylim = range(x) * c(0, 1.2))
		}
		if(labels) text(rk.temp.barplot,x, labels=x, pos=place, offset=.5)
	}
	else {
		if(rainbow) barplot((x), col=rainbow(x), legend.text=legend.text)
		else barplot((x), legend.text=legend.text)
}
}
<?
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
rk.temp.barplot2(rk.temp.x)	
})
rk.graph.off ()


<?
	}
	
	function cleanup () {
	?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
	<?
	}
?>
