<?
function preprocess () {
  // we'll need the eRm package, so in case it's not loaded...
?>
  require(eRm)
<?}

function calculate () {
}

function printout () {
        doPrintout (true);
}

function preview () {
        preprocess ();
        calculate ();
        doPrintout (false);
}

function doPrintout ($final) {
        // this function takes care of generating the code for the printout() section. If $final is set to true,
        // it generates the full code, including headers. If $final is set to false, only the essentials will
        // be generated.

  // let's read all values into php variables for the sake of readable code
  $rad_splitcr      = getRK_val("rad_splitcr");
  $splitvector      = getRK_val("splitvector");
  $inp_items        = getRK_val("inp_items");
  $chk_se           = getRK_val("chk_se");
  $chk_confint      = getRK_val("chk_confint");
  $spin_confint     = getRK_val("spin_confint");
  $chk_ctrline      = getRK_val("chk_ctrline");
  $spin_ctrline     = getRK_val("spin_ctrline");
  $spin_abilfrom    = getRK_val("spin_abilfrom");
  $spin_abilto      = getRK_val("spin_abilto");
  $annotation       = getRK_val("annotation");

  // in case there are generic plot options defined:
  $plot_options     = getRK_val("plotoptions.code.printout");
  $plot_ops_main    = getRK_val("plotoptions.main");
  $plot_ops_type    = getRK_val("plotoptions.pointtype");
  $plot_ops_xlab    = getRK_val("plotoptions.xlab");
  $plot_ops_ylab    = getRK_val("plotoptions.ylab");

  ///////////////////////////////////
  // check for selected options
  // these two arrays will contain the options for the two functions that will be called:
  $options_lrtest = array() ;
  $options_plotgof = array() ;
  // plot all items?
  if($inp_items) {
    // for user convenience, we replace "-", ";" and space, split all input into an array
    // and join it again, separated by commas:
    $inp_items = str_replace("-",":",$inp_items);
    $arr_items = split('[ ;]', $inp_items);
    $options_plotgof[] = "beta.subset=c(".join(",", $arr_items).")"; }
  if($rad_splitcr == "mean" || $rad_splitcr == "all.r")
    $options_lrtest[] = "splitcr=\"$rad_splitcr\"";
  if($rad_splitcr == "vector")
    $options_lrtest[] = "splitcr=$splitvector";
  if($chk_se == "se")
    $options_lrtest[] = "se=TRUE";
  if($chk_confint == "conf") {
    if($spin_confint != "0.95")
      $options_plotgof[] = "conf=list(gamma=".round($spin_confint,2).", col=\"red\", lty=\"dashed\", ia=FALSE)";
    else
      $options_plotgof[] = "conf=list()"; }
  if($chk_ctrline == "ctrline") {
    if($spin_ctrline != "0.95")
      $options_plotgof[] = "ctrline=list(gamma=".round($spin_ctrline,2).", col=\"blue\", lty=\"solid\")";
    else
      $options_plotgof[] = "ctrline=list()"; }

/*
  if($ == "")
    $options[] = "=list(\"$\")" ;
*/

  // more advanced options
  // user defined ranges? we'll round it to two digits
  if(($spin_abilfrom != "-3" || $spin_abilto != "3") && $spin_abilfrom < $spin_abilto)
    $options_plotgof[] = "xlim=c(".round($spin_abilfrom,2).",".round($spin_abilto,2).")" ;
  // annotate lines and show legend?
  if($annotation == "number" || $annotation == "none" || $annotation == "identify")
    $options_plotgof[] = "tlab=\"$annotation\"" ;

        if ($final) { ?>
rk.header("Andersen's LR test")

rk.graph.on()
<?       }
        // only the following section will be generated for $final==false

        ?>
lr.res <- LRtest(<? getRK("x");
              if($options_lrtest) echo(", ".join(", ", $options_lrtest));
          ?>)
try(plotGOF(lr.res<?
              if($options_plotgof) echo(", ".join(", ", $options_plotgof));
              if($plot_options) echo($plot_options);
          ?>))
<?
        if ($final) { ?>
rk.graph.off()
<? }
}
?>