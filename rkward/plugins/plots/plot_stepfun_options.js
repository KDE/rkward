function printout () {

	var options = "";
	var addtoplot = getValue ("addtoplot");
	if (addtoplot) options += ', add=TRUE';

	var lty = getValue ("linetype");
	if (lty != "") options += ", lty=\"" + lty + "\"";

	var verticals = getValue ("verticals");
	if (verticals) {
		options += ', verticals=TRUE' + getValue ("col_vert.code.printout");
	} else options += ', verticals=FALSE';

	var do_points = getValue ("do_points");
	if (do_points) {
		options += getValue ("col_points.code.printout");
	} else options += ', do.points=FALSE';

	var col_hor = getValue ("col_hor.code.printout");
	if (col_hor != "") options += getValue ("col_hor.code.printout");

	if (getValue ("allow_col01line")) {
		var col_y0 = getValue ("col_y0.code.printout");
		var col_y1 = getValue ("col_y1.code.printout");
		if ((col_y0 != "") && (col_y1 != "")) {
			options += ", col.01line=c(" + col_y0 + "," + col_y1 + ")";
		} else if ((col_y0 != "") || (col_y1 != "")) {
			options += ", col.01line=" + col_y0 + col_y1;
		} // col.01line option to plot.ecdf()
	}

	echo (options);
}

