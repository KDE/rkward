function preprocess () {
	printIndented ("\t\t", "This is\n\t\a\ntest\n");
}

function calculate () {
	echo ('model = glm (' + getValue ("model") + ', data=' + getValue ("model.table") + ')\n');
	echo ('labels = ' + getValue ("model.labels") + '\n');
	echo ('result = anova (model)\n');
}

function printout () {
	new Header (i18n ("SimpleAnova"))
			.add (i18nc ("GLM Model specification", "Model"), getValue ("model"))
			.add (i18n ("Data"), getValue ("model.table"))
			.add ("Test", noquote ("print ('hi')")).print ();
	echo ('rk.echo (result)\n');

	comment ('i18n tests below');
	echo ('print (' + i18n ("This is an i18n text") + ')\n');
	echo ('print (' + i18n ('This is a single quoted i18n text, with a variety of quoting chars (\' " \` \" `), and a %1 inside:', 1) + ')\n');
	echo ('# A comment: ' + i18nc ("Dummy context", noquote ("This is a non-auto-quoted i18n'ed text with context")) + '\n');
	for (var i = 10; i > 0; --i) {
		echo ('print (' + i18np ("There was one green bottle standing on the %2", "There were %1 green bottles standing on the %2", i, i18n (noquote ("wall"))) + ')\n');
	}
	for (var i = 10; i > 0; --i) {
		echo ('print (' + i18ncp ("Dummy context", "There was one contextualized %2 bottle standing on the %3", "There were %1 contextualized %2 bottles standing on the %3", i, i18n (noquote ("green")), i18n (noquote ("wall"))) + ')\n');
	}
}
