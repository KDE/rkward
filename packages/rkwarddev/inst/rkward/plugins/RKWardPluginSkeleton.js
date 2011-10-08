function preprocess(){
	// add requirements etc. here.
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
	var chcDfndpndn = getValue("chc_Dfndpndn");
	var inpRKWardmn = getValue("inp_RKWardmn");
	var inpRKWardmx = getValue("inp_RKWardmx");
	var inpRmin = getValue("inp_Rmin");
	var inpRmax = getValue("inp_Rmax");

	// put the R code to be evaluated here.
	// define the array arrOptAuthorRole for values of R option "role"
	var arrOptAuthorRole = new Array();
		if(chcAuthor) {
			arrOptAuthorRole.push("\"" + chcAuthor + "\"");
		} else {}
		if(chcMaintanr) {
			arrOptAuthorRole.push("\"" + chcMaintanr + "\"");
		} else {}
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
		} else {}
		if(inpFamilynm) {
			arrOptAuthor.push("family=\"" + inpFamilynm + "\"");
		} else {}
		if(inpEmail) {
			arrOptAuthor.push("email=\"" + inpEmail + "\"");
		} else {}
		if(optAuthorRole) {
			arrOptAuthor.push(optAuthorRole);
		} else {}
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
		} else {}
		if(inpVrsnnmbr) {
			arrOptAbout.push("version=\"" + inpVrsnnmbr + "\"");
		} else {}
		if(inpRlsdtmpt) {
			arrOptAbout.push("date=\"" + inpRlsdtmpt + "\"");
		} else {}
		if(inpHomepage) {
			arrOptAbout.push("url=\"" + inpHomepage + "\"");
		} else {}
		if(inpLicense) {
			arrOptAbout.push("license=\"" + inpLicense + "\"");
		} else {}
		if(inpCategory) {
			arrOptAbout.push("category=\"" + inpCategory + "\"");
		} else {}
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
		if(chcDfndpndn && inpRKWardmn) {
			arrOptDependencies.push("rkward.min=\"" + inpRKWardmn + "\"");
		} else {}
		if(chcDfndpndn && inpRKWardmx) {
			arrOptDependencies.push("rkward.max=\"" + inpRKWardmx + "\"");
		} else {}
		if(chcDfndpndn && inpRmin) {
			arrOptDependencies.push("R.min=\"" + inpRmin + "\"");
		} else {}
		if(chcDfndpndn && inpRmax) {
			arrOptDependencies.push("R.max=\"" + inpRmax + "\"");
		} else {}
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
		} else {}
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
		if(inpPluginnm) {
			arrOptSkeleton.push("\n\tname=\"" + inpPluginnm + "\"");
		} else {}
		if(brwDTEMPDIR) {
			arrOptSkeleton.push("\n\tpath=\"" + brwDTEMPDIR + "\"");
		} else {}
		if(chcAddwzrds) {
			arrOptSkeleton.push("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")");
		} else {}
		if(optPluginmap) {
			arrOptSkeleton.push("\n\t" + optPluginmap);
		} else {}
		if(chcOvrwrtxs) {
			arrOptSkeleton.push("\n\toverwrite=TRUE");
		} else {}
		if(chcIncldplg) {
			arrOptSkeleton.push("\n\ttests=TRUE");
		} else {}
		if(chcOpnflsfr) {
			arrOptSkeleton.push("\n\tedit=TRUE");
		} else {}
		if(chcAddplRKW) {
			arrOptSkeleton.push("\n\tload=TRUE");
		} else {}
		if(chcShwthplg) {
			arrOptSkeleton.push("\n\tshow=TRUE");
		} else {}
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
	} else {}
	echo(optAuthor);
	echo(optAbout);
	echo(optDependencies);
	echo("\n)\n\n");
	echo("plugin.dir <- rk.plugin.skeleton(\n\tabout=about.plugin,");
	echo(optSkeleton);
	echo("\n)\n\n");
}

function printout(){
	// printout the results
	echo("rk.header(\"RKWard Plugin Skeleton results\", level=2)\n");
	echo("rk.print(\"\")\n");
}