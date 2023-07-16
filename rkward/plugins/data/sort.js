/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preview () {
	calculate (true);
}

function makeSortItem (objectname, reversed, mode, conversion_custom) {
	var obj = objectname;
	if (mode == 'character') obj = 'as.character (' + obj + ')';
	else if (mode == 'number') obj = 'as.numeric (' + obj + ')';
	else if (mode == 'custom') {
		obj = conversion_custom.replace (/_X_/g, obj);
	}
	if (reversed) {
		if (mode != 'number') obj = '-xtfrm (' + obj + ')';
		else obj = '-' + obj;
	}
	return obj;
}

function calculate (is_preview) {
	var object = getString ("object");
	var is_data_frame = getBoolean ("sortby.enabled");

	var saveto = object;
	if (getString ("saveto_select") == 'other') saveto = getString ("saveto");
	var sortby;
	if (is_data_frame) {
		sortby = "";
		var sortbylist = getList ("sortby");
		var sortbyreverse = getList ("multi_sortby.reverse");
		var sortbymode = getList ("multi_sortby.conversion");
		var sortbycustom = getList ("multi_sortby.conversion_custom");
		for (var i = 0; i < sortbylist.length; ++i) {
			if (i > 0) sortby = sortby + ', ';
			sortby = sortby + makeSortItem (sortbylist[i], sortbyreverse[i] != "", sortbymode[i], sortbycustom[i]);
		}
	} else {
		sortby = makeSortItem (object, false, getString ("conversion"), getString ("conversion_custom"));
	}

	var sorted = object + '[order (' + sortby + getString ("order") + ')';
	if (is_data_frame) sorted = sorted + ',';
	sorted = sorted + ']';

	if (is_preview) {
		echo ('sorted <- ' + sorted + '\n');
		echo ('sorted <- as.data.frame (sorted)\n');
		echo ('preview_data <- sorted[1:min(dim(sorted)[1],200),1:min(dim(sorted)[2],50),drop=FALSE]\n');
	} else {
		echo ('.GlobalEnv$' + saveto + ' <- ' + sorted + '\n');
	}
}
