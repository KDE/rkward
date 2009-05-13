<?
function preprocess () {
}

function calculate () {
?>
write.table ( x =  <? getRK("data") ; ?> , file =  "<? getRK("file") ; ?>" , append =  <? getRK("append") ; ?>  ,quote = <? getRK("quote") ?> ,  sep = <? getRK("sep") ?> , eol = "<? getRK("eol")  ?>" , na = "<? getRK("na")  ?>" ,  dec = <? getRK("dec") ?> , row.names =  <? if (getRK_val("rows") == "custoRow") getRK("rownames") ; else  getRK("rows")  ; ?> ,  col.names =  <? if (getRK_val("columns") == "custoCol") getRK("colnames") ; else  getRK("columns")  ; ?> , qmethod=  <? getRK("qmethod") ; ?> ) 
<?
}

function printout () {
	makeHeaderCode ("Write as table", array ("File" => getRK_val ("file"), "Data" => getRK_val ("data")));
}
?>
