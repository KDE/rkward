<?
	function preprocess () {
	}
	
	function calculate () {
?>
write ( x =  <? getRK("data") ; ?> , file =  "<? getRK("file") ; ?>" , ncolumns =  <? getRK("ncolumns") ; ?> , append =  <? getRK("append") ; ?> )
<?
	}
	
	function printout () {
	// produce the output
	}
	
	function cleanup () {
	}
?>
