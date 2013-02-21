// this code was generated using the rkwarddev package.
//perhaps don't make changes here, but in the rkwarddev script instead!



function preprocess(){
	// add requirements etc. here
	echo("require(rkwarddev)\n");
}

function calculate(){
	// read in variables from dialog
	var inpPluginnm = getValue("inp_Pluginnm");
	var inpLicense = getValue("inp_License");
	var inpShrtdscr = getValue("inp_Shrtdscr");
	var inpVrsnnmbr = getValue("inp_Vrsnnmbr");
	var inpRlsdtmpt = getValue("inp_Rlsdtmpt");
	var inpHomepage = getValue("inp_Homepage");
	var inpCategory = getValue("inp_Category");
	var inpGivennam = getValue("inp_Givennam");
	var inpFamilynm = getValue("inp_Familynm");
	var inpEmail = getValue("inp_Email");
	var chcAuthor = getValue("chc_Author");
	var chcMaintanr = getValue("chc_Maintanr");
	var brwDTEMPDIR = getValue("brw_DTEMPDIR");
	var chcOvrwrtxs = getValue("chc_Ovrwrtxs");
	var chcAddwzrds = getValue("chc_Addwzrds");
	var chcIncldplg = getValue("chc_Incldplg");
	var chcOpnflsfr = getValue("chc_Opnflsfr");
	var chcAddplRKW = getValue("chc_AddplRKW");
	var chcShwthplg = getValue("chc_Shwthplg");
	var drpPlcntpmn = getValue("drp_Plcntpmn");
	var inpRKWardmn = getValue("inp_RKWardmn");
	var inpRKWardmx = getValue("inp_RKWardmx");
	var inpRmin = getValue("inp_Rmin");
	var inpRmax = getValue("inp_Rmax");
	var frmDfndpndnChecked = getValue("frm_Dfndpndn.checked");

	// the R code to be evaluated
	// define the array arrOptAuthorRole for values of R option "role"
	var arrOptAuthorRole = new Array();
		if(chcAuthor) {
			arrOptAuthorRole.push("\"aut\"");
		}
		if(chcMaintanr) {
			arrOptAuthorRole.push("\"cre\"");
		}
	// clean array arrOptAuthorRole from empty strings
	arrOptAuthorRole = arrOptAuthorRole.filter(String);
	// set the actual variable optAuthorRole with all values for R option "role"
	if(arrOptAuthorRole.length > 0) {
		var optAuthorRole = "role=c(" + arrOptAuthorRole.join(", ") + ")";
	} else {
		var optAuthorRole = "";
	}

	// define the array arrOptAuthor for values of R option "author"
	var arrOptAuthor = new Array();
		if(inpGivennam) {
			arrOptAuthor.push("given=\"" + inpGivennam + "\"");
		}
		if(inpFamilynm) {
			arrOptAuthor.push("family=\"" + inpFamilynm + "\"");
		}
		if(inpEmail) {
			arrOptAuthor.push("email=\"" + inpEmail + "\"");
		}
		if(optAuthorRole) {
			arrOptAuthor.push(optAuthorRole);
		}
	// clean array arrOptAuthor from empty strings
	arrOptAuthor = arrOptAuthor.filter(String);
	// set the actual variable optAuthor with all values for R option "author"
	if(arrOptAuthor.length > 0) {
		var optAuthor = ",\n\tauthor=person(" + arrOptAuthor.join(", ") + ")";
	} else {
		var optAuthor = "";
	}

	// define the array arrOptAbout for values of R option "about"
	var arrOptAbout = new Array();
		if(inpShrtdscr) {
			arrOptAbout.push("desc=\"" + inpShrtdscr + "\"");
		}
		if(inpVrsnnmbr) {
			arrOptAbout.push("version=\"" + inpVrsnnmbr + "\"");
		}
		if(inpRlsdtmpt) {
			arrOptAbout.push("date=\"" + inpRlsdtmpt + "\"");
		}
		if(inpHomepage) {
			arrOptAbout.push("url=\"" + inpHomepage + "\"");
		}
		if(inpLicense) {
			arrOptAbout.push("license=\"" + inpLicense + "\"");
		}
		if(inpCategory) {
			arrOptAbout.push("category=\"" + inpCategory + "\"");
		}
	// clean array arrOptAbout from empty strings
	arrOptAbout = arrOptAbout.filter(String);
	// set the actual variable optAbout with all values for R option "about"
	if(arrOptAbout.length > 0) {
		var optAbout = ",\n\tabout=list(" + arrOptAbout.join(", ") + ")";
	} else {
		var optAbout = "";
	}

	// define the array arrOptDependencies for values of R option "dependencies"
	var arrOptDependencies = new Array();
		if(frmDfndpndnChecked && inpRKWardmn) {
			arrOptDependencies.push("rkward.min=\"" + inpRKWardmn + "\"");
		}
		if(frmDfndpndnChecked && inpRKWardmx) {
			arrOptDependencies.push("rkward.max=\"" + inpRKWardmx + "\"");
		}
		if(frmDfndpndnChecked && inpRmin) {
			arrOptDependencies.push("R.min=\"" + inpRmin + "\"");
		}
		if(frmDfndpndnChecked && inpRmax) {
			arrOptDependencies.push("R.max=\"" + inpRmax + "\"");
		}
	// clean array arrOptDependencies from empty strings
	arrOptDependencies = arrOptDependencies.filter(String);
	// set the actual variable optDependencies with all values for R option "dependencies"
	if(arrOptDependencies.length > 0) {
		var optDependencies = ",\n\tdependencies=list(" + arrOptDependencies.join(", ") + ")";
	} else {
		var optDependencies = "";
	}

	// define the array arrOptPluginmap for values of R option "pluginmap"
	var arrOptPluginmap = new Array();
		if(drpPlcntpmn!= "test") {
			arrOptPluginmap.push("hierarchy=\"" + drpPlcntpmn + "\"");
		}
	// clean array arrOptPluginmap from empty strings
	arrOptPluginmap = arrOptPluginmap.filter(String);
	// set the actual variable optPluginmap with all values for R option "pluginmap"
	if(arrOptPluginmap.length > 0) {
		var optPluginmap = "pluginmap=list(" + arrOptPluginmap.join(", ") + ")";
	} else {
		var optPluginmap = "";
	}

	// define the array arrOptSkeleton for values of R option ""
	var arrOptSkeleton = new Array();
		if(brwDTEMPDIR) {
			arrOptSkeleton.push("\n\tpath=\"" + brwDTEMPDIR + "\"");
		}
		if(chcAddwzrds) {
			arrOptSkeleton.push("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")");
		}
		if(optPluginmap) {
			arrOptSkeleton.push("\n\t" + optPluginmap);
		}
		if(chcOvrwrtxs) {
			arrOptSkeleton.push("\n\toverwrite=TRUE");
		}
		if(chcIncldplg) {
			arrOptSkeleton.push("\n\ttests=TRUE");
		}
		if(chcOpnflsfr) {
			arrOptSkeleton.push("\n\tedit=TRUE");
		}
		if(chcAddplRKW) {
			arrOptSkeleton.push("\n\tload=TRUE");
		}
		if(chcShwthplg) {
			arrOptSkeleton.push("\n\tshow=TRUE");
		}
	// clean array arrOptSkeleton from empty strings
	arrOptSkeleton = arrOptSkeleton.filter(String);
	// set the actual variable optSkeleton with all values for R option ""
	if(arrOptSkeleton.length > 0) {
		var optSkeleton = "" + arrOptSkeleton.join(", ") + "";
	} else {
		var optSkeleton = "";
	}

	echo("about.plugin <- rk.XML.about(");
	if(inpPluginnm) {
		echo("\n\tname=\"" + inpPluginnm + "\"");
	}
	echo(optAuthor);
	echo(optAbout);
	echo("\n)\n\n");
	if(frmDfndpndnChecked && optDependencies) {
		echo("plugin.dependencies <- rk.XML.dependencies(" + optDependencies + "\n)\n\n");
	}
	echo("plugin.dir <- rk.plugin.skeleton(\n\tabout=about.plugin,");
	if(frmDfndpndnChecked && optDependencies) {
		echo("\n\tdependencies=plugin.dependencies,");
	}
	echo(optSkeleton);
	echo("\n)\n\n");
}

function printout(){
	// printout the results
	echo("rk.header(\"Create RKWard plugin skeleton results\")\n");
echo("rk.print(\"\")\n");

}

