## intro
# This should be the first line in each test suite file: Include the
# test framework, unless already included (multiple inclusion would not
# really do any harm either, though
if (!isClass ("RKTestSuite")) source ("test_framework.R")

## definition of the test suite
suite <- new ("RKTestSuite", id="plots",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("R2HTML", "datasets", "graphics"),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			# prepare some different files for loading
			library ("datasets")
			data (women)
			data (swiss)
			data (warpbreaks)

			x <- data.frame ("A" = rep (c (1, 2), 8), "B" = rep (c (1, 1, 2, 2), 4), "C" = rep (c (1, 1, 1, 1, 2, 2, 2, 2), 2), "D"= c (rep (1, 8), rep (2, 8)))
			x[2,2] <- NA
			assign ("test_table", x, envir=globalenv())
		}
	## the tests
	), tests = list (
		new ("RKTest", id="barplot", call=function () {
			rk.call.plugin ("rkward::barplot", barplot_embed.colors.string="rainbow", barplot_embed.labels.state="1", barplot_embed.legend.state="0", barplot_embed.place.string="3", barplot_embed.plotoptions.add_grid.state="0", barplot_embed.plotoptions.asp.real="0.00000000", barplot_embed.plotoptions.main.text="", barplot_embed.plotoptions.pointcolor.color.string="", barplot_embed.plotoptions.pointtype.string="", barplot_embed.plotoptions.sub.text="", barplot_embed.plotoptions.xaxt.state="", barplot_embed.plotoptions.xlab.text="", barplot_embed.plotoptions.xlog.state="", barplot_embed.plotoptions.xmaxvalue.text="", barplot_embed.plotoptions.xminvalue.text="", barplot_embed.plotoptions.yaxt.state="", barplot_embed.plotoptions.ylab.text="", barplot_embed.plotoptions.ylog.state="", barplot_embed.plotoptions.ymaxvalue.text="", barplot_embed.plotoptions.yminvalue.text="", barplot_embed.type.string="juxtaposed", names_exp.text="rownames (swiss)", names_mode.string="rexp", tabulate.state="0", x.available="swiss[[\"Catholic\"]]", submit.mode="submit")

			rk.call.plugin ("rkward::barplot", barplot_embed.colors.string="default", barplot_embed.legend.state="1", barplot_embed.plotoptions.add_grid.state="0", barplot_embed.plotoptions.asp.real="0.00000000", barplot_embed.plotoptions.main.text="", barplot_embed.plotoptions.pointcolor.color.string="", barplot_embed.plotoptions.pointtype.string="", barplot_embed.plotoptions.sub.text="", barplot_embed.plotoptions.xaxt.state="", barplot_embed.plotoptions.xlab.text="", barplot_embed.plotoptions.xlog.state="", barplot_embed.plotoptions.xmaxvalue.text="", barplot_embed.plotoptions.xminvalue.text="", barplot_embed.plotoptions.yaxt.state="", barplot_embed.plotoptions.ylab.text="", barplot_embed.plotoptions.ylog.state="", barplot_embed.plotoptions.ymaxvalue.text="", barplot_embed.plotoptions.yminvalue.text="", barplot_embed.type.string="stacked", names_exp.text="names (x)", names_mode.string="default", tabulate.state="0", x.available="x", submit.mode="submit")
		}),
		new ("RKTest", id="boxplot", call=function () {
			rk.call.plugin ("rkward::box_plot", cex_sd_mean.real="1.00000000", mean.state="TRUE", names_exp.text="names (x)", names_mode.string="default", notch.state="FALSE", orientation.string="FALSE", outline.state="TRUE", pch_mean.real="15.000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", sd.state="", sd_mean_color.color.string="blue", x.available="women[[\"weight\"]]\nwomen[[\"height\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="density_plot", call=function () {
			rk.call.plugin ("rkward::density_plot", adjust.real="1.00000000", bw.string="nrd0", kern.string="gaussian", n.real="512.00000000", narm.state="na.rm=TRUE", plot_type.string="hdr_plot", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", rug.state="0", x.available="women[[\"height\"]]", submit.mode="submit")
		}, libraries = c ("hdrcde")),
		new ("RKTest", id="dotchart", call=function () {
			rk.call.plugin ("rkward::dotchart", names_exp.text="women$weight", names_mode.string="rexp", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="This is a test", plotoptions.mainisquote.state="1", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="This is a subtitle", plotoptions.subisquote.state="1", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", tabulate.state="0", x.available="women[[\"height\"]]", submit.mode="submit")

			rk.call.plugin ("rkward::dotchart", names_exp.text="names (x)", names_mode.string="default", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", tabulate.state="1", x.available="warpbreaks[[\"tension\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="ecdf_plot", call=function () {
			rk.call.plugin ("rkward::ecdf_plot", adjust_th_pnorm.state="1", col_rug.color.string="", col_thnorm.color.string="blue", lwd.real="0.50000000", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", rug.state="1", side.string="side = 3", stepfun_options.addtoplot.state="", stepfun_options.col_hor.color.string="", stepfun_options.col_points.color.string="", stepfun_options.col_y0.color.string="", stepfun_options.col_y1.color.string="", stepfun_options.do_points.state="1", stepfun_options.linetype.string="", stepfun_options.verticals.state="", th_pnorm.state="1", ticksize.real="0.03000000", x.available="swiss[[\"Catholic\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="generic_plot", call=function () {
			rk.call.plugin ("rkward::plot", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", xvarslot.available="swiss", yvarslot.available="", submit.mode="submit")
		}),
		new ("RKTest", id="histogram", call=function () {
			rk.call.plugin ("rkward::histogram", adjust.real="4.00000000", bw.string="nrd", col_density.color.string="blue", density.state="1", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="1", histogram_opt.density.real="-1.000000", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="vec", histogram_opt.histbreaks_veclength.real="6.000000", histogram_opt.histlinetype.string="solid", histogram_opt.include_lowest.state="1", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="", n.real="512.00000000", narm.state="na.rm=TRUE", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", x.available="swiss[[\"Education\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="pareto_chart", call=function () {
			rk.call.plugin ("rkward::pareto", descriptives.state="TRUE", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", tabulate.state="FALSE", x.available="swiss[[\"Catholic\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="piechart", call=function () {
			rk.call.plugin ("rkward::piechart", angle.real="45.00000000", angle_inc.real="6.00000000", clockwise.state="1", colors.string="grayscale", density.real="3.00000000", density_inc.real="1.00000000", names_exp.text="names (x)", names_mode.string="default", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", radius.real="0.80000000", tabulate.state="0", x.available="test_table[[\"A\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="scatterplot", call=function () {
			rk.call.plugin ("rkward::scatterplot", cex.text="1", col.text="c ('black', 'red')", color.string="each", isCex.string="all", isPch.string="all", pch.text="1", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", pointtype.string="p", type_mode.string="all", x.available="women[[\"weight\"]]\nswiss[[\"Education\"]]", y.available="women[[\"height\"]]\nswiss[[\"Catholic\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="scatterplot_matrix", call=function () {
			rk.call.plugin ("rkward::scatterplot_matrix", diag.string="histogram", ellipse.state="FALSE", plot_points.state="TRUE", smooth.state="FALSE", x.available="swiss", submit.mode="submit")
		}),
		new ("RKTest", id="stem_leaf_plot", call=function () {
			rk.call.plugin ("rkward::stem", atom.real="0.00000001", scale.real="1.50000000", width.real="80.00000000", x.available="swiss[[\"Fertility\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="stripchart", call=function () {
			rk.call.plugin ("rkward::stripchart", g.available="warpbreaks[[\"tension\"]]", method.string="stack", offset.real="0.50000000", orientation.string="Horizontal", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00000000", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", x.available="warpbreaks[[\"breaks\"]]", submit.mode="submit")
		})
	), postCalls = list (	# like initCalls: run after all tests to clean up.
		function () {
			suppressWarnings (rm (list=c ("women", "swiss", "warpbreaks", "test_table"), envir=globalenv()))
		}
	)
)

## always store the result in "results" and print it
results <- rktest.runRKTestSuite (suite)
print (results)
