function calculate () {
	echo ("## Manual set: ##\n")
	echo ("row_count <- " + getValue ("mset.row_count") + "\n");
	echo ("current_row <- " + getValue ("mset.current_row") + "\n");
	echo ("mset.contents.enabled <- " + getValue ("mset.contents.enabled") + "\n");
	echo ("## Driven set: ##\n")
	echo ("row_count <- " + getValue ("set.row_count") + "\n");
	echo ("current_row <- " + getValue ("set.current_row") + "\n");
	echo ("set.contents.enabled <- " + getValue ("set.contents.enabled") + "\n");

	var codeprops = getList ("set.plotoption_printout");
	for (i = 0; i < codeprops.length; ++i) {
		echo ("Plotoption string printout " + i + " in driven set: " + codeprops[i] + "\n");
	}
	codeprops = getList ("set.plotoption_pre");
	for (i = 0; i < codeprops.length; ++i) {
		echo ("Plotoption string preprocess " + i + " in driven set: " + codeprops[i] + "\n");
	}
}
