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

rk.header ("Levene's Test",names)

rk.print (result)
<?
}
?>
