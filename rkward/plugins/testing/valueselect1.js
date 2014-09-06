function calculate () {
	echo (getList ("select").join ("\n"));
	echo ("\n\n");
	echo (getList ("valueslot").join ("\n"));
	echo ("\n\n");
}

