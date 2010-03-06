<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $spin_groups     = getRK_val("spin_groups");
  $drop_sumgroups  = getRK_val("drop_sumgroups");
  $rad_pvalue      = getRK_val("rad_pvalue");
  $spin_mc         = getRK_val("spin_mc");

  ///////////////////////////////////
  // check for selected options
  $options = array() ;
  if($spin_groups != "10")
    $options[] = "G=$spin_groups" ;
  if($drop_sumgroups != "median")
    $options[] = "FUN=$drop_sumgroups" ;
  if($rad_pvalue == "montecarlo")
    $options[] = "simulate.p.value=TRUE" ;
  if($rad_pvalue == "montecarlo" && $spin_mc != "100" )
    $options[] = "B=$spin_mc" ;


?>itemfit.res <- item.fit(<? getRK("x");
                  // check if any advanced control options must be inserted
                  if($options) echo(", ".join(", ", $options));
 ?>)
<?}

function printout () {
?>
rk.header ("Item-fit statistics (<? getRK("x"); ?>)")
rk.print (itemfit.res)
<?
}
?>