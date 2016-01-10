function preview () {
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout (real) {
	echo ('x <- as.data.frame (matrix (' + getString ("value") + ', nrow=' + getString ("size") + ', ncol=' + getString ("size") + '))\n');
	if (!real) {
		// Not to be imitated, for testing purposes, we handle both a data-preview, and an html-preview, here
		echo ('preview_data <- x\n');
		echo ('rk.print (x)\n');
	}
}