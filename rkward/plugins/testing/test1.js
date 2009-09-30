function preprocess () {
	printIndented ("\t\t", "This is\n\t\a\ntest");
}

function calculate () {
	echo ('model = glm (' + getValue ("model") + ', data=' + getValue ("model.table") + ')\n');
	echo ('labels = ' + getValue ("model.labels") + '\n');
	echo ('result = anova (model)');
}

function printout () {
	makeHeaderCode ("SimpleAnova", new Array ("Model", getValue ("model"), "Data", getValue ("model.table"), "Test", noquote ("print ('hi')")))
//	makeHeaderCode ("SimpleAnova", new Array ("Model", "Data"))
	echo ('rk.echo (result)');
}
