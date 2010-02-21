function calculate () {
	echo ('write.table ( x =  ' + getValue("data") + ' , file =  "' + getValue("file") + '" , append =  ' + getValue("append") + '  ,quote = ' + getValue("quote") + ' ,  sep = ' + getValue("sep") + ' , eol = "' + getValue("eol") + '" , na = "' + getValue("na") + '" ,  dec = ' + getValue("dec") + ' , row.names =  ');
	if (getValue("rows") == "custoRow") echo (getValue("rownames")) ;
	else  echo (getValue("rows") + ' ,  col.names =  ');
	if (getValue("columns") == "custoCol") echo (getValue("colnames")) ;
	else  echo (getValue("columns") + ' , qmethod=  ' + getValue("qmethod") + ' ) \n');
}

function printout () {
	makeHeaderCode ("Write as table", new Array("File", getValue ("file"), "Data",  getValue ("data")));
}

