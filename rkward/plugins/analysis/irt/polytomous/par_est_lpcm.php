<?
function preprocess () {
  // we'll need the eRm package, so in case it's not loaded...
?>
  require(eRm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $mpoints      = getRK_val("mpoints");
  $groups       = getRK_val("groups");
  $group_vec    = getRK_val("group_vec");
  $design       = getRK_val("design");
  $design_mtx   = getRK_val("design_mtx");
  $etastart     = getRK_val("etastart");
  $etastart_vec = getRK_val("etastart_vec");
  $stderr       = getRK_val("stderr");
  $sumnull      = getRK_val("sumnull");

?>estimates.lpcm <- LPCM(<? getRK("x");
                  // any additional options?
                  if($design == "matrix") echo(", W=".$design_mtx);
                  if($mpoints > 1) echo(", mpoints=".$mpoints);
                  if($groups == "contrasts") echo(", groupvec=".$group_vec);
                  if($stderr != "se") echo(", se=FALSE");
                  if($sumnull != "sum0") echo(", sum0=FALSE");
                  if($etastart == "startval") echo(", etaStart=".$etastart_vec);
 ?>)
<?}

function printout () {
  // check whether parameter estimations should be kept in the global enviroment
  $save         = getRK_val("chk_save");
  $save_name    = getRK_val("save_name");
?>
rk.header ("LPCM  parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.lpcm$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print(t(rbind(Eta=estimates.lpcm$etapar,StdErr=estimates.lpcm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.lpcm$loglik, digits=1),
"<br />Number of iterations:",estimates.lpcm$iter,"<br />Number of parameters:",estimates.lpcm$npar))
<?
// check if results are to be saved:
if ($save && $save_name) {
?>
# keep results in current workspace
<? echo($save_name); ?> <<- estimates.lpcm
<?}
}
?>