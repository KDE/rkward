// this code was generated using the rkwarddev package.
//perhaps don't make changes here, but in the rkwarddev script instead!



function preprocess(){
	// add requirements etc. here
	echo("require(rkwarddev)\n");

	var brwDTEMPDIR = getValue("brw_DTEMPDIR");
	var chcOvrwrtxs = getValue("chc_Ovrwrtxs");
	var chcGsRKW060 = getValue("chc_GsRKW060");
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
	echo("\n# if you set guess.getters to TRUE, the resulting code will need RKWard >= 0.6.0\nguess.getter <- ");
	if(chcGsRKW060) {
		echo("TRUE");
	} else {
		echo("FALSE");
	}
	echo("\n\n");
}

function calculate(){
	// read in variables from dialog

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
	var ocolOclInpGvnnmtx = getList("ost_fPPPPGGFFE.ocl_inpGvnnmtx");
	var ocolOclInpFmlynmt = getList("ost_fPPPPGGFFE.ocl_inpFmlynmt");
	var ocolOclInpEmaltxt = getList("ost_fPPPPGGFFE.ocl_inpEmaltxt");
	var ocolOclChcAthrstt = getList("ost_fPPPPGGFFE.ocl_chcAthrstt");
	var ocolOclChcMntnrst = getList("ost_fPPPPGGFFE.ocl_chcMntnrst");
	var ocolOclChcCntrbtr = getList("ost_fPPPPGGFFE.ocl_chcCntrbtr");
	var brwDTEMPDIR = getString("brw_DTEMPDIR");
	var drpPlcntpmn = getString("drp_Plcntpmn");
	var inpNmnmnplg = getString("inp_Nmnmnplg");
	var inpRKWardmn = getString("inp_RKWardmn");
	var inpRKWardmx = getString("inp_RKWardmx");
	var inpRmin = getString("inp_Rmin");
	var inpRmax = getString("inp_Rmax");
	var inpPackage = getString("inp_Package");
	var inpMin = getString("inp_min");
	var inpMax = getString("inp_max");
	var inpRepostry = getString("inp_Repostry");
	var ocolOclInpPckgtxt = getList("ost_fDRDRPPRRP.ocl_inpPckgtxt");
	var ocolOclInpmintext = getList("ost_fDRDRPPRRP.ocl_inpmintext");
	var ocolOclInpmaxtext = getList("ost_fDRDRPPRRP.ocl_inpmaxtext");
	var ocolOclInpRpstryt = getList("ost_fDRDRPPRRP.ocl_inpRpstryt");
	var inpSummary = getString("inp_Summary");
	var inpUsage = getString("inp_Usage");
	var chcAuthor = getBoolean("chc_Author.state");
	var chcMaintanr = getBoolean("chc_Maintanr.state");
	var chcContrbtr = getBoolean("chc_Contrbtr.state");
	var chcOvrwrtxs = getBoolean("chc_Ovrwrtxs.state");
	var chcAddwzrds = getBoolean("chc_Addwzrds.state");
	var chcIncldplg = getBoolean("chc_Incldplg.state");
	var chcOpnflsfr = getBoolean("chc_Opnflsfr.state");
	var chcAddplRKW = getBoolean("chc_AddplRKW.state");
	var chcShwthplg = getBoolean("chc_Shwthplg.state");
	var chcGsRKW060 = getBoolean("chc_GsRKW060.state");
	var frmDfndpndnChecked = getBoolean("frm_Dfndpndn.checked");
	var frmWrthlpflChecked = getBoolean("frm_Wrthlpfl.checked");

	// the R code to be evaluated
	// define the array arrOptAbout for values of R option "about"
	var arrOptAbout = new Array();
		if(inpShrtdscr) {
			arrOptAbout.push("desc=\"" + inpShrtdscr + "\"");
		} else {
			arrOptAbout.push();
		}
		if(inpVrsnnmbr) {
			arrOptAbout.push("version=\"" + inpVrsnnmbr + "\"");
		} else {
			arrOptAbout.push();
		}
		if(inpRlsdtmpt) {
			arrOptAbout.push("date=\"" + inpRlsdtmpt + "\"");
		} else {
			arrOptAbout.push();
		}
		if(inpHomepage) {
			arrOptAbout.push("url=\"" + inpHomepage + "\"");
		} else {
			arrOptAbout.push();
		}
		if(inpLicense) {
			arrOptAbout.push("license=\"" + inpLicense + "\"");
		} else {
			arrOptAbout.push();
		}
		if(inpCategory) {
			arrOptAbout.push("category=\"" + inpCategory + "\"");
		} else {
			arrOptAbout.push();
		}
	// clean array arrOptAbout from empty strings
	arrOptAbout = arrOptAbout.filter(String);
	// set the actual variable optAbout with all values for R option "about"
	if(arrOptAbout.length > 0) {
		var optAbout = ", about=list(" + arrOptAbout.join(", ") + ")";
	} else {
		var optAbout = "";
	}

	// define the array arrOptDependencies for values of R option "dependencies"
	var arrOptDependencies = new Array();
		if(frmDfndpndnChecked && inpRKWardmn) {
			arrOptDependencies.push("rkward.min=\"" + inpRKWardmn + "\"");
		} else {
			arrOptDependencies.push();
		}
		if(frmDfndpndnChecked && inpRKWardmx) {
			arrOptDependencies.push("rkward.max=\"" + inpRKWardmx + "\"");
		} else {
			arrOptDependencies.push();
		}
		if(frmDfndpndnChecked && inpRmin) {
			arrOptDependencies.push("R.min=\"" + inpRmin + "\"");
		} else {
			arrOptDependencies.push();
		}
		if(frmDfndpndnChecked && inpRmax) {
			arrOptDependencies.push("R.max=\"" + inpRmax + "\"");
		} else {
			arrOptDependencies.push();
		}
	// clean array arrOptDependencies from empty strings
	arrOptDependencies = arrOptDependencies.filter(String);
	// set the actual variable optDependencies with all values for R option "dependencies"
	if(arrOptDependencies.length > 0) {
		var optDependencies = ", dependencies=list(" + arrOptDependencies.join(", ") + ")";
	} else {
		var optDependencies = "";
	}

	// define the array arrOptPluginmap for values of R option "pluginmap"
	var arrOptPluginmap = new Array();
		if(inpNmnmnplg) {
			arrOptPluginmap.push("name=\"" + inpNmnmnplg + "\"");
		} else {
			arrOptPluginmap.push("name=\"" + inpPluginnm + "\"");
		}
		if(drpPlcntpmn) {
			arrOptPluginmap.push("hierarchy=\"" + drpPlcntpmn + "\"");
		} else {
			arrOptPluginmap.push();
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
		} else {
			arrOptSkeleton.push("\n\t#provides=c(\"logic\", \"dialog\")");
		}
		if(optPluginmap) {
			arrOptSkeleton.push("\n\t" + optPluginmap);
		} else {
			arrOptSkeleton.push("\n\t#pluginmap=list(name=\"\", hierarchy=\"\", require=\"\")");
		}
		if(chcIncldplg) {
			arrOptSkeleton.push("\n\ttests=TRUE");
		} else {
			arrOptSkeleton.push("\n\ttests=FALSE");
		}
		if(chcOpnflsfr) {
			arrOptSkeleton.push("\n\tedit=TRUE");
		} else {
			arrOptSkeleton.push("\n\tedit=FALSE");
		}
		if(chcAddplRKW) {
			arrOptSkeleton.push("\n\tload=TRUE");
		} else {
			arrOptSkeleton.push("\n\tload=FALSE");
		}
		if(chcShwthplg) {
			arrOptSkeleton.push("\n\tshow=TRUE");
		} else {
			arrOptSkeleton.push("\n\tshow=FALSE");
		}
	// clean array arrOptSkeleton from empty strings
	arrOptSkeleton = arrOptSkeleton.filter(String);
	// set the actual variable optSkeleton with all values for R option ""
	if(arrOptSkeleton.length > 0) {
		var optSkeleton = ", " + arrOptSkeleton.join(", ") + "";
	} else {
		var optSkeleton = "";
	}

	echo("about.plugin <- rk.XML.about(");
	if(inpPluginnm) {
		echo("\n\tname=\"" + inpPluginnm + "\"");
	}
	var ocolOclInpGvnnmtx = getList("ost_fPPPPGGFFE.ocl_inpGvnnmtx");
	var ocolOclInpFmlynmt = getList("ost_fPPPPGGFFE.ocl_inpFmlynmt");
	var ocolOclInpEmaltxt = getList("ost_fPPPPGGFFE.ocl_inpEmaltxt");
	var ocolOclChcAthrstt = getList("ost_fPPPPGGFFE.ocl_chcAthrstt");
	var ocolOclChcMntnrst = getList("ost_fPPPPGGFFE.ocl_chcMntnrst");
	var ocolOclChcCntrbtr = getList("ost_fPPPPGGFFE.ocl_chcCntrbtr");
	if(ocolOclInpGvnnmtx != "") {
		echo("\tauthor=c(\n\t\t\t");
	for (var i = 0; i < ocolOclInpGvnnmtx.length; ++i){
		// define the array arrOptAuthorRole for values of R option "role"
		var arrOptAuthorRole = new Array();
				if(ocolOclChcAthrstt[i] == 1) {
						arrOptAuthorRole.push("\"aut\"");
				} else {
						arrOptAuthorRole.push();
				}
				if(ocolOclChcMntnrst[i] == 1) {
						arrOptAuthorRole.push("\"cre\"");
				} else {
						arrOptAuthorRole.push();
				}
				if(ocolOclChcCntrbtr[i] == 1) {
						arrOptAuthorRole.push("\"ctb\"");
				} else {
						arrOptAuthorRole.push();
				}
		// clean array arrOptAuthorRole from empty strings
		arrOptAuthorRole = arrOptAuthorRole.filter(String);
		// set the actual variable optAuthorRole with all values for R option "role"
		if(arrOptAuthorRole.length > 0) {
				var optAuthorRole = ", role=c(" + arrOptAuthorRole.join(", ") + ")";
		} else {
				var optAuthorRole = "";
		}

		echo("person(");
		echo("given=\"" + ocolOclInpGvnnmtx[i] + "\"");
		if(ocolOclInpFmlynmt[i]) {
				echo(", family=\"" + ocolOclInpFmlynmt[i] + "\"");
		}
		if(ocolOclInpEmaltxt[i]) {
				echo(", email=\"" + ocolOclInpEmaltxt[i] + "\"");
		}
		if(optAuthorRole) {
				echo(optAuthorRole);
		}
		echo(")");
		if(i + 1 < ocolOclInpGvnnmtx.length) {
			echo(",\n\t\t\t");
		}
	}
	echo("\n\t\t),\n");
	}
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
		if(ocolOclInpPckgtxt!= "") {
			echo("\n\tpackage=list(\n\t\t");
	for (var i = 0; i < ocolOclInpPckgtxt.length; ++i){
		echo("c(");
		echo("name=\"" + ocolOclInpPckgtxt[i] + "\"");
		if(ocolOclInpmintext[i]) {
				echo(", min=\"" + ocolOclInpmintext[i] + "\"");
		}
		if(ocolOclInpmaxtext[i]) {
				echo(", max=\"" + ocolOclInpmaxtext[i] + "\"");
		}
		if(ocolOclInpRpstryt[i]) {
				echo(", repository=\"" + ocolOclInpRpstryt[i] + "\"");
		}
		echo(")");
		if(i + 1 < ocolOclInpPckgtxt.length) {
			echo(",\n\t\t");
		}
	}
	echo("\n\t)");
		}
		echo("\n)\n\n");
	}
	echo("# name of the main component, relevant for help page content\nrk.set.comp(\"");
	if(inpNmnmnplg) {
		echo(inpNmnmnplg + "\")\n\n");
	} else {
		echo(inpPluginnm + "\")\n\n");
	}
	echo("############\n## your plugin dialog and JavaScript should be put here\n############\n\n");
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
	} else {
		echo("\n\t#dependencies=plugin.dependencies,");
	}
	echo("\n\tpath=output.dir," + "\n\tguess.getter=guess.getter," + "\n\tscan=c(\"var\", \"saveobj\", \"settings\")," + "\n\txml=list(\n\t\t#dialog=,\n\t\t#wizard=,\n\t\t#logic=,\n\t\t#snippets=\n\t)," + "\n\tjs=list(\n\t\t#results.header=FALSE,\n\t\t#load.silencer=,\n\t\t#require=,\n\t\t#variables=," + "\n\t\t#globals=,\n\t\t#preprocess=,\n\t\t#calculate=,\n\t\t#printout=,\n\t\t#doPrintout=\n\t),");
	if(frmWrthlpflChecked) {
		echo("\n\trkh=list(\n\t\tsummary=plugin.summary,\n\t\tusage=plugin.usage#," + "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t)," + "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\", \"rkh\"),");
	} else {
		echo("\n\trkh=list(" + "\n\t\t#summary=,\n\t\t#usage=," + "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t)," + "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\"),");
	}
	echo("\n\toverwrite=overwrite,");
	echo("\n\t#components=list(),");
	echo(optSkeleton);
	echo("\n)\n\n");
}

function printout(){
	// printout the results




}

