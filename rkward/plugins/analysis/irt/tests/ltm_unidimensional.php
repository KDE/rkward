<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $spin_samples    = getRK_val("spin_samples");

?>unidim.res <<- unidimTest(<? getRK("x");
                  // check if any options must be inserted
                  if($spin_samples != "100") echo(", B=$spin_samples") ;
 ?>)
<?}

function printout () {
?>
rk.header ("Unidimensionality check (<? getRK("x"); ?>)")
rk.print (unidim.res)
<?
}
?>