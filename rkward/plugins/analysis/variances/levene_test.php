<?
function preprocess () { ?>
require(car)
<?}

function calculate () {

?>
result <- levene.test (<? getRK ("y"); ?>, <? getRK ("group"); ?>)
<?
}

function printout () {
?>
names <- rk.get.description (<? getRK ("y"); ?>, <? getRK ("group"); ?>)

rk.header ("Levene's Test", list ("response variable", names[1], "groups", names[2]))

rk.print (result)
<?
}
?>
