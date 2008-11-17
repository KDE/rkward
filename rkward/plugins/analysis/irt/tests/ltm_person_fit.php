<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $rad_hypot       = getRK_val("rad_hypot");
  $rad_resppat     = getRK_val("rad_resppat");
  $mtx_resppat     = getRK_val("mtx_resppat");
  $rad_pvalue      = getRK_val("rad_pvalue");
  $spin_mc         = getRK_val("spin_mc");

  ///////////////////////////////////
  // check for selected options
  $options = array() ;
  if($rad_hypot == "greater" || $rad_hypot == "two.sided")
    $options[] = "alternative=\"$rad_hypot\"" ;
  if($rad_resppat == "resp_matrix" && $mtx_resppat)
    $options[] = "resp.patterns=$mtx_resppat" ;
  if($rad_pvalue == "montecarlo")
    $options[] = "simulate.p.value=TRUE" ;
  if($rad_pvalue == "montecarlo" && $spin_mc != "1000" )
    $options[] = "B=$spin_mc" ;


?>personfit.res <- person.fit(<? getRK("x");
                  // check if any advanced control options must be inserted
                  if($options) echo(", ".join(", ", $options));
 ?>)
<?}

function printout () {
?>
rk.header ("Person-fit statistics (<? getRK("x"); ?>)")
rk.print (personfit.res)
<?
}
?>