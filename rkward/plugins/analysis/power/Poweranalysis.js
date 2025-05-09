/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// this code was generated using the rkwarddev package.
// perhaps don't make changes here, but in the rkwarddev script instead!

function preview() {
	preprocess(true);
	calculate(true);
	printout(true);
}

function preprocess(is_preview) {
	// add requirements etc. here
	echo("require(pwr)\n");
}

function calculate(is_preview) {
	// read in variables from dialog
	var drpPwrStat = getString("drp_pwr_stat");
	var spnNmbrfgrp = getString("spn_Nmbrfgrp");
	var drpPwrType = getString("drp_pwr_type");
	var drpPwrProptype = getString("drp_pwr_proptype");
	var drpPwrHypothesis = getString("drp_pwr_hypothesis");
	var radEffctEta = getString("rad_effct_eta");
	var radPwrParam = getString("rad_pwr_param");
	var spnPower = getString("spn_Power");
	var pwrSpinDf = getString("pwr_spin_df");
	var pwrSpinDfu = getString("pwr_spin_dfu");
	var pwrSpinDfv = getString("pwr_spin_dfv");
	var pwrSpinSample0 = getString("pwr_spin_sample0");
	var pwrSpinSample1 = getString("pwr_spin_sample1");
	var pwrSpinSample2 = getString("pwr_spin_sample2");
	var spnEffectsz = getString("spn_Effectsz");
	var spnSgnfcncl = getString("spn_Sgnfcncl");
	var svbSvrsltst = getString("svb_Svrsltst");

	// the R code to be evaluated
	echo("\tpwr.result <- try(\n\t\t");
	if (drpPwrStat == "pwr.t.test") {
		if (drpPwrType == "two.sample.diff") {
			echo("pwr.t2n.test(");
			if (radPwrParam != "Sample size") {
				echo("\n\t\t\tn1=" + pwrSpinSample1 + ",\n\t\t\tn2=" + pwrSpinSample2);
			} else {
				echo("\n\t\t\tn1=" + pwrSpinSample1 + ",");
			}
		} else {
			echo("pwr.t.test(");
			if (radPwrParam != "Sample size") {
				echo("\n\t\t\tn=" + pwrSpinSample0);
			}
		}
		if (radPwrParam != "Effect size") {
			if (radPwrParam != "Sample size") {
				echo(",");
			}
			echo("\n\t\t\td=" + spnEffectsz);
		}
	}
	if (drpPwrStat == "pwr.r.test") {
		echo("pwr.r.test(");
		if (radPwrParam != "Sample size") {
			echo("\n\t\t\tn=" + pwrSpinSample0);
		}
		if (radPwrParam != "Effect size") {
			if (radPwrParam != "Sample size") {
				echo(",");
			}
			echo("\n\t\t\tr=" + spnEffectsz);
		}
	}
	if (drpPwrStat == "pwr.anova.test") {
		echo("pwr.anova.test(");
		echo("\n\t\t\tk=" + spnNmbrfgrp);
		if (radPwrParam != "Sample size") {
			echo(",\n\t\t\tn=" + pwrSpinSample0);
		}
		if (radPwrParam != "Effect size") {
			if (radEffctEta == "f") {
				echo(",\n\t\t\tf=" + spnEffectsz);
			} else {
				echo(",\n\t\t\tf=sqrt(" + spnEffectsz + "/(1-" + spnEffectsz + "))");
				comment("calculate f from eta squared", "\t");
			}
		}
	}
	if (drpPwrStat == "pwr.f2.test") {
		echo("pwr.f2.test(");
		if (radPwrParam != "Parameter count") {
			echo("\n\t\t\tu=" + pwrSpinDfu);
		}
		if (radPwrParam != "Sample size") {
			if (radPwrParam != "Parameter count") {
				echo(",");
			}
			echo("\n\t\t\tv=" + pwrSpinDfv);
		}
		if (radPwrParam != "Effect size") {
			echo(",\n\t\t\tf2=" + spnEffectsz);
		}
	}
	if (drpPwrStat == "pwr.chisq.test") {
		echo("pwr.chisq.test(");
		if (radPwrParam != "Effect size") {
			echo("\n\t\t\tw=" + spnEffectsz);
		}
		if (radPwrParam != "Sample size") {
			if (radPwrParam != "Effect size") {
				echo(",");
			}
			echo("\n\t\t\tN=" + pwrSpinSample0);
		}
		echo(",\n\t\t\tdf=" + pwrSpinDf);
	}
	if (drpPwrStat == "pwr.p.test") {
		if (drpPwrProptype == "two.sample.same") {
			echo("pwr.2p.test(");
		}
		if (drpPwrProptype == "two.sample.diff") {
			echo("pwr.2p2n.test(");
		}
		if (drpPwrProptype == "one.sample") {
			echo("pwr.p.test(");
		}
		if (radPwrParam != "Effect size") {
			echo("\n\t\t\th=" + spnEffectsz);
		}
		if (radPwrParam != "Sample size") {
			if (radPwrParam != "Effect size") {
				echo(",");
			}
			if (drpPwrProptype != "two.sample.diff") {
				echo("\n\t\t\tn=" + pwrSpinSample0);
			} else {
				echo("\n\t\t\tn1=" + pwrSpinSample1 + ",\n\t\t\tn2=" + pwrSpinSample2);
			}
		} else {
			if (drpPwrProptype == "two.sample.diff") {
				echo(",\n\t\t\tn1=" + pwrSpinSample1);
			}
		}
	}
	if (drpPwrStat == "pwr.norm.test") {
		echo("pwr.norm.test(");
		if (radPwrParam != "Effect size") {
			echo("\n\t\t\td=" + spnEffectsz);
		}
		if (radPwrParam != "Sample size") {
			if (radPwrParam != "Effect size") {
				echo(",");
			}
			echo("\n\t\t\tn=" + pwrSpinSample0);
		}
	}
	if (radPwrParam != "Significance level") {
		if (spnSgnfcncl != 0.05) {
			echo(",\n\t\t\tsig.level=" + spnSgnfcncl);
		}
	} else {
		echo(",\n\t\t\tsig.level=NULL");
	}
	if (radPwrParam != "Power") {
		echo(",\n\t\t\tpower=" + spnPower);
	}
	if (drpPwrStat == "pwr.t.test" && drpPwrType != "two.sample.diff" && drpPwrType != "two.sample") {
		echo(",\n\t\t\ttype=\"" + drpPwrType + "\"");
	}
	if (drpPwrStat != "pwr.anova.test" && drpPwrStat != "pwr.f2.test" && drpPwrStat != "pwr.chisq.test") {
		if (drpPwrHypothesis != "two.sided") {
			echo(",\n\t\t\talternative=\"" + drpPwrHypothesis + "\"");
		}
	}
	echo("\n\t\t)\n\t)\n\n");
}

