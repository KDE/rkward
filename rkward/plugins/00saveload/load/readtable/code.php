<?
	function preprocess () {
	}
	
	function calculate () {
getRK("name") ; ?> <- read.table(file =  "<? getRK("file") ; ?>", header = <? getRK("header") ?>, sep = <? getRK("sep") ?> , dec = <? getRK("dec") ?>, <? # doing row names (what a pity...) 
if (getRK_val("isrow")=="true") {
	echo( "row.names = ");
	if (getRK_val("rowname")=="NULL") echo (getRK_val("rowname") . ",");
	else if (getRK_val("rowname")=="rowcol") echo (getRK("nomrow") . ",");
	else if (getRK_val("rowname")=="custoRow") echo (getRK_val("rownames") . ",");
}
# doing col names (what a pity...)
if (getRK_val("colname") == "custoCol") echo ( "col.names = " . getRK_val ("colnames") . ",");
# doing col class (what a pity...)
if (getRK_val("colclass") == "custoClass") echo( "colClasses = " . getRK_val ("custoClasses") . ",");
#doing what is left?> na.strings = "<? getRK("na")  ?>" ,  nrows = <? getRK("nrows") ; ?> , skip =  <? getRK("skip") ; ?> , check.names = <? getRK("checkname") ; ?> , fill = <? getRK("fill") ; ?> , strip.white =  <? getRK("stripwhite") ; ?>, blank.lines.skip = <? getRK("blanklinesskip") ; ?> ,comment.char = "<? getRK("commentchar") ; ?>" )

<?
	}
	
	function printout () {
	// produce the output
	}
	
	function cleanup () {
	}
?>
