/* NOTE: This file is currently not used by the color_chooser plugin.
 Pehaps we can start using it again, now that a full PHP backend no longer needs to be started for this simple plugin. TODO: test it.
*/

function printout () {
	var col = getValue ("color");
	if (empty (col)) col = getValue ("default_color");
	if (!empty (col)) col = getValue ("argument") + "\"" + col + "\"";

	echo (col);
}
