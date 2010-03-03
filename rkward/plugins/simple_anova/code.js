function calculate () {
	echo ('model = glm (' + getValue ("model") + ', data=' + getValue ("model.table") + ')\n');
	echo ('labels = ' + getValue ("model.labels") + ' \n');
	echo ('result = anova (model)\n');
}

function printout () {
	echo ('rk.header ("Simple Anova")\n');
	echo ('rk.print (result)\n');
}

