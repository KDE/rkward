<?
function preprocess () {
  // we'll need the eRm package, so in case it's not loaded...
?>
  require(eRm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $design       = getRK_val("design");
  $design_mtx   = getRK_val("design_mtx");
  $etastart     = getRK_val("etastart");
  $etastart_vec = getRK_val("etastart_vec");
  $stderr       = getRK_val("stderr");
  $sumnull      = getRK_val("sumnull");

?>estimates.rsm <<- RSM(<? getRK("x");
                  // any additional options?
                  if($design == "matrix") echo(", W=".$design_mtx);
                  if($stderr != "se") echo(", se=FALSE");
                  if($sumnull != "sum0") echo(", sum0=FALSE");
                  if($etastart == "startval") echo(", etaStart=".$etastart_vec);
 ?>)
<?}

function printout () {
?>
rk.header ("RSM  parameter estimation")
rk.print (estimates.rsm)
<?
}
?>