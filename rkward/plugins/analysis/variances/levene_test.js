function preprocess () {
	echo ('require(car)\n');
}

function calculate () {
	echo ('result <- levene.test (' + getValue ("y") + ', ' + getValue ("group") + ')\n');
}

function printout () {
	echo ('names <- rk.get.description (' + getValue ("y") + ', ' + getValue ("group") + ')\n');
	echo ('\n');
	echo ('rk.header ("Levene\'s Test", list ("response variable", names[1], "groups", names[2]))\n');
	echo ('\n');
	echo ('rk.print (result)\n');
}

