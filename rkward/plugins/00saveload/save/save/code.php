<?
	function preprocess () {
	}
	
	function calculate () {
$vars = str_replace ("\n", ",", trim (getRK_val ("data"))) ;
?>

save ( <? echo ($vars) ; ?> , file =  "<? getRK("file") ; ?>" , ascii =  <? getRK("ascii") ; ?> , compress =  <? getRK("compress") ; ?> )

<?
	}
	
	function printout () {
	// produce the output
?>

<?
	}
	
	function cleanup () {
?>


<?
	}
?>
