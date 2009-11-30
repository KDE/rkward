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
  $startval_mtx = getRK_val("startval_mtx");
  $naaction     = getRK_val("naaction");
  $irtparam     = getRK_val("irtparam");
  $optimeth     = getRK_val("optimeth");
  $verbose      = getRK_val("verbose");
  // these are 3pl specific
  $ghk_3pl      = getRK_val("ghk_3pl");
  $iterqn_3pl   = getRK_val("iterqn_3pl");
  $type         = getRK_val("type");
  $maxguess     = getRK_val("maxguess");
  $optimizer    = getRK_val("optimizer");
  $epshess      = getRK_val("epshess");
  // $parscale     = getRK_val("parscale"); not implemented yet...

  ///////////////////////////////////
  // check for selected advanced control options
  $control = array() ;
  if($optimizer != "optim")
    $control[] = "optimizer=\"nlminb\"" ;
  if($iterqn_3pl != "1000")
    $control[] = "iter.qN=".$iterqn_3pl ;
  if($ghk_3pl != "21")
    $control[] = "GHk=".$ghk_3pl ;
  if($optimizer == "optim" && $optimeth != "BFGS")
    $control[] = "method=\"".$optimeth."\"" ;
  if($verbose == "TRUE")
    $control[] = "verbose=TRUE" ;
  if($epshess != "1e-03")
    $control[] = "eps.hessian=".$epshess ;

?>estimates.3pl <- tpm(<? getRK("x");
                  // any additional options?
                  if($type == "rasch") echo(", type=\"rasch\"");
                  if($constraint) echo(", constraint=".$constraint);
                  if($maxguess != "1") echo(", max.guessing=".$maxguess);
                  if($irtparam != "TRUE") echo(", IRT.param=FALSE");
                  if($startval == "random") echo(", start.val=\"random\"");
                  if($startval == "matrix") echo(", start.val=".$startval_mtx);
                  if($naaction) echo(", na.action=".$naaction);
                  // finally check if any advanced control options must be inserted
                  if($control) echo(", control=list(".join(", ", $control).")");
 ?>)
<?}

function printout () {
  // check whether parameter estimations should be kept in the global enviroment
  $save         = getRK_val("chk_save");
  $save_name    = getRK_val("save_name");
?>
rk.header ("3PL parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.3pl$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print (coef(estimates.3pl))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.3pl$log.Lik, digits=1)))
<?
// check if results are to be saved:
if ($save && $save_name) {
?>
# keep results in current workspace
<? echo($save_name); ?> <<- estimates.3pl
<?}
}
?>