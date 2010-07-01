function calculate () {
	var object = getValue ("object");
	var is_data_frame = getValue ("sortby_frame.enabled");

	var saveto = object;
	if (getValue ("saveto_select") == "other") saveto = getValue ("saveto");

	echo (".GlobalEnv$" + saveto + " <- " + object + "[order (" + getValue ("sortby") + getValue ("order") + ")");
	if (is_data_frame) echo (",");
	echo ("]\n");
}
