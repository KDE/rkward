function preprocess () {
	printIndented ("\t\t", "This is\n\t\a\ntest");
	printIndented ("---", getValue ("embedded.code.preprocess"));
}

function calculate () {
	echo ('model = glm (' + getValue ("model") + ', data=' + getValue ("model.table") + ')\n');
	echo ('labels = ' + getValue ("model.labels") + '\n');
	echo ('result = anova (model)\n');

	printIndented ("---", getValue ("embedded.code.calculate"));
}

function printout () {
	makeHeaderCode ("SimpleAnova", new Array ("Model", getValue ("model"), "Data", getValue ("model.table"), "Test", noquote ("print ('hi')")))
//	makeHeaderCode ("SimpleAnova", new Array ("Model", "Data"))
	echo ('rk.echo (result)');
}
