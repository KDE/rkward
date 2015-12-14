function preview () {
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout (real) {
	echo ('x <- as.data.frame (matrix (' + getString ("value") + ', nrow=' + getString ("size") + ', ncol=' + getString ("size") + '))\n');
	if (!real) echo ('preview_data <- x');
}