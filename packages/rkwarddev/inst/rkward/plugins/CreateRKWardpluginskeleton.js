// this code was generated using the rkwarddev package.
//perhaps don't make changes here, but in the rkwarddev script instead!



function preprocess(){
	// add requirements etc. here
	echo("require(rkwarddev)\n");

	var brwDTEMPDIR = getValue("brw_DTEMPDIR");
	var chcOvrwrtxs = getValue("chc_Ovrwrtxs");
	echo("rkwarddev.required(\"0.06-5\")");
	echo("\n\n# define where the plugin should write its files\noutput.dir <- ");
	if(brwDTEMPDIR) {
		echo("\"" + brwDTEMPDIR + "\"");
	} else {
		echo("tempdir()");
	}
	echo("\n# overwrite an existing plugin in output.dir?\noverwrite <- ");
	if(chcOvrwrtxs) {
		echo("TRUE");
	} else {
		echo("FALSE");
	}
	echo("\n\n");
}

function calculate(){
	// read in variables from dialog
	var ocolOclInpPckgtxt = getList("ost_frmlDRDRPP.ocl_inpPckgtxt");
	var inpPluginnm = getString("inp_Pluginnm");
	var inpLicense = getString("inp_License");
	var inpShrtdscr = getString("inp_Shrtdscr");
	var inpVrsnnmbr = getString("inp_Vrsnnmbr");
	var inpRlsdtmpt = getString("inp_Rlsdtmpt");
	var inpHomepage = getString("inp_Homepage");
	var inpCategory = getString("inp_Category");
	var inpGivennam = getString("inp_Givennam");
	var inpFamilynm = getString("inp_Familynm");
	var inpEmail = getString("inp_Email");
	var chcAuthor = getBoolean("chc_Author");
	var chcMaintanr = getBoolean("chc_Maintanr");
	var brwDTEMPDIR = getString("brw_DTEMPDIR");
	var chcOvrwrtxs = getBoolean("chc_Ovrwrtxs");
	var chcAddwzrds = getBoolean("chc_Addwzrds");
	var chcIncldplg = getBoolean("chc_Incldplg");
	var chcOpnflsfr = getBoolean("chc_Opnflsfr");
	var chcAddplRKW = getBoolean("chc_AddplRKW");
	var chcShwthplg = getBoolean("chc_Shwthplg");
	var drpPlcntpmn = getString("drp_Plcntpmn");
	var inpRKWardmn = getString("inp_RKWardmn");
	var inpRKWardmx = getString("inp_RKWardmx");
	var inpRmin = getString("inp_Rmin");
	var inpRmax = getString("inp_Rmax");
	var inpSummary = getString("inp_Summary");
	var inpUsage = getString("inp_Usage");
	var frmDfndpndnChecked = getBoolean("frm_Dfndpndn.checked");
	var frmWrthlpflChecked = getBoolean("frm_Wrthlpfl.checked");

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
		if(chcAddwzrds) {
			arrOptSkeleton.push("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")");
		}
		if(optPluginmap) {
			arrOptSkeleton.push("\n\t" + optPluginmap);
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
	if(frmDfndpndnChecked && (optDependencies || ocolOclInpPckgtxt)) {
		echo("plugin.dependencies <- rk.XML.dependencies(");
		if(optDependencies) {
			echo(optDependencies);
		}
		if(optDependencies && ocolOclInpPckgtxt) {
			echo(",");
		}
		if(ocolOclInpPckgtxt) {
			echo("\n\tpackage=list(\n\t\tc(name=\"" + ocolOclInpPckgtxt.join("\"),\n\t\tc(name=\"") + "\")\n\t)");
		}
		echo("\n)\n\n");
	}
	if(frmWrthlpflChecked) {
		echo("############\n## help page\nplugin.summary <- rk.rkh.summary(\n\t");
		if(inpSummary) {
			echo("\"" + inpSummary + "\"\n)");
		} else {
			echo("\"" + inpShrtdscr + "\"\n)");
		}
		echo("\nplugin.usage <- rk.rkh.usage(\n\t\"" + inpUsage + "\"\n)\n\n");
	}
	echo("#############\n" + "## the main call\n" + "## if you run the following function call, files will be written to output.dir!\n" + "#############\n" + "# this is where things get serious, that is, here all of the above is put together into one plugin\n" + "plugin.dir <- rk.plugin.skeleton(\n\tabout=about.plugin,");
	if(frmDfndpndnChecked && optDependencies) {
		echo("\n\tdependencies=plugin.dependencies,");
	}
	echo("\n\tpath=output.dir,");
	echo("\n\toverwrite=overwrite,");
	if(frmWrthlpflChecked) {
		echo("\n\trkh=list(\n\t\tsummary=plugin.summary,\n\t\tusage=plugin.usage\n\t)," + "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\", \"rkh\"),");
	} else {
		echo("\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\"),");
	}
	echo(optSkeleton);
	echo("\n)\n\n");
}

function printout(){
	// printout the results
	echo("rk.header(\"Create RKWard plugin skeleton results\")\n");
echo("rk.print(\"\")\n");

}

