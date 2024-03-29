# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

## definition of the test suite
suite <- new ("RKTestSuite", id="analysis_plugins",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("datasets", "stats"),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			# prepare some different files for loading
			library ("datasets")
			data (women)
			data (warpbreaks)
			data (rock)

			assign ("test50x", 100+c (1:50), envir=globalenv())
			assign ("test50y", 200+c (1:50), envir=globalenv())
			assign ("test50z", c (1:50)*4, envir=globalenv())
			assign ("test10x", 100+c (1:10, NA), envir=globalenv())
			assign ("test10y", 200+c (1:10, NA), envir=globalenv())
			assign ("test10z", c (1:10, NA)*4, envir=globalenv())
			assign ("test10a", c (1:5, 1:5, NA), envir=globalenv())
			x <- data.frame ("A" = rep (c (1, 2), 8), "B" = rep (c (1, 1, 2, 2), 4), "C" = rep (c (1, 1, 1, 1, 2, 2, 2, 2), 2), "D"= c (rep (1, 8), rep (2, 8)))
			x[2,2] <- NA
			assign ("test_table", x, envir=globalenv())
		}
	## the tests
	), tests = list (
		new ("RKTest", id="basic_statistics_a", call=function () {
			rk.call.plugin ("rkward::basic_statistics", autre.real="6.00", constMad.real="1.4628", customMu.state="1", customS.state="1", huber.state="1", initmu.string="median", irq.state="1", length.state="1", mad.state="1", maximum.state="1", mean.state="1", median.state="1", minimum.state="1", mu.text="3", narm.state="1", nbmaximum.real="0.00", nbminimum.real="0.00", saveas.active="1", saveas.objectname="my.data", pourcent.real="0.05", quartile.state="1", s.text="", sd.state="1", tol.real="0.07", trim.state="1", vari.state="1", winsor.real="1.50", z.available="women[[\"weight\"]]\ntest50x", submit.mode="submit")
		}, libraries=c("MASS")),
		new ("RKTest", id="basic_statistics_b", call=function () {
			rk.call.plugin ("rkward::basic_statistics", autre.real="0.00", huber.state="", irq.state="0", length.state="0", mad.state="", maximum.state="0", mean.state="0", median.state="0", minimum.state="0", narm.state="1", nbmaximum.real="3.00", nbminimum.real="2.00", quartile.state="0", sd.state="0", trim.state="", vari.state="0", z.available="test10x\nwomen[[\"height\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="correlation_matrix", call=function () {		
			rk.call.plugin ("rkward::corr_matrix", do_p.state="1", method.string="pearson", use.string="pairwise", x.available="test50x\ntest50y\ntest50z", submit.mode="submit")

			rk.call.plugin ("rkward::corr_matrix", do_p.state="", method.string="polychoric", use.string="pairwise", x.available="test10y\ntest10a", submit.mode="submit")

			rk.call.plugin ("rkward::corr_matrix", do_p.state="", method.string="pearson", use.string="pairwise", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]", submit.mode="submit")
		}, libraries=c("polycor")),
		new ("RKTest", id="correlation_matrix_plot", call=function () {
			rk.call.plugin ("rkward::cor_graph", digits.real="3.00", method.string="pearson", scale.state="TRUE", use.string="pairwise.complete.obs", x.available="rock", submit.mode="submit")
		}),
		new ("RKTest", id="descriptive_stats", call=function () {
			rk.call.plugin ("rkward::descriptive", constMad.real="1.4628", length.state="1", mad.state="1", mad_type.string="average", mean.state="1", median.state="1", prod.state="1", range.state="1", sd.state="1", sum.state="1", trim.real="0.00", x.available="women[[\"height\"]]\ntest10z", submit.mode="submit")
		}),
		new ("RKTest", id="t_test", call=function () {
			rk.call.plugin ("rkward::t_test", confint.checked="1", conflevel.real="0.95", hypothesis.string="two.sided", test_form.string="indep", varequal.state="0", x.available="test50x", y.available="test50y", submit.mode="submit")

			rk.call.plugin ("rkward::t_test", confint.checked="1", conflevel.real="0.99", hypothesis.string="less", test_form.string="paired", x.available="test10y", y.available="test10z", submit.mode="submit")

			rk.call.plugin ("rkward::t_test", confint.checked="1", conflevel.real="0.95", hypothesis.string="two.sided", mu.real="20.00", test_form.string="const", x.available="test10z", submit.mode="submit")
		}),
		new ("RKTest", id="wilcoxon_tests", call=function () {
			rk.call.plugin ("rkward::wilcoxon_tests", alternative.string="two.sided", confint.checked="1", conflevel.real="0.95", correct.state="", exact.string="TRUE", mu.real="0.00", svb_Svrsltst.active="0", svb_Svrsltst.objectname="wcox.result", svb_Svrsltst.parent=".GlobalEnv", ties.state="", x.available="test50x", y.available="", submit.mode="submit")
			rk.call.plugin ("rkward::wilcoxon_tests", alternative.string="less", confint.checked="0", correct.state="correct", exact.string="automatic", mu.real="0.00", paired.state="true", svb_Svrsltst.active="0", svb_Svrsltst.objectname="wcox.result", svb_Svrsltst.parent=".GlobalEnv", ties.state="", x.available="test50x", y.available="test50y", submit.mode="submit")
			# exact tests, allowing ties
			rk.call.plugin ("rkward::wilcoxon_tests", alternative.string="two.sided", confint.checked="1", conflevel.real="0.95", correct.state="", exact.string="TRUE", mu.real="0.00", svb_Svrsltst.active="0", svb_Svrsltst.objectname="wcox.result", svb_Svrsltst.parent=".GlobalEnv", ties.state="true", x.available="test50x", y.available="", submit.mode="submit")
			rk.call.plugin ("rkward::wilcoxon_tests", alternative.string="less", confint.checked="0", correct.state="correct", exact.string="automatic", mu.real="0.00", paired.state="true", svb_Svrsltst.active="0", svb_Svrsltst.objectname="wcox.result", svb_Svrsltst.parent=".GlobalEnv", ties.state="true", x.available="test50x", y.available="test50y", submit.mode="submit")
		}, libraries=c ("exactRankTests")),
		new ("RKTest", id="moments_moment", call=function () {
			rk.call.plugin ("rkward::moment", absolute.state="FALSE", central.state="FALSE", length.state="0", narm.state="TRUE", order.real="1.00", x.available="test50z\ntest50y\ntest50x\ntest10z\ntest10y\ntest10x", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="bonett_test", call=function () {
			rk.call.plugin ("rkward::bonett_test", alternative.string="two.sided", length.state="1", show_alternative.state="1", x.available="test50z\ntest50y\ntest50x\ntest10z\ntest10y\ntest10x", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="agostino_test", call=function () {
			rk.call.plugin ("rkward::agostino_test", alternative.string="two.sided", length.state="1", show_alternative.state="1", x.available="warpbreaks[[\"breaks\"]]\ntest50z\ntest10x", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="anscombe_test", call=function () {
			rk.call.plugin ("rkward::anscombe_test", alternative.string="two.sided", length.state="1", show_alternative.state="1", x.available="warpbreaks[[\"breaks\"]]\ntest50z\ntest10y", submit.mode="submit")
		}, fuzzy_output=TRUE, libraries=c ("moments")),
		new ("RKTest", id="skewness_kurtosis", call=function () {
			rk.call.plugin ("rkward::skewness_kurtosis", geary.state="1", kurtosis.state="1", length.state="1", narm.state="1", skewness.state="1", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]\nwarpbreaks[[\"breaks\"]]", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="F_test", call=function () {
			rk.call.plugin ("rkward::F_test", alternative.string="two.sided", conflevel.real="0.95", ratio.real="1.00", x.available="test50z", y.available="test50y", submit.mode="submit")
		}),
		new ("RKTest", id="fligner_test", call=function () {
			rk.call.plugin ("rkward::fligner_test", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]\ntest50z\ntest50y\ntest50x\ntest10z\ntest10y\ntest10x", submit.mode="submit")
		}),
		new ("RKTest", id="bartlett_test", call=function () {
			rk.call.plugin ("rkward::bartlett_test", x.available="warpbreaks[[\"breaks\"]]\ntest50z\ntest50y\ntest50x\ntest10z\ntest10y\ntest10x", submit.mode="submit")
		}),
		new ("RKTest", id="levene_test", call=function () {
			rk.call.plugin ("rkward::levene_test", group.available="warpbreaks[[\"tension\"]]", y.available="warpbreaks[[\"breaks\"]]", submit.mode="submit")
		}, libraries = c ("car")),
		new ("RKTest", id="ansari_bradley_test", call=function () {
			rk.call.plugin ("rkward::ansari_bradley_test", alternative.string="two.sided", confint.state="TRUE", conflevel.real="0.95", exact.string="yes", x.available="test50x", y.available="test10y", submit.mode="submit")

			rk.call.plugin ("rkward::ansari_bradley_test", alternative.string="less", confint.state="FALSE", exact.string="automatic", x.available="test50x", y.available="test50y", submit.mode="submit")
		}),
		new ("RKTest", id="ansari_bradley_exact_test", call=function () {
			rk.call.plugin ("rkward::ansari_bradley_exact_test", alternative.string="two.sided", confint.state="TRUE", conflevel.real="0.95", exact.string="yes", x.available="test50x", y.available="test10y", submit.mode="submit")

			rk.call.plugin ("rkward::ansari_bradley_exact_test", alternative.string="less", confint.state="FALSE", exact.string="automatic", x.available="test50x", y.available="test50y", submit.mode="submit")
		}, libraries=c ("exactRankTests")),
		new ("RKTest", id="mood_test", call=function () {
			rk.call.plugin ("rkward::mood_test", alternative.string="two.sided", x.available="test50z", y.available="test50x", submit.mode="submit")
		}),
		new ("RKTest", id="chisq_out_test", call=function () {
			rk.call.plugin ("rkward::chisq_out_test", descriptives.state="1", length.state="1", opposite.state="FALSE", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]\nrock[[\"peri\"]]\nrock[[\"area\"]]", submit.mode="submit")

			rk.call.plugin ("rkward::chisq_out_test", descriptives.state="0", length.state="0", opposite.state="TRUE", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]\nrock[[\"peri\"]]\nrock[[\"area\"]]", submit.mode="submit")
		}, libraries = c ("outliers")),
		new ("RKTest", id="dixon_test", call=function () {
			rk.call.plugin ("rkward::dixon_test", descriptives.state="1", length.state="1", opposite.state="FALSE", two_sided.state="TRUE", type.string="0", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]", submit.mode="submit")

			rk.call.plugin ("rkward::dixon_test", descriptives.state="0", length.state="0", opposite.state="TRUE", two_sided.state="FALSE", type.string="0", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]", submit.mode="submit")
		}, libraries = c ("outliers")),
		new ("RKTest", id="outlier", call=function () {
			rk.call.plugin ("rkward::outlier", descriptives.state="0", length.state="1", opposite.state="FALSE", x.available="warpbreaks[[\"breaks\"]]\ntest50z", submit.mode="submit")

			rk.call.plugin ("rkward::outlier", descriptives.state="1", length.state="0", opposite.state="TRUE", x.available="warpbreaks[[\"breaks\"]]\ntest50z", submit.mode="submit")
		}, libraries = c ("outliers")),
		new ("RKTest", id="grubbs_test", call=function () {
			rk.call.plugin ("rkward::grubbs_test", descriptives.state="0", length.state="1", opposite.state="FALSE", two_sided.state="TRUE", type.string="10", x.available="warpbreaks[[\"breaks\"]]\ntest10z", submit.mode="submit")

			rk.call.plugin ("rkward::grubbs_test", descriptives.state="1", length.state="1", opposite.state="TRUE", two_sided.state="FALSE", type.string="11", x.available="warpbreaks[[\"breaks\"]]\ntest10z", submit.mode="submit")
		}, libraries = c ("outliers")),
		new ("RKTest", id="pp_test", call=function () {
			rk.call.plugin ("rkward::PP_test", length.state="1", lshort.string="FALSE", narm.state="0", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]\nrock[[\"peri\"]]\nrock[[\"area\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="crosstab_n_to_1", call=function () {
			rk.call.plugin ("rkward::crosstab", barplot.state="TRUE", barplot_embed.colors.string="default", barplot_embed.labels.state="0", barplot_embed.legend.state="0", barplot_embed.plotoptions.add_grid.state="0", barplot_embed.plotoptions.asp.real="0.00", barplot_embed.plotoptions.main.text="", barplot_embed.plotoptions.pointcolor.color.string="", barplot_embed.plotoptions.pointtype.string="", barplot_embed.plotoptions.sub.text="", barplot_embed.plotoptions.xaxt.state="", barplot_embed.plotoptions.xlab.text="", barplot_embed.plotoptions.xlog.state="", barplot_embed.plotoptions.xmaxvalue.text="", barplot_embed.plotoptions.xminvalue.text="", barplot_embed.plotoptions.yaxt.state="", barplot_embed.plotoptions.ylab.text="", barplot_embed.plotoptions.ylog.state="", barplot_embed.plotoptions.ymaxvalue.text="", barplot_embed.plotoptions.yminvalue.text="", barplot_embed.type.string="juxtaposed", chisq.state="TRUE", simpv.string="FALSE", x.available="warpbreaks[[\"tension\"]]", y.available="warpbreaks[[\"wool\"]]\nwarpbreaks[[\"tension\"]]", submit.mode="submit")

			rk.call.plugin ("rkward::crosstab", barplot.state="TRUE", barplot_embed.colors.string="rainbow", barplot_embed.labels.state="1", barplot_embed.legend.state="0", barplot_embed.place.string="3", barplot_embed.plotoptions.add_grid.state="0", barplot_embed.plotoptions.asp.real="0.00", barplot_embed.plotoptions.main.text="", barplot_embed.plotoptions.pointcolor.color.string="", barplot_embed.plotoptions.pointtype.string="", barplot_embed.plotoptions.sub.text="", barplot_embed.plotoptions.xaxt.state="", barplot_embed.plotoptions.xlab.text="", barplot_embed.plotoptions.xlog.state="", barplot_embed.plotoptions.xmaxvalue.text="", barplot_embed.plotoptions.xminvalue.text="", barplot_embed.plotoptions.yaxt.state="", barplot_embed.plotoptions.ylab.text="", barplot_embed.plotoptions.ylog.state="", barplot_embed.plotoptions.ymaxvalue.text="", barplot_embed.plotoptions.yminvalue.text="", barplot_embed.type.string="juxtaposed", chisq.state="TRUE", chisq_expected.state="TRUE", margins.state="TRUE", prop_column.state="TRUE", prop_row.state="TRUE", prop_total.state="TRUE", simpv.string="FALSE", x.available="warpbreaks[[\"tension\"]]", y.available="warpbreaks[[\"wool\"]]\nwarpbreaks[[\"tension\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="crosstab_multi", call=function () {
			rk.call.plugin ("rkward::crosstab_multi", exclude_nas.state="1", x.available="test_table[[\"A\"]]\ntest_table[[\"B\"]]\ntest_table[[\"C\"]]\ntest_table[[\"D\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="box_test", call=function () {
			rk.call.plugin ("rkward::Box_test", lag.real="1.00", length.state="1", narm.state="0", type.string="Box-Pierce", x.available="test50x\ntest10y", submit.mode="submit")
		}),
		new ("RKTest", id="kpss_test", call=function () {
			rk.call.plugin ("rkward::kpss_test", length.state="1", lshort.string="FALSE", narm.state="0", null.string="Trend", x.available="test50x\ntest50y\ntest50z", submit.mode="submit")
		}, libraries=c("tseries")),
		new ("RKTest", id="hp_filter", call=function () {
			.GlobalEnv$co2 <- datasets::co2		# another, incompatible co2 dataset exists in package locfit
			rk.sync.global()

			rk.call.plugin ("rkward::hp_filter", cycle_name.active="1", trend_name.active="1", custom.state="0", cycle_col.color.string="green4", cycle_lty.string="", cycle_lwd.real="1.00", cycle_name.objectname="hpcycle", downlab.text="", lambda.string="1600", plot_cycle.state="1", series_col.color.string="blue", series_lty.string="", series_lwd.real="1.00", trend_col.color.string="red", trend_lty.string="", trend_lwd.real="1.00", trend_name.objectname="hptrend", uplab.text="", x.available="co2", submit.mode="submit")

			rk.print (summary (hptrend))
			rk.print (summary (hpcycle))
		}),
		new ("RKTest", id="linear_regression", call=function () {
			rk.call.plugin ("rkward::linear_regression", intercept.state="1", x.available="warpbreaks[[\"tension\"]]\nwarpbreaks[[\"wool\"]]", y.available="warpbreaks[[\"breaks\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="power_analysis", call=function () {
			rk.call.plugin ("rkward::power_analysis", drp_pwr_hypothesis.string="two.sided", drp_pwr_stat.string="pwr.t.test", drp_pwr_type.string="two.sample", pwr_spin_sample0.real="30.00", rad_pwr_param.string="Power", spn_Effectsz.real="0.30", spn_Sgnfcncl.real="0.05", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
			rk.call.plugin ("rkward::power_analysis", drp_pwr_hypothesis.string="two.sided", drp_pwr_stat.string="pwr.t.test", drp_pwr_type.string="two.sample.diff", pwr_spin_sample1.real="27.00", pwr_spin_sample2.real="33.00", rad_pwr_param.string="Power", spn_Effectsz.real="0.30", spn_Sgnfcncl.real="0.05", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
			rk.call.plugin ("rkward::power_analysis", drp_pwr_hypothesis.string="two.sided", drp_pwr_stat.string="pwr.r.test", rad_pwr_param.string="Sample size", spn_Effectsz.real="0.30", spn_Power.real="0.81", spn_Sgnfcncl.real="0.05", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
			rk.call.plugin ("rkward::power_analysis", drp_pwr_stat.string="pwr.chisq.test", pwr_spin_df.real="32.00", pwr_spin_sample0.real="30.00", rad_pwr_param.string="Significance level", spn_Effectsz.real="0.30", spn_Power.real="0.81", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
			rk.call.plugin ("rkward::power_analysis", drp_pwr_hypothesis.string="greater", drp_pwr_proptype.string="two.sample.same", drp_pwr_stat.string="pwr.p.test", pwr_spin_sample0.real="30.00", rad_pwr_param.string="Significance level", spn_Effectsz.real="0.30", spn_Power.real="0.81", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
			rk.call.plugin ("rkward::power_analysis", drp_pwr_hypothesis.string="two.sided", drp_pwr_stat.string="pwr.norm.test", pwr_spin_sample0.real="30.00", rad_pwr_param.string="Significance level", spn_Effectsz.real="0.30", spn_Power.real="0.80", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
			rk.call.plugin ("rkward::power_analysis", drp_pwr_stat.string="pwr.f2.test", pwr_spin_dfv.real="30.00", pwr_spin_sample0.real="30.00", rad_pwr_param.string="Parameter count", spn_Effectsz.real="0.30", spn_Power.real="0.80", spn_Sgnfcncl.real="0.10", svb_Svrsltst.active="0", svb_Svrsltst.objectname="pwr.result", svb_Svrsltst.parent=".GlobalEnv", submit.mode="submit")
		}, libraries=c("pwr"))
	), postCalls = list (	# like initCalls: run after all tests to clean up.
		function () {
			suppressWarnings (rm (list=c ("women", "warpbreaks", "rock", "co2", "test50x", "test50y", "test50z", "test10x", "test10y", "test10z", "test_table", "hptrend", "hpcycle"), envir=globalenv()))
		}
	)
)
