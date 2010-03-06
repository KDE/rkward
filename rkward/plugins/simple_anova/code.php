<?
function preprocess () {
}

function calculate () {
?>model = glm (<? getRK ("model"); ?>, data=<? getRK ("model.table"); ?>)
labels = <? getRK ("model.labels"); ?> 
result = anova (model)
<?
}

function printout () {
?>
rk.header ("Simple Anova")
rk.print (result)
<?
}
?>
