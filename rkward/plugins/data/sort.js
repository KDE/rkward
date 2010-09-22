function calculate () {
	var object = getValue ("object");
	var is_data_frame = (getValue ("sortby_frame.enabled") == "true");

	var saveto = object;
	if (getValue ("saveto_select") == "other") saveto = getValue ("saveto");
	var sortby = getValue ("sortby");
	if (!is_data_frame) sortby = object;

	echo (".GlobalEnv$" + saveto + " <- " + object + "[order (" + sortby + getValue ("order") + ")");
	if (is_data_frame) echo (",");
	echo ("]\n");
}
