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

function calculate () {
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

	echo ('.GlobalEnv$' + saveto + ' <- ' + object + '[order (' + sortby + getString ("order") + ')');
	if (is_data_frame) echo (',');
	echo (']\n');
}
