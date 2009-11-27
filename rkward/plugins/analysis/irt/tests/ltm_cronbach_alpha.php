<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $data            = getRK_val("x");
  $chk_select      = getRK_val("chk_select");
  $inp_items       = getRK_val("inp_items");
  $spin_samples    = getRK_val("spin_samples");
  $chk_standard    = getRK_val("chk_standard");
  $chk_na          = getRK_val("chk_na");
  $chk_bsci        = getRK_val("chk_bsci");
  $spin_ci         = getRK_val("spin_ci");

  // reformat $inp_items
  if($inp_items)
    $inp_items       = str_replace("\n", ", ", preg_replace("/(.+)\[\[(.+)\]\]/", "$2", $inp_items));

  ///////////////////////////////////
  // check for selected options
  $options = array() ;
  if($chk_standard == "standard")
    $options[] = "standardized=TRUE" ;
  if($chk_bsci == "bsci")
    $options[] = "CI=TRUE" ;
  if($spin_ci != ".95") {
    $cilo = (1-$spin_ci)/2 ;
    $cihi = 1-$cilo ;
    $options[] = "probs=c($cilo, $cihi)" ;
    }
  if($spin_samples != "1000")
    $options[] = "B=$spin_samples" ;
  if($chk_na == "rm")
    $options[] = "na.rm=TRUE" ;


?>cronalpha.res <- cronbach.alpha(<?
		  if($data && $chk_select && $inp_items)
		    echo("subset(".$data.", select=c(".$inp_items."))");
		  else
		    echo($data);
                  // check if any advanced control options must be inserted
                  if($options) echo(", ".join(", ", $options));
 ?>)
<?}

function printout () {
  $inp_items       = getRK_val("inp_items");
  // reformat $inp_items
  if($inp_items)
    $inp_items       = str_replace("\n", ", ", preg_replace("/(.+)\[\[\"(.+)\"\]\]/", "$2", $inp_items));
?>
rk.header ("Cronbach's alpha (<?
  getRK("x");
  if($inp_items)
    echo(", subset: ".$inp_items);
 ?>)")
rk.print (cronalpha.res)
<?
}
?>