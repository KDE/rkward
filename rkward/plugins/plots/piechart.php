<?
function preprocess () {
}

function calculate () {
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function doPrintout ($final) {
	$vars = getRK_val ("x");
	$tabulate= getRK_val ("tabulate")=="TRUE";

if($tabulate) {?>
x <- table (<? echo ($vars); ?>, exclude=NULL)
<?      } else { ?>
x <- <? echo ($vars); ?>

if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}
<?      } ?>

<?	if ($final) { ?>
rk.header ("Pie chart", list ("Variable", rk.get.description (<? echo ($vars); ?>)))

rk.graph.on ()
<?	}
?>
try ({
	pie(x, col=rainbow( if(is.matrix(x)) dim(x) else length(x)))
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
