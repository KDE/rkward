include ("convert_encoding.js");

function preprocess () {
	doPreprocess (false);
}

function doPreprocess (is_preview) {
	if (is_preview) {
		echo ('if (!base::require (XLConnect)) stop (' + i18n ("Preview not available, because package XLConnect is not installed or cannot be loaded.") + ')\n');
	} else {
		echo ('require (XLConnect)\n');
	}
	makeEncodingPreprocessCode ();
}

function preview () {
	doPreprocess (true);
	doCalculate (true);
}

function calculate () {
	doCalculate (false);
}

function doCalculate (is_preview) {
	var options = '';

	var range = getString ("range");
	if (range) options += ', region=' + quote ("range");
	else {
		options += makeOption ("startRow", getString ("startrow")) + makeOption ("startCol", getString ("startcol")) + makeOption ("endRow", getString ("endrow")) + makeOption ("endCol", getString ("endcol"));
	}

	if (!getBoolean ("autofitrow")) options += ', autoFitRow=FALSE';
	if (!getBoolean ("autofitcol")) options += ', autoFitCol=FALSE';
	if (!getBoolean ("header")) options += ', header=FALSE';
	options += makeOption ("rownames", getString ("rownames"));

	if (getValue ("coltypes.columns") > 0) {
		options += ', colTypes=c (' + getList ("coltypes.row.0") + ')';
	}

	echo ('data <- readWorksheetFromFile (' + quote (getString ("file")) + ', sheet=' + getString ("sheet") + options + ')\n');
	makeEncodingCall ('data');
	echo ('\n');
	if (is_preview) {
		echo ('preview_data <- data[1:min(50,dim(data)[1]),1:min(50,dim(data)[2])]\n');
	} else {
		var object = getString ("saveto");
		echo ('.GlobalEnv$' + object + ' <- data		'); comment ('assign to globalenv()');
		if (getValue ("doedit")) {
			echo ('rk.edit (.GlobalEnv$' + object + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Import SPSS data")).addFromUI ("file").addFromUI ("saveto").print ();
}

 
