<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $data         = getRK_val("x");
  $chk_select   = getRK_val("chk_select");
  $inp_items    = getRK_val("inp_items");
  // reformat $inp_items
  if($inp_items)
    $inp_items       = str_replace("\n", ", ", preg_replace("/(.+)\[\[(.+)\]\]/", "$2", $inp_items));

  $constraint   = getRK_val("constraint");
  $startval     = getRK_val("startval");
  $startval_lst = getRK_val("startval_lst");
  $naaction     = getRK_val("naaction");
  $irtparam     = getRK_val("irtparam");
  $optimeth     = getRK_val("optimeth");
  $verbose      = getRK_val("verbose");
  // these are gpcm specific
  $ghk_gpcm     = getRK_val("ghk_gpcm");
  $iterqn_gpcm  = getRK_val("iterqn_gpcm");
  $optimizer    = getRK_val("optimizer");
  $numrderiv    = getRK_val("numrderiv");
  $epshess      = getRK_val("epshess");
  // $parscale     = getRK_val("parscale"); not implemented yet...

  ///////////////////////////////////
  // check for selected advanced control options
  $control = array() ;
  if($iterqn_gpcm != "150")
    $control[] = "iter.qN=".$iterqn_gpcm ;
  if($ghk_gpcm != "21")
    $control[] = "GHk=".$ghk_gpcm ;
  if($optimizer != "optim")
    $control[] = "optimizer=\"nlminb\"" ;
  if($optimizer == "optim" && $optimeth != "BFGS")
    $control[] = "optimMethod=\"".$optimeth."\"" ;
  if($numrderiv != "fd")
    $control[] = "numrDeriv=\"cd\"" ;
  if($epshess != "1e-06")
    $control[] = "epsHes=".$epshess ;
  if($verbose == "TRUE")
    $control[] = "verbose=TRUE" ;

?>estimates.gpcm <- gpcm(<?
		  if($data && $chk_select && $inp_items)
		    echo("subset(".$data.", select=c(".$inp_items."))");
		  else
		    echo($data);
                  // any additional options?
                  if($constraint != "gpcm") echo(", constraint=\"".$constraint."\"");
                  if($irtparam != "TRUE") echo(", IRT.param=FALSE");
                  if($startval == "list" && $startval_lst) echo(", start.val=".$startval_lst);
		  if($startval == "random") echo(", start.val=\"random\"");
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
rk.header ("GPCM parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.gpcm$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print (coef(estimates.gpcm))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.gpcm$log.Lik, digits=1)))
<?
// check if results are to be saved:
if ($save && $save_name) {
?>
# keep results in current workspace
<? echo($save_name); ?> <<- estimates.gpcm
<?}
}
?>