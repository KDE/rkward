<?
// internal helper function
function quoteString ($string) {
	return ('"' . strtr ($string, array ('"'=>'\\"')) . '"');
}

function preprocess () {
}

function calculate () {
	$dec = getRK_val ("dec");
	if ($dec == "other") $dec = quoteString (getRK_val ("custom_dec"));
	$sep = getRK_val ("sep");
	if ($sep == "other") $sep = quoteString (getRK_val ("custom_sep"));
	$quote = getRK_val ("quote");
	if ($quote == "other") $quote = quoteString (getRK_val ("custom_quote"));

getRK("name"); ?> <- read.table (file="<? getRK("file"); ?>", header=<? getRK("header"); ?>, sep=<? echo ($sep); ?>, quote=<? echo ($quote); ?>, dec=<? echo ($dec); ?>, <? # doing row names (what a pity...) 
	if (getRK_val("rowname")!="NULL") {
		echo( "row.names = ");
		if (getRK_val("rowname")=="rowcol") echo (getRK("nomrow") . ",");
		else echo (getRK_val("rownames") . ",");
	}
	# doing col names (what a pity...)
	if (getRK_val("colname") == "custoCol") echo ( "col.names = " . getRK_val ("colnames") . ",");
	# doing col class (what a pity...)
	if (getRK_val("colclass") == "custoClass") echo( "colClasses = " . getRK_val ("custoClasses") . ",");
	#doing what is left
?> na.strings = "<? getRK("na")  ?>", nrows = <? getRK("nrows") ; ?>, skip =  <? getRK("skip") ; ?>, check.names = <? getRK("checkname") ; ?>, fill = <? getRK("fill") ; ?>, strip.white = <? getRK("stripwhite") ; ?>, blank.lines.skip = <? getRK("blanklinesskip") ; ?>, comment.char=<? echo (quoteString (getRK_val("commentchar"))); ?><? getRK("allow_escapes"); ?><? getRK("flush"); ?><? getRK("strings_as_factors"); ?>)
<?	if (getRK_val ("doedit")) { ?>

rk.edit (<? getRK ("name"); ?>)
<?	}
}

function printout () {
// produce the output
}

function cleanup () {
}
?>
