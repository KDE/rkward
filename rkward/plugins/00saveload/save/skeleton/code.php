<?
function preprocess () {
}

function calculate () {
$vars = str_replace ("\n", "','", trim (getRK_val ("data"))) ;
?>
package.skeleton(name="<? getRK("name"); ?>", list=c('<? echo ($vars); ?>'), path="<? getRK("path"); ?>", force= <? getRK("force"); ?>)
<?
}

function printout () {
}
?>
