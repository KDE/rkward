<?
function preprocess () {
  // we'll need the ltm package, so in case it's not loaded...
?>
  require(ltm)
<?}

function calculate () {
  // let's read all values into php variables for the sake of readable code
  $spin_samples    = getRK_val("spin_samples");

?>unidim.res <- unidimTest(<? getRK("x");
                  // check if any options must be inserted
                  if($spin_samples != "100") echo(", B=$spin_samples") ;
 ?>)
<?}

function printout () {
  $save         = getRK_val("chk_save");
  $save_name    = getRK_val("save_name");
?>
rk.header ("Unidimensionality check (<? getRK("x"); ?>)")
rk.print (unidim.res)
<?
// check if results are to be saved:
if ($save && $save_name) {
?>
# keep results in current workspace
<? echo($save_name); ?> <<- unidim.res
<?}
}
?>