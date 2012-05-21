function calculate () {
	echo ("## Manual set: ##\n")
	echo ("row_count <- " + getValue ("mset.row_count") + "\n");
	echo ("current_row <- " + getValue ("mset.current_row") + "\n");
	echo ("## Driven set: ##\n")
	echo ("row_count <- " + getValue ("set.row_count") + "\n");
	echo ("current_row <- " + getValue ("set.current_row") + "\n");
}
