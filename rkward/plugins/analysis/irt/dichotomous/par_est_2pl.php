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
  // these are 2pl specific
  $ghk_2pl      = getRK_val("ghk_2pl");
  $iterem       = getRK_val("iterem");
  $iterqn_2pl   = getRK_val("iterqn_2pl");
  $interact     = getRK_val("interact");

  ///////////////////////////////////
  // check for selected advanced control options
  $control = array() ;
  if($iterem != "40")
    $control[] = "iter.em=".$iterem ;
  if($iterqn_2pl != "150")
    $control[] = "iter.qN=".$iterqn_2pl ;
  if($ghk_2pl != "15")
    $control[] = "GHk=".$ghk_2pl ;
  if($optimeth != "BFGS")
    $control[] = "method=\"".$optimeth."\"" ;
  if($verbose == "TRUE")
    $control[] ="verbose=TRUE" ;

?>estimates.2pl <- ltm(<? getRK("x"); ?> ~ z1<?
                  // any additional options?
                  if($interact == "TRUE") echo(" * z2");
                  if($constraint) echo(", constraint=".$constraint);
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
rk.header ("2PL parameter estimation")
rk.print (estimates.2pl)
<?
// check if results are to be saved:
if ($save && $save_name) {
?>
# keep results in current workspace
<? echo($save_name); ?> <<- estimates.2pl
<?}
}
?>