function printout(is_preview) {
	// read in variables from dialog
	var drpPwrStat = getString("drp_pwr_stat");
	var spnNmbrfgrp = getString("spn_Nmbrfgrp");
	var drpPwrType = getString("drp_pwr_type");
	var drpPwrProptype = getString("drp_pwr_proptype");
	var drpPwrHypothesis = getString("drp_pwr_hypothesis");
	var radEffctEta = getString("rad_effct_eta");
	var radPwrParam = getString("rad_pwr_param");
	var spnPower = getString("spn_Power");
	var pwrSpinDf = getString("pwr_spin_df");
	var pwrSpinDfu = getString("pwr_spin_dfu");
	var pwrSpinDfv = getString("pwr_spin_dfv");
	var pwrSpinSample0 = getString("pwr_spin_sample0");
	var pwrSpinSample1 = getString("pwr_spin_sample1");
	var pwrSpinSample2 = getString("pwr_spin_sample2");
	var spnEffectsz = getString("spn_Effectsz");
	var spnSgnfcncl = getString("spn_Sgnfcncl");
	var svbSvrsltst = getString("svb_Svrsltst");

	// printout the results

	var drpPwrStat = getValue("drp_pwr_stat");
	var radPwrParam = getValue("rad_pwr_param");
	comment("Catch errors due to unsuitable data", "\t");
	echo("\tif(class(pwr.result) == \"try-error\"){\n" +
	     "\t\trk.print(" + i18n("Power analysis not possible with the data you provided") + ")\n" +
	     "\t\treturn()\n\t}\n\n");
	comment("Prepare printout", "\t");
	echo("\tnote <- pwr.result[[\"note\"]]\n");
	if (!is_preview) {
		header = new Header().addFromUI("rad_pwr_param");
		echo("\tparameters <- list(");
		echo(header.extractParameters());
		echo(")\n" +
		     "\tif(!is.null(pwr.result[[\"alternative\"]])){\n\t\tparameters[[" + i18n("alternative") + "]] <- pwr.result[[\"alternative\"]]\n\t}\n\n" +
		     "\trk.header(pwr.result[[\"method\"]], parameters=parameters)\n");
	}
	echo("\tpwr.result[c(\"method\", \"note\", \"alternative\")] <- NULL\n" +
	     "\tpwr.result <- as.data.frame(unlist(pwr.result))\n" +
	     "\tcolnames(pwr.result) <- " + i18n("Parameters") + "\n\n" +
	     "\trk.results(pwr.result)\n" +
	     "\tif(!is.null(note)){\n\t\trk.print(paste(" + i18n("<strong>Note:</strong>") + ", note))\n\t}\n\n");
	if (drpPwrStat == "pwr.t.test" || drpPwrStat == "pwr.norm.test") {
		echo("\trk.print(" + i18nc("Argument is name of statistic, e.g. 'r'", "Interpretation of effect size <strong>%1</strong> (according to Cohen):", "d") + ")\n" +
		     "\trk.results(data.frame(" + i18nc("effect size", "small") + "=0.2, " + i18nc("effect size", "medium") + "=0.5, " + i18nc("effect size", "large") + "=0.8))\n");
	}
	if (drpPwrStat == "pwr.r.test") {
		echo("\trk.print(" + i18nc("Argument is name of statistic, e.g. 'r'", "Interpretation of effect size <strong>%1</strong> (according to Cohen):", "r") + ")\n" +
		     "\trk.results(data.frame(" + i18nc("effect size", "small") + "=0.1, " + i18nc("effect size", "medium") + "=0.3, " + i18nc("effect size", "large") + "=0.5))\n");
	}
	if (drpPwrStat == "pwr.f2.test") {
		echo("\trk.print(" + i18nc("Argument is name of statistic, e.g. 'r'", "Interpretation of effect size <strong>%1</strong> (according to Cohen):", "f<sup>2</sup>") + ")\n" +
		     "\trk.results(data.frame(" + i18nc("effect size", "small") + "=0.02, " + i18nc("effect size", "medium") + "=0.15, " + i18nc("effect size", "large") + "=0.35))\n");
	}
	if (drpPwrStat == "pwr.anova.test") {
		echo("\trk.print(" + i18nc("Argument is name of statistic, e.g. 'r'", "Interpretation of effect size <strong>%1</strong> (according to Cohen):", "f") + ")\n" +
		     "\trk.results(data.frame(" + i18nc("effect size", "small") + "=0.1, " + i18nc("effect size", "medium") + "=0.25, " + i18nc("effect size", "large") + "=0.4))\n");
	}
	if (drpPwrStat == "pwr.chisq.test") {
		echo("\trk.print(" + i18nc("Argument is name of statistic, e.g. 'r'", "Interpretation of effect size <strong>%1</strong> (according to Cohen):", "w") + ")\n" +
		     "\trk.results(data.frame(" + i18nc("effect size", "small") + "=0.1, " + i18nc("effect size", "medium") + "=0.3, " + i18nc("effect size", "large") + "=0.5))\n");
	}
	if (drpPwrStat == "pwr.p.test") {
		echo("\trk.print(" + i18nc("Argument is name of statistic, e.g. 'r'", "Interpretation of effect size <strong>%1</strong> (according to Cohen):", "h") + ")\n" +
		     "\trk.results(data.frame(" + i18nc("effect size", "small") + "=0.2, " + i18nc("effect size", "medium") + "=0.5, " + i18nc("effect size", "large") + "=0.8))\n");
	}
	if (!is_preview) {
		//// save result object
		// read in saveobject variables
		var svbSvrsltst = getValue("svb_Svrsltst");
		var svbSvrsltstActive = getValue("svb_Svrsltst.active");
		var svbSvrsltstParent = getValue("svb_Svrsltst.parent");
		// assign object to chosen environment
		if (svbSvrsltstActive) {
			echo(".GlobalEnv$" + svbSvrsltst + " <- pwr.result\n");
		}
	}
}
