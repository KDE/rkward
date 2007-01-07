<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp.glm = glm (<? getRK ("model"); ?>, data=<? getRK ("model.table"); ?>)
rk.temp.labels = <? getRK ("model.labels"); ?> 
rk.temp.anova = anova (rk.temp.glm)
<?
	}
	
	function printout () {
?>
rk.header ("Simple Anova")
rk.print (rk.temp.anova)
<?
	}
	
	function cleanup () {
?>rm (rk.temp.glm)
rm (rk.temp.labels)
rm (rk.temp.anova)
<?
	}
?>
