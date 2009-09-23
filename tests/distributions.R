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
		}),
		new ("RKTest", id="cauchy_probabilities", call=function () {
			rk.call.plugin ("rkward::cauchy_probabilities", location.real="0.03000000", logp.string="log.p = FALSE", q.text="0.95", scale.real="1.02000000", tail.string="lower.tail=TRUE", submit.mode="submit")

			rk.call.plugin ("rkward::cauchy_probabilities", location.real="-0.02000000", logp.string="log.p = TRUE", q.text="0.95", scale.real="1.03000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="cauchy_quantiles", call=function () {
			rk.call.plugin ("rkward::cauchy_quantiles", location.real="-0.03000000", logp.string="log.p = FALSE", p.text="0.95", scale.real="1.03000000", tail.string="lower.tail=TRUE", submit.mode="submit")

			rk.call.plugin ("rkward::cauchy_quantiles", location.real="0.02000000", logp.string="log.p = TRUE", p.text="-1 -2", scale.real="0.98000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="plot_cauchy_distribution", call=function () {
			rk.call.plugin ("rkward::plot_cauchy_distribution", function.string="d", loc.real="0.00000000", log.state="0", max.real="3.29000000", min.real="-3.29000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scale.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="chi_squared_probabilities", call=function () {
			rk.call.plugin ("rkward::chi_squared_probabilities", df.real="3.00000000", logp.string="log.p = FALSE", ncp.real="0.05000000", q.text="0.97 0.65", tail.string="lower.tail=TRUE", submit.mode="submit")

			rk.call.plugin ("rkward::chi_squared_probabilities", df.real="1.01000000", logp.string="log.p = TRUE", ncp.real="0.02000000", q.text="1", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="chi_squared_quantiles", call=function () {
			rk.call.plugin ("rkward::chi_squared_quantiles", df.real="1.00000000", logp.string="log.p = FALSE", ncp.real="0.00000000", p.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")

			rk.call.plugin ("rkward::chi_squared_quantiles", df.real="1.02000000", logp.string="log.p = TRUE", ncp.real="3.00000000", p.text="-1 -2", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="chi_squared_clt", call=function () {
			rk.call.plugin ("rkward::plot_chi_squared_clt", df.real="4.00000000", dist_stepfun.addtoplot.state="", dist_stepfun.col_hor.color.string="", dist_stepfun.col_vert.color.string="", dist_stepfun.col_y0.color.string="", dist_stepfun.col_y1.color.string="", dist_stepfun.do_points.state="", dist_stepfun.linetype.string="", dist_stepfun.verticals.state="1", drawnorm.state="1", function.string="dist", nAvg.real="12.000000", nDist.real="1000.000000", ncp.real="0.60000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="1", submit.mode="submit")
		}),
		new ("RKTest", id="plot_chi_squared_distribution", call=function () {
			rk.call.plugin ("rkward::plot_chi_squared_distribution", df.real="4.00000000", function.string="d", log.state="0", max.real="24.10000000", min.real="0.30000000", n.real="100.000000", ncp.real="0.00000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", submit.mode="submit")
		}),
		new ("RKTest", id="exponential_probabilities", call=function () {
			rk.call.plugin ("rkward::exponential_probabilities", logp.string="log.p = FALSE", q.text="0.96", rate.real="1.07000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="exponential_quantiles", call=function () {
			rk.call.plugin ("rkward::exponential_quantiles", logp.string="log.p = TRUE", p.text="-1.1", rate.real="1.05000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="exponential_clt", call=function () {
			rk.call.plugin ("rkward::plot_exponential_clt", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="grey4", normpointtype.string="h", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", rate.real="1.00000000", scalenorm.state="1", submit.mode="submit")
		}),
		new ("RKTest", id="plot_exponential_distribution", call=function () {
			rk.call.plugin ("rkward::plot_exponential_distribution", function.string="p", log.state="0", lower.state="0", max.real="10.00000000", min.real="0.00000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", rate.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="f_probabilities", call=function () {
			rk.call.plugin ("rkward::f_probabilities", df1.real="1.02000000", df2.real="1.11000000", logp.string="log.p = FALSE", ncp.real="0.02000000", q.text=".1, .2", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="f_quantiles", call=function () {
			rk.call.plugin ("rkward::f_quantiles", df1.real="1.00000000", df2.real="1.00000000", logp.string="log.p = FALSE", ncp.real="0.00000000", p.text="0.95", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="f_clt", call=function () {
			rk.call.plugin ("rkward::plot_f_clt", df1.real="5.00000000", df2.real="5.00000000", drawnorm.state="0", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", nAvg.real="10.000000", nDist.real="1000.000000", ncp.real="0.00000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", submit.mode="submit")
		}),
		new ("RKTest", id="plot_f_distribution", call=function () {
			rk.call.plugin ("rkward::plot_f_distribution", df1.real="5.00000000", df2.real="5.00000000", function.string="d", log.state="1", max.real="25.00000000", min.real="0.00100000", n.real="100.000000", ncp.real="0.00000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", submit.mode="submit")
		}),
		new ("RKTest", id="gamma_probabilities", call=function () {
			rk.call.plugin ("rkward::gamma_probabilities", logp.string="log.p = FALSE", q.text="0.95", rate.real="1.00000000", shape.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="gamma_quantiles", call=function () {
			rk.call.plugin ("rkward::gamma_quantiles", logp.string="log.p = TRUE", p.text="-5", rate.real="1.00000000", shape.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="gamma_clt", call=function () {
			rk.call.plugin ("rkward::plot_gamma_clt", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", rate.real="0.30000000", scalenorm.state="0", shape.real="5.90000000", submit.mode="submit")
		}),
		new ("RKTest", id="plot_gamma_distribution", call=function () {
			rk.call.plugin ("rkward::plot_gamma_distribution", function.string="d", log.state="0", max.real="4.60000000", min.real="0.01000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", rate.real="0.87000000", shape.real="1.61000000", submit.mode="submit")
		}),
		new ("RKTest", id="gumbel_probabilities", call=function () {
			rk.call.plugin ("rkward::gumbel_probabilities", logp.string="log.p = FALSE", q.text="0.95", scale.real="1.04000000", shape.real="1.03000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="gumbel_quantiles", call=function () {
			rk.call.plugin ("rkward::gumbel_quantiles", logp.string="log.p = FALSE", p.text="0.95", scale.real="1.00000000", shape.real="1.00000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="logistic_probabilities", call=function () {
			rk.call.plugin ("rkward::logistic_probabilities", location.real="1.04000000", logp.string="log.p = TRUE", q.text="0.95", scale.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="logistic_quantiles", call=function () {
			rk.call.plugin ("rkward::logistic_quantiles", location.real="1.00000000", logp.string="log.p = FALSE", p.text="0.95", scale.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="logistic_clt", call=function () {
			rk.call.plugin ("rkward::plot_logistic_clt", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="1", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="int", histogram_opt.histlinetype.string="solid", histogram_opt.include_lowest.state="1", histogram_opt.rightclosed.state="", histogram_opt.usefillcol.state="", loc.real="0.00000000", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scale.real="3.00000000", scalenorm.state="0", submit.mode="submit")
		}),
		new ("RKTest", id="plot_logistic_distribution", call=function () {
			rk.call.plugin ("rkward::plot_logistic_distribution", function.string="d", loc.real="0.00000000", log.state="0", max.real="3.29000000", min.real="-3.29000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scale.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="log_normal_probabilities", call=function () {
			rk.call.plugin ("rkward::log_normal_probabilities", logp.string="log.p = FALSE", meanlog.real="0.00000000", q.text="0.95", sdlog.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="log_normal_quantiles", call=function () {
			rk.call.plugin ("rkward::log_normal_quantiles", logp.string="log.p = FALSE", meanlog.real="0.00000000", p.text="0.95", sdlog.real="1.00000000", tail.string="lower.tail=FALSE", submit.mode="submit")
		}),
		new ("RKTest", id="log_normal_clt", call=function () {
			rk.call.plugin ("rkward::plot_log_normal_clt", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.angle.real="45.00000000", histogram_opt.barlabels.state="", histogram_opt.density.real="7.000000", histogram_opt.doborder.state="", histogram_opt.freq.state="0", histogram_opt.histbreaksFunction.string="Scott", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", mean.real="0.00000000", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", sd.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="plot_log_normal_distribution", call=function () {
			rk.call.plugin ("rkward::plot_lognormal_distribution", function.string="p", log.state="0", lower.state="1", max.real="3.29000000", mean.real="4.00000000", min.real="0.01000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", sd.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="normal_probabilities", call=function () {
			rk.call.plugin ("rkward::normal_probabilities", logp.string="log.p = FALSE", mean.real="0.00000000", q.text="0.95", sd.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="normal_quantiles", call=function () {
			rk.call.plugin ("rkward::normal_quantiles", logp.string="log.p = FALSE", mean.real="0.00000000", p.text="0.95", sd.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="normal_clt", call=function () {
			rk.call.plugin ("rkward::plot_normal_clt", dist_stepfun.addtoplot.state="", dist_stepfun.col_hor.color.string="blue", dist_stepfun.col_vert.color.string="blue", dist_stepfun.col_y0.color.string="gold", dist_stepfun.col_y1.color.string="cyan", dist_stepfun.do_points.state="", dist_stepfun.linetype.string="", dist_stepfun.verticals.state="1", drawnorm.state="1", function.string="dist", mean.real="0.00000000", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", sd.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="plot_normal_distribution", call=function () {
			rk.call.plugin ("rkward::plot_normal_distribution", function.string="d", log.state="0", max.real="3.29000000", mean.real="0.00000000", min.real="-3.29000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", sd.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="t_probabilities", call=function () {
			rk.call.plugin ("rkward::t_probabilities", df.real="1.00000000", logp.string="log.p = FALSE", ncp.real="0.00000000", q.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="t_quantiles", call=function () {
			rk.call.plugin ("rkward::t_quantiles", df.real="1.00000000", logp.string="log.p = FALSE", ncp.real="0.00000000", p.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="t_clt", call=function () {
			rk.call.plugin ("rkward::plot_t_clt", df.real="3.00000000", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", nAvg.real="10.000000", nDist.real="1000.000000", ncp.real="0.00000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", submit.mode="submit")
		}),
		new ("RKTest", id="plot_t_distribution", call=function () {
			rk.call.plugin ("rkward::plot_t_distribution", df.real="1.00000000", function.string="p", log.state="0", lower.state="1", max.real="12.92400000", min.real="-12.92400000", n.real="100.000000", ncp.real="0.00000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", submit.mode="submit")
		}),
		new ("RKTest", id="tukey_probabilities", call=function () {
			rk.call.plugin ("rkward::tukey_probabilities", df.text="5", logp.string="log.p = FALSE", nmeans.real="2.000000", nranges.real="1.000000", q.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="tukey_quantiles", call=function () {
			rk.call.plugin ("rkward::tukey_quantiles", df.text="2:11", logp.string="log.p = FALSE", nmeans.real="2.000000", nranges.real="1.000000", p.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="plot_tukey_distribution", call=function () {
			rk.call.plugin ("rkward::plot_tukey_distribution", df.real="5.00000000", log.state="0", lower.state="1", max.real="8.00000000", min.real="-1.00000000", n.real="101.000000", nmeans.real="6.000000", nranges.real="1.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", submit.mode="submit")
		}),
		new ("RKTest", id="uniform_probabilities", call=function () {
			rk.call.plugin ("rkward::uniform_probabilities", logp.string="log.p = FALSE", max.real="1.00000000", min.real="0.00000000", q.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="uniform_quantiles", call=function () {
			rk.call.plugin ("rkward::uniform_quantiles", logp.string="log.p = FALSE", max.real="1.00000000", min.real="0.00000000", p.text="0.95", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="uniform_clt", call=function () {
			rk.call.plugin ("rkward::plot_uniform_clt", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", llim.real="0.00000000", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", ulim.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="plot_uniform_distribution", call=function () {
			rk.call.plugin ("rkward::plot_uniform_distribution", function.string="d", llim.real="0.00000000", log.state="0", max.real="2.00000000", min.real="-1.00000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", ulim.real="1.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="weibull_probabilities", call=function () {
			rk.call.plugin ("rkward::weibull_probabilities", logp.string="log.p = FALSE", q.text="0.95", scale.real="1.00000000", shape.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="weibull_quantiles", call=function () {
			rk.call.plugin ("rkward::weibull_quantiles", logp.string="log.p = FALSE", p.text="0.95", scale.real="1.00000000", shape.real="1.00000000", tail.string="lower.tail=TRUE", submit.mode="submit")
		}),
		new ("RKTest", id="weibull_clt", call=function () {
			rk.call.plugin ("rkward::plot_weibull_clt", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", nAvg.real="10.000000", nDist.real="1000.000000", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scale.real="1.00000000", scalenorm.state="0", shape.real="2.00000000", submit.mode="submit")
		}),
		new ("RKTest", id="plot_weibull_distribution", call=function () {
			rk.call.plugin ("rkward::plot_weibull_distribution", function.string="d", log.state="0", max.real="5.00000000", min.real="0.00000000", n.real="100.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scale.real="1.00000000", shape.real="2.00000000", submit.mode="submit")
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
