function calculate () {
	echo (getList ("select").join ("\n"));
	echo ("\n\n");
	echo (getList ("valueselect.labeled").join ("\n"));
	echo ("\n\n");
	echo (getList ("valueselect.selected").join ("\n"));
	echo ("\n\n");
}

