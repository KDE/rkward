<?
function preprocess () {
}

function calculate () {
?>
rk.temp.x <- substitute (<? getRK ("x"); ?>)
<?
	}
?>

<?


function printout () {
?>
rk.header ("Stem-and-Leaf Plot",
	parameters=list ("Variable", paste (rk.get.description (rk.temp.x, is.substitute=TRUE)), "Plot Length", "<? getRK ("scale"); ?>","Plot Width", "<? getRK ("width"); ?>", "Tolerance", "<? getRK ("atom"); ?>"))

rk.print.literal(capture.output(stem(<? getRK ("x"); ?>, scale = <? getRK ("scale"); ?>, width = <? getRK ("width"); ?>, atom = <? getRK ("atom"); ?>)))

<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
