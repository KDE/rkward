<?
function preprocess () {
$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>
data <- data.frame (<? echo ($vars); ?>)
datadescription <- paste (rk.get.description (<? echo ($vars); ?>), collapse=", ");
<?
}

function calculate () {
?>
result <- ftable (data);
<?
}

function printout () {
?>
rk.header ("Crosstabs (n to n)", parameters=list ("Variables", datadescription))

rk.print (result)
<?
}
?>
