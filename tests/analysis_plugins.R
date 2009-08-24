## intro
# This should be the first line in each test suite file: Include the
# test framework, unless already included (multiple inclusion would not
# really do any harm either, though
if (!isClass ("RKTestSuite")) source ("test_framework.R")

## definition of the test suite
suite <- new ("RKTestSuite", id="analysis_plugins",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("R2HTML", "datasets", "stats"),
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
		}
	## the tests
	), tests = list (
		new ("RKTest", id="basic_statistics_a", call=function () {
			rk.call.plugin ("rkward::basic_statistics", autre.real="6.000000", constMad.real="1.46280000", customMu.state="1", customS.state="1", huber.state="1", initmu.string="median", irq.state="1", length.state="1", mad.state="1", maximum.state="1", mean.state="1", median.state="1", minimum.state="1", mu.text="3", narm.state="1", nbmaximum.real="0.000000", nbminimum.real="0.000000", nom.selection="my.data", pourcent.real="0.05000000", quartile.state="1", result.state="1", s.text="", sd.state="1", tol.real="0.070000", trim.state="1", vari.state="1", winsor.real="1.50000000", z.available="women[[\"weight\"]]\ntest50x", submit.mode="submit")
		}, libraries=c("MASS")),
		new ("RKTest", id="basic_statistics_b", call=function () {
			rk.call.plugin ("rkward::basic_statistics", autre.real="0.000000", huber.state="", irq.state="0", length.state="0", mad.state="", maximum.state="0", mean.state="0", median.state="0", minimum.state="0", narm.state="1", nbmaximum.real="3.000000", nbminimum.real="2.000000", quartile.state="0", result.state="", sd.state="0", trim.state="", vari.state="0", z.available="test10x\nwomen[[\"height\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="correlation_matrix", call=function () {		
			rk.call.plugin ("rkward::corr_matrix", do_p.state="1", method.string="pearson", use.string="pairwise", x.available="test50x\ntest50y\ntest50z", submit.mode="submit")

			rk.call.plugin ("rkward::corr_matrix", do_p.state="", method.string="pearson", use.string="pairwise", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="correlation_matrix_plot", call=function () {
			rk.call.plugin ("rkward::cor_graph", digits.real="3.000000", method.string="pearson", scale.state="TRUE", use.string="pairwise.complete.obs", x.available="rock", submit.mode="submit")
		}),
		new ("RKTest", id="descriptive_stats", call=function () {
			rk.call.plugin ("rkward::descriptive", constMad.real="1.46280000", length.state="1", mad.state="1", mad_type.string="average", mean.state="1", median.state="1", prod.state="1", range.state="1", sd.state="1", sum.state="1", trim.real="0.00000000", x.available="women[[\"height\"]]\ntest10z", submit.mode="submit")
		}),
		new ("RKTest", id="t_test_two_vars", call=function () {
			rk.call.plugin ("rkward::t_test_two_vars", confint.state="1", conflevel.real="0.95000000", hypothesis.string="two.sided", paired.state="0", varequal.state="0", x.available="test50x", y.available="test50y", submit.mode="submit")

			rk.call.plugin ("rkward::t_test_two_vars", confint.state="1", conflevel.real="0.99000000", hypothesis.string="less", paired.state="1", x.available="test10y", y.available="test10z", submit.mode="submit")
		}),
		new ("RKTest", id="wilcoxon_test", call=function () {
			rk.call.plugin ("rkward::wilcoxon_test", alternative.string="two.sided", confint.state="TRUE", conflevel.real="0.95000000", correct.state="FALSE", exact.string="yes", mu.real="0.00000000", x.available="test50x", y.available="", submit.mode="submit")

			rk.call.plugin ("rkward::wilcoxon_test", alternative.string="less", confint.state="FALSE", correct.state="TRUE", exact.string="automatic", mu.real="0.00000000", paired.state="TRUE", x.available="test50x", y.available="test50y", submit.mode="submit")
		}),
		new ("RKTest", id="wilcoxon_exact_test", call=function () {
			rk.call.plugin ("rkward::wilcoxon_exact_test", alternative.string="two.sided", confint.state="TRUE", conflevel.real="0.95000000", correct.state="FALSE", exact.string="yes", mu.real="0.00000000", x.available="test50x", y.available="", submit.mode="submit")

			rk.call.plugin ("rkward::wilcoxon_exact_test", alternative.string="less", confint.state="FALSE", correct.state="TRUE", exact.string="automatic", mu.real="0.00000000", paired.state="TRUE", x.available="test50x", y.available="test50y", submit.mode="submit")
		}, libraries=c ("exactRankTests")),
		new ("RKTest", id="ansari_bradley_test", call=function () {
			rk.call.plugin ("rkward::ansari_bradley_test", alternative.string="two.sided", confint.state="TRUE", conflevel.real="0.95000000", exact.string="yes", x.available="test50x", y.available="test10y", submit.mode="submit")

			rk.call.plugin ("rkward::ansari_bradley_test", alternative.string="less", confint.state="FALSE", exact.string="automatic", x.available="test50x", y.available="test50y", submit.mode="submit")
		}),
		new ("RKTest", id="ansari_bradley_exact_test", call=function () {
			rk.call.plugin ("rkward::ansari_bradley_exact_test", alternative.string="two.sided", confint.state="TRUE", conflevel.real="0.95000000", exact.string="yes", x.available="test50x", y.available="test10y", submit.mode="submit")

			rk.call.plugin ("rkward::ansari_bradley_exact_test", alternative.string="less", confint.state="FALSE", exact.string="automatic", x.available="test50x", y.available="test50y", submit.mode="submit")
		}, libraries=c ("exactRankTests")),
		new ("RKTest", id="moments_moment", call=function () {
			rk.call.plugin ("rkward::moment", absolute.state="FALSE", central.state="FALSE", length.state="0", narm.state="TRUE", order.real="1.000000", x.available="test50z\ntest50y\ntest50x\ntest10z\ntest10y\ntest10x", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="bonett_test", call=function () {
			rk.call.plugin ("rkward::bonett_test", alternative.string="two.sided", length.state="1", show_alternative.state="1", x.available="test50z\ntest50y\ntest50x\ntest10z\ntest10y\ntest10x", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="agostino_test", call=function () {
			rk.call.plugin ("rkward::agostino_test", alternative.string="two.sided", length.state="1", show_alternative.state="1", x.available="warpbreaks[[\"breaks\"]]\ntest50z\ntest10x", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="anscombe_test", call=function () {
			rk.call.plugin ("rkward::anscombe_test", alternative.string="two.sided", length.state="1", show_alternative.state="1", x.available="warpbreaks[[\"breaks\"]]\ntest50z\ntest10y", submit.mode="submit")
		}, libraries=c ("moments")),
		new ("RKTest", id="skewness_kurtosis", call=function () {
			rk.call.plugin ("rkward::skewness_kurtosis", geary.state="1", kurtosis.state="1", length.state="1", narm.state="1", skewness.state="1", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]\nwarpbreaks[[\"breaks\"]]", submit.mode="submit")
		}, libraries=c ("moments"))
	), postCalls = list (	# like initCalls: run after all tests to clean up.
		function () {
			suppressWarnings (rm (list=c ("women", "warpbreaks", "rock", "test50x", "test50y", "test50z", "test10x", "test10y", "test10z"), envir=globalenv())) 
		}
	)
)

## always store the result in "results" and print it
results <- rktest.runRKTestSuite (suite)
print (results)
