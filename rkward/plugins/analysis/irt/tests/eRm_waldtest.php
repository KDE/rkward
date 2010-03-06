<?
function preprocess () {
  // we'll need the eRm package, so in case it's not loaded...
?>
  require(eRm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $rad_splitcr    = getRK_val("rad_splitcr");
  $splitvector     = getRK_val("splitvector");

?>waldtest.res <- Waldtest(<? getRK("x");
                  // check if any advanced control options must be inserted
                  if($rad_splitcr == "mean") echo(", splitcr=\"mean\"") ;
                  if($rad_splitcr == "vector") echo(", splitcr=".$splitvector) ;
 ?>)
<?}

function printout () {
?>
rk.header ("Wald test (<? getRK("x"); ?>)")
rk.print (waldtest.res)
<?
}
?>