## intro
# This should be the first line in each test suite file: Include the
# test framework, unless already included (multiple inclusion would not
# really do any harm either, though
if (!isClass ("RKTestSuite")) source ("test_framework.R")

## definition of the test suite
suite <- new ("RKTestSuite", id="distributions",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("R2HTML", "datasets", "stats"),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			data (rock)
		}
	## the tests
	), tests = list (
		new ("RKTest", id="shapiro_wilk_test", call=function () {
			rk.call.plugin ("rkward::shapiro_test", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="ad_test", call=function () {
			rk.call.plugin ("rkward::ad_test", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}, libraries=c("nortest")),
		new ("RKTest", id="cvm_test", call=function () {
			rk.call.plugin ("rkward::cvm_test", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}, libraries=c("nortest")),
		new ("RKTest", id="pearson_test", call=function () {
			rk.call.plugin ("rkward::pearson_test", adjust.string="adjust = TRUE", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")

			rk.call.plugin ("rkward::pearson_test", adjust.string="adjust = FALSE", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}, libraries=c("nortest")),
		new ("RKTest", id="sf_test", call=function () {
			rk.call.plugin ("rkward::sf_test", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}, libraries=c("nortest")),
		new ("RKTest", id="lillie_test", call=function () {
			rk.call.plugin ("rkward::lillie_test", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}, libraries=c("nortest")),
		new ("RKTest", id="jb_test", call=function () {
			rk.call.plugin ("rkward::jb_test", excludenas.state="1", length.state="1", x.available="rock[[\"shape\"]]\nrock[[\"perm\"]]", submit.mode="submit")
		}, libraries=c("tseries")),
		new ("RKTest", id="beta_probabilities", call=function () {
			rk.call.plugin ("rkward::beta_probabilities", logp.string="log.p = FALSE", ncp.real="0.00000000", q.text="0.95", shape1.real="1.00000000", shape2.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")

			rk.call.plugin ("rkward::beta_probabilities", logp.string="log.p = TRUE", ncp.real="0.02000000", q.text="0.96", shape1.real="1.01000000", shape2.real="1.01000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="beta_quantiles", call=function () {
			rk.call.plugin ("rkward::beta_quantiles", logp.string="log.p = FALSE", ncp.real="0.00000000", p.text="0.95", shape1.real="1.00000000", shape2.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")

			rk.call.plugin ("rkward::beta_quantiles", logp.string="log.p = TRUE", ncp.real="0.08000000", p.text="-1", shape1.real="1.04000000", shape2.real="1.03000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="beta_clt", call=function () {
			rk.call.plugin ("rkward::plot_beta_clt", a.real="2.00000000", b.real="2.00000000", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="1", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histfillcol.color.string="azure", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="1", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", submit.mode="submit")
		}),
		new ("RKTest", id="plot_beta_distribution", call=function () {
			rk.call.plugin ("rkward::plot_beta_distribution", a.real="2.00000000", b.real="2.00000000", function.string="p", log.state="1", lower.state="1", max.real="1.00000000", min.real="0.00000000", n.real="100.000000", ncp.real="0.00000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", submit.mode="submit")
		})
	), postCalls = list (	# like initCalls: run after all tests to clean up.
		function () {
			suppressWarnings (rm (list=c (), envir=globalenv()))
		}
	)
)

## always store the result in "results" and print it
results <- rktest.runRKTestSuite (suite)
print (results)
