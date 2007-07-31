<?
function preprocess () {
}

function calculate () {
}

function printout () {
$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>
data <- data.frame (<? echo ($vars); ?>)

rk.print (ftable(data))
<?
}
?>

	
