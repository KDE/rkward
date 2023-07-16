/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// this code was generated using the rkwarddev package.
// perhaps don't make changes here, but in the rkwarddev script instead!



function preview(){
	preprocess(true);
	calculate(true);
	printout(true);
}

function preprocess(is_preview){
	// add requirements etc. here

}

function calculate(is_preview){
	// the R code to be evaluated

	var data = getString ('var_data');
	var filter_var = getString ('vrsl_Fltrbyvr.shortname');
	var filter_expr = getString ('inp_Exprssnr');

	echo ('\tsset.result <- subset(');
	if (data != '') {
		echo ('\n\t\t' + data);

		// row filter
		var row_filter_exp = '';
		if (filter_var != '') {
			var filter_operand = getString ('swt_csfltrdt');
			if (getBoolean ('swt_swtcsflt')) {
				var range_limit = '';
				var max_range = '';
				var fmin = getString ('inp_Mnmmrmpt');
				var fmax = getString ('inp_Mxmmrmpt');
				var fmininc = getBoolean ('mininc');
				var fmaxinc = getBoolean ('maxinc');
				if (fmin != '') range_limit = filter_var + ' >' + (fmininc ? '= ' : ' ') + fmin;
				if (fmax != '') max_range = filter_var + ' <' + (fmaxinc ? '= ' : ' ') + fmax;
				if (!(max_range == '' || range_limit == '')) range_limit = '(' + range_limit + ') & (' + max_range + ')';
				else range_limit += max_range;

				if (filter_operand == 'range') row_filter_exp += range_limit;
				else row_filter_exp += '!(' + range_limit + ')';
			} else if (getString ('case_filter_data_mode') == 'logical') {
				if (filter_operand == 'TRUE') row_filter_exp += filter_var;
				else row_filter_exp += '!' + filter_var;
			} else {
				var input_filter = getString ('inp_Vlpstdss');
				if (filter_operand == '!%in%') row_filter_exp += '!(' + filter_var + ' %in% ' + input_filter + ')';
				else row_filter_exp += filter_var + ' ' + filter_operand + ' ' + input_filter;
			}
		}
		if (filter_expr != '') {
			if (row_filter_exp != '') row_filter_exp = '(' + row_filter_exp + ') & (' + filter_expr + ')';
			else row_filter_exp = filter_expr;
		}
		if (row_filter_exp != '') echo (',\n\t\t' + row_filter_exp);

		// column filter
		if (getBoolean ('frm_Onlyssbs.checked')) {
			var selected_vars = getList ('vrsl_Slctdvrb.shortname').join (', ');
			if (selected_vars != '') echo (',\n\t\tselect=c (' + selected_vars + ')');
		}
	}
	echo ('\n\t)\n\n');
	if (is_preview) {
		echo ('preview_data <- sset.result[1:min(dim(sset.result)[1],500),1:min(dim(sset.result)[2],100),drop=FALSE]\n');
	}

}

function printout(is_preview){
	// printout the results



	if(!is_preview) {
		//// save result object
		// read in saveobject variables
		var svbSvrsltst = getValue("svb_Svrsltst");
		var svbSvrsltstActive = getValue("svb_Svrsltst.active");
		var svbSvrsltstParent = getValue("svb_Svrsltst.parent");
		// assign object to chosen environment
		if(svbSvrsltstActive) {
			echo(".GlobalEnv$" + svbSvrsltst + " <- sset.result\n");
		}	
	}

}

