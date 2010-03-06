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
  $inp_items        = getRK_val("inp_items");
  $spin_abilfrom    = getRK_val("spin_abilfrom");
  $spin_abilto      = getRK_val("spin_abilto");
  $spin_probfrom    = getRK_val("spin_probfrom");
  $spin_probto      = getRK_val("spin_probto");
  $annotation       = getRK_val("annotation");
  $chk_ask          = getRK_val("chk_ask");
  $chk_mplot        = getRK_val("chk_mplot");

  // in case there are generic plot options defined:
  $plot_options     = getRK_val("plotoptions.code.printout");
  $plot_ops_main    = getRK_val("plotoptions.main");
  $plot_ops_type    = getRK_val("plotoptions.pointtype");
  $plot_ops_xlab    = getRK_val("plotoptions.xlab");
  $plot_ops_ylab    = getRK_val("plotoptions.ylab");

  ///////////////////////////////////
  // check for selected options
  $options = array() ;
  // plot all items?
  if($inp_items) {
    // for user convenience, we replace "-", ";" and space, split all input into an array
    // and join it again, separated by commas:
    $inp_items = str_replace("-",":",$inp_items);
    $arr_items = split('[ ;]', $inp_items);
    $options[] = "item.subset=c(".join(",", $arr_items).")"; }
  if($chk_mplot == "mplot")
    $options[] = "mplot=TRUE" ;
  if($chk_ask != "ask")
    $options[] = "ask=FALSE" ;

  // more advanced options
  // user defined ranges? we'll round it to two digits
  if(($spin_abilfrom != "-4" || $spin_abilto != "4") && $spin_abilfrom < $spin_abilto)
    $options[] = "xlim=c(".round($spin_abilfrom,2).",".round($spin_abilto,2).")" ;
  if(($spin_probfrom != "0" || $spin_probto != "1") && $spin_probfrom < $spin_probto)
    $options[] = "ylim=c(".round($spin_probfrom,2).",".round($spin_probto,2).")" ;
  // annotate lines and show legend?
  if($annotation == "plain")
    $options[] = "legpos=FALSE" ;

        if ($final) { ?>
rk.header("Rating scale model plot")

rk.graph.on()
<?       }
        // only the following section will be generated for $final==false

        ?>
try(plotICC(<? getRK("x");
              if($options) echo(", ".join(", ", $options));
              if($plot_options) echo($plot_options);
          ?>))
<?
        if ($final) { ?>
rk.graph.off()
<? }
}
?>