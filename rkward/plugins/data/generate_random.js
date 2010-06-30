function calculate () {
	if (getValue ("length.enabled.numeric")) {
		length = getValue ("length");
	} else {	// this happens when the saveto.parent is a data.frame, only.
		length = "dim (" + getValue ("saveto.parent") + ")[1]";
	}

	echo (".GlobalEnv$" + getValue ("saveto") + " <- rnorm (" + length + ", mean=" + getValue ("mean") + ", sd=" + getValue ("sd") + ")\n");
}
