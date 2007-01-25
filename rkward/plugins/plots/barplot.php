<?
	function preprocess () {
}
	
	function calculate () {
}

	function printout () {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ; 
	?>

rk.temp.x <- (<? echo ($vars); ?>)
if(is.factor(rk.temp.x)) rk.temp.x=summary(rk.temp.x)

rk.header ("Barplot", parameters=list ("Rainbow colors", "<? getRK ("rainbow"); ?>", "Beside", "<? getRK ("beside"); ?>", "Legend", "<? getRK ("legend"); ?>"))

rk.graph.on ()
try ({
	<?
	if (getRK_val ("beside") == "TRUE") {
	?>rk.temp.barplot <-barplot((rk.temp.x)<? if (getRK_val ("rainbow")=="TRUE") { ?>,
col=rainbow(rk.temp.x) <? } ?>, beside=<? getRK ("beside"); ?>, legend.text=<?
getRK ("legend"); ?>,  ylim = range(rk.temp.x) * c(0, 1.2))
	 <?
	}
	if ((getRK_val ("beside") == "TRUE") && getRK_val ("labels")=="TRUE") { ?>
text(rk.temp.barplot, rk.temp.x, labels=rk.temp.x, pos=<? getRK ("place"); ?>,
offset=.5)
	<? } 
	if (getRK_val ("beside") == "FALSE") {
?> barplot((rk.temp.x)<? if (getRK_val ("rainbow")=="TRUE") { ?>,
col=rainbow(rk.temp.x) <? } ?>, legend.text=<? getRK ("legend"); ?>)
	<? } ?>
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
