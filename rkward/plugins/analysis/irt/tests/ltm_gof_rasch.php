<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $spin_samples    = getRK_val("spin_samples");

?>GoFRasch.res <- GoF.rasch(<? getRK("x");
                  // check if any advanced control options must be inserted
                  if($spin_samples != "49") echo(", B=".$spin_samples);
 ?>)
<?}

function printout () {
?>
rk.header ("Goodness of Fit for Rasch Models (<? getRK("x"); ?>)")
rk.print (GoFRasch.res)
<?
}
?>