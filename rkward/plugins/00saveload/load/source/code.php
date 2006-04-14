<?
	function preprocess () {
	}
	
	function calculate () {
?>
source ( file =  "<? getRK("file") ; ?>" ,local =  <? getRK("local") ; ?> , echo =  <? getRK("echo") ; ?> , print.eval =  <? getRK("printeval") ; ?> , verbose =  <? getRK("verbose") ; ?> , prompt.echo =   <? getRK("promptecho") ; ?> , max.deparse.length =  <? getRK("maxdeparselength") ; ?> , chdir =  <? getRK("chdir") ; ?> )
<?
	}
	
	function printout () {
	// produce the output
	}
	
	function cleanup () {
	}
?>
