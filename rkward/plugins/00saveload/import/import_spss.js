/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("convert_encoding.js");

function preprocess () {
	echo ('require (foreign)\n');
	makeEncodingPreprocessCode ();
}

function preview () {
	preprocess ();
	calculate (true);
}

function calculate (is_preview) {
	var data_frame = "";
	var data_frame_opt = "";
	if (getValue ("data_frame") || is_preview) {
		data_frame = true;
		data_frame_opt = ", to.data.frame=TRUE";
	}

	var labels_opt = "";
	if (getValue ("use_labels")) {
		labels_opt += ", max.value.labels=" + getValue ("labels_limit");
		if (getValue ("trim_labels")) labels_opt += ", trim.factor.names=TRUE";
	} else {
		labels_opt = ", use.value.labels=FALSE";
	}

	var object = getValue ("saveto");

	echo ('data <- read.spss ("' + getValue ("file") + '"' + data_frame_opt + labels_opt + ')\n');
	makeEncodingCall ('data');
	if (getValue ("convert_var_labels")) {
		echo ('\n');
		comment ('set variable labels for use in RKWard');
		echo ('labels <- attr (data, "variable.labels");\n');
		echo ('if (!is.null (labels)) {\n');
		echo ('	for (i in 1:length (labels)) {\n');
		echo ('		col <- make.names (names (labels[i]))\n');
		echo ('		if (!is.null (col)) {\n');
		echo ('			rk.set.label (data[[col]], labels[i])\n');
		echo ('		}\n');
		echo ('	}\n');
		echo ('}\n');
	}
	echo ('\n');
	if (is_preview) {
		echo ('preview_data <- data[1:min(50,dim(data)[1]),1:min(50,dim(data)[2]),drop=FALSE]\n');
	} else {
		echo ('.GlobalEnv$' + object + ' <- data		'); comment ('assign to globalenv()');
		if (getValue ("doedit") && data_frame) {
			echo ('rk.edit (.GlobalEnv$' + object + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Import SPSS data")).addFromUI ("file").addFromUI ("saveto").print ();
}

