<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $constraint   = getRK_val("constraint");
  $startval     = getRK_val("startval");
  $startval_lst = getRK_val("startval_lst");
  $hessian      = getRK_val("hessian");
  $naaction     = getRK_val("naaction");
  $irtparam     = getRK_val("irtparam");
  $optimeth     = getRK_val("optimeth");
  $verbose      = getRK_val("verbose");
  // these are grm specific
  $ghk_grm      = getRK_val("ghk_grm");
  $iterqn_grm   = getRK_val("iterqn_grm");
  $dig_abbrv    = getRK_val("dig_abbrv");

  ///////////////////////////////////
  // check for selected advanced control options
  $control = array() ;
  if($iterqn_grm != "150")
    $control[] = "iter.qN=".$iterqn_grm ;
  if($ghk_grm != "21")
    $control[] = "GHk=".$ghk_grm ;
  if($optimeth != "BFGS")
    $control[] = "method=\"".$optimeth."\"" ;
  if($verbose == "TRUE")
    $control[] = "verbose=TRUE" ;
  if($dig_abbrv != "6")
    $control[] = "digits.abbrv=".$dig_abbrv ;

?>estimates.grm <<- grm(<? getRK("x");
                  // any additional options?
                  if($constraint == "const_discr") echo(", constrained=TRUE");
                  if($irtparam != "TRUE") echo(", IRT.param=FALSE");
                  if($hessian == "hessian") echo(", Hessian=TRUE");
                  if($startval == "random") echo(", start.val=\"random\"");
                  if($startval == "list") echo(", start.val=".$startval_lst);
                  if($naaction) echo(", na.action=".$naaction);
                  // finally check if any advanced control options must be inserted
                  if($control) echo(", control=list(".join(", ", $control).")");
 ?>)
<?}

function printout () {
?>
rk.header ("GRM parameter estimation")
rk.print (estimates.grm)
<?
}
?>