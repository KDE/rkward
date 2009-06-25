<?
// internal helper function
function quoteString ($string) {
	return ('"' . strtr ($string, array ('"'=>'\\"')) . '"');
}

function preprocess () {
}

function calculate () {
	$quick = getRK_val ("quick");
	if ($quick == "table") {
		$dec = getRK_val ("dec");
		if ($dec == "other") $dec = quoteString (getRK_val ("custom_dec"));
		$sep = getRK_val ("sep");
		if ($sep == "other") $sep = quoteString (getRK_val ("custom_sep"));
		$quote = getRK_val ("quote");
		if ($quote == "other") $quote = quoteString (getRK_val ("custom_quote"));
		$header = getRK_val ("header");
		$fill = getRK_val ("fill");
		$comchar = quoteString (getRK_val ("commentchar"));
		$tableOptions = ", header={$header}, sep={$sep}, quote={$quote}, dec={$dec}, fill={$fill}, comment.char={$comchar}";
	} else {
		$tableOptions = "";
	}
// Other method is to use read.table and show all the options - more transparent
?>imported <<- read.<? echo ($quick); ?> (file="<? getRK("file"); ?>"<? echo ($tableOptions); ?>, <? # doing row names (what a pity...) 
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
?> na.strings = "<? getRK("na")  ?>", nrows = <? getRK("nrows") ; ?>, skip = <? getRK("skip") ; ?>, check.names = <? getRK("checkname") ; ?>, strip.white = <? getRK("stripwhite") ; ?>, blank.lines.skip = <? getRK("blanklinesskip") ; ?><? getRK("allow_escapes"); ?><? getRK("flush"); ?><? getRK("strings_as_factors"); ?>)

# copy from the local environment to globalenv()
assign("<? getRK("name"); ?>", imported, envir=globalenv())
<?	if (getRK_val ("doedit")) { ?>

rk.edit (<? getRK ("name"); ?>)
<?	}
}

function printout () { 
	makeHeaderCode ("Import text / csv data", array ("File" => getRK_val ("file"), "Import as" => getRK_val ("name")));
}
?>
