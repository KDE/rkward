<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp.glm = glm (<? getRK ("model"); ?>, data=<? getRK ("model.data"); ?>)
rk.temp.labels = <? getRK ("model.labels"); ?> 
rk.temp.anova = anova (rk.temp.glm)<?
	}
	
	function printout () {
?>cat ("<h1>TODO: format Output</h1>")
print (rk.temp.anova)
<?
	}
	
	function cleanup () {
?>rm (rk.temp.glm)
rm (rk.temp.labels)
rm (rk.temp.anova)
<?
	}
?>
