# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

## definition of the test suite
library("rkwardtests", lib.loc=rk.home("lib"))
suite <- new ("RKTestSuite", id="rkward_application_tests",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = character (0),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
		}
	## the tests
	), tests = list (
		new ("RKTest", id="active_binding", call=function () {
			.GlobalEnv$active.binding.value <- 1
			makeActiveBinding("active.binding", function () { message ("active.binding"); .GlobalEnv$active.binding.value }, .GlobalEnv)
			message("before sync")
			rk.sync.global() # NOTE: This will currently create two "active.binding"-messages, one for the binding being copied to the shadow env,
			                 #       and one for the frontend syncing info about the symbol.
			                 #       That's an implementation detail, and could reasonably change, i.e. a failure to match the exact messages is
			                 #       not necessarily an error. It just needs careful review.
			                 #       The further tests, below, *are* critical, however
			message("after sync")

			stopifnot(.GlobalEnv$active.binding == .GlobalEnv$active.binding.value)

			.GlobalEnv$active.binding.value <- 123
			stopifnot(.GlobalEnv$active.binding == 123)

			# NOTE: the message "active.binding" should be displayed in the message output
		}),
		new ("RKTest", id="object_modifications", call=function () {
			env <- new.env()
			for (a in letters) {
				for (b in letters) {
					for (c in letters) {
						assign(paste0(a, b, c), 1, pos=env);
					}
				}
			}
			assign(paste0("lll", 0), 1, pos=env);
			rkward:::rk.check.env.changes(env)
			res <- system.time({
				for (i in 1:5) {
					env$lll <- env$lll + 1
					assign(paste0("lll", i), 1, pos=env);
					rm(list=paste0("lll", i-1), pos=env);
					rk.print(rkward:::rk.check.env.changes(env))
				}
			})
			# this is really crude, and might give false positives, but the idea is trying to catch potential performance regressions
			stopifnot(res[1] < 0.5)
		}),
		new ("RKTest", id="promise_in_globalenv", call=function () {
			.GlobalEnv$promised.value <- 1
			delayedAssign ("promise.symbol", { message ("delayed assign"); promised.value }, eval.env=.GlobalEnv, assign.env=.GlobalEnv)

			rk.sync.global () # should evaluate the promise, but not force it permanently
			message ("after sync")

			.GlobalEnv$promised.value <- 123
			# promise should be permanently forced at this point:
			stopifnot (.GlobalEnv$promise.symbol == .GlobalEnv$promised.value)

			# promise should not be evaluated again
			.GlobalEnv$promised.value <- 245
			stopifnot (.GlobalEnv$promise.symbol == 123)
		}),
		new ("RKTest", id="output_graphics_formats", call=function () {
			rk.graph.on(); plot (1, 1); rk.graph.off()	# should produce PNG, 480*480
			old.options <- options()
			options (rk.graphics.type="JPG",
				 rk.graphics.width=500,
				 rk.graphics.height=500,
				 rk.graphics.jpg.quality=34)
			rk.graph.on(); plot (1, 1); rk.graph.off()
			# restore options
			options (rk.graphics.type=old.options$rk.graphics.type,
				 rk.graphics.width=old.options$rk.graphics.width,
				 rk.graphics.height=old.options$rk.graphics.height,
				 rk.graphics.jpg.quality=old.options$rk.graphics.jpg.quality)

			rk.graph.on (device.type="SVG", width=300); plot (1, 1); rk.graph.off ()
		}),
		new ("RKTest", id="no_crash_on_strange_objects", call=function () {
			# This object was created by library (XML) v. 3.1-0
			# xmlTreeParse ("<log><description>An unclosed quote\"</description></log>")
			#
			# In R 2.11.0, str (x) produces an error "subecript out of bounds" (only if library (XML) is loaded!
			# The main concern is that we should handle this object gracefully, i.e. do not crash while syncing it.
			load ("rkward_application_tests_strange_object.RData")
			rk.sync.global ()
			rk.sync (x)
		}, libraries=c ("XML"), files=c("../rkward_application_tests_strange_object.RData")),
		new ("RKTest", id="dev_off_bug", call=function () {
			graphics.off()
			stopifnot (is.null (dev.list ()))

			plot (1, 1); rk.embed.device (grDevices::x11()); plot (2, 2)

			stopifnot (all.equal (as.numeric (dev.list ()), c (2, 3)))
			dev.off (2)
			stopifnot (all.equal (as.numeric (dev.list ()), 3))
			dev.off ()
			stopifnot (is.null (dev.list ()))
		}),
		new ("RKTest", id="plot_history_basics", call=function () {
			le <- "package:lattice" %in% search ()
			compareCurrentPlotWith <- function (x) {
				if (inherits (x, "trellis")) {
					matches <- identical (trellis.last.object (), x)
				} else {
					matches <- identical (recordPlot (), x)
				}
				if (!matches) {
					message ("Current plot does not match with ", deparse (substitute (x)))
				}
			}

			graphics.off()
			rk.clear.plot.history()
			options(rk.graphics.hist.max.plotsize=1000)
			rk.toggle.plot.history(TRUE)
			rk.verify.plot.hist.limits (5)
			assign (".pop.notify", FALSE, envir=rk.record.plot)

			plots <- list ()
			plot (1, 1)
			plots[[1]] <- recordPlot()
			if (le) {
				print (xyplot (2~2))
				plots[[2]] <- trellis.last.object ()
			} else {
				plot (2,2)
				plots[[2]] <- recordPlot()
			}
			plot (3, 3)
			plots[[3]] <- recordPlot()
			rk.force.append.plot ()
			stopifnot (dev.cur() == 2)
			RK ()
			plot (4, 4)
			plots[[4]] <- recordPlot()
			if (le) {
				print (xyplot (5~5))
				plots[[5]] <- trellis.last.object ()
			} else {
				plot (5,5)
				plots[[5]] <- recordPlot()
			}
			stopifnot (dev.cur() == 3)

			## Navigation
			message ("mark 1")
			rk.previous.plot (2)
			stopifnot (dev.cur() == 3)
			dev.set (2)
			compareCurrentPlotWith (plots[[2]])
			rk.next.plot (2)
			compareCurrentPlotWith (plots[[3]])

			rk.previous.plot (3)
			dev.set (3)
			compareCurrentPlotWith (plots[[4]])
			rk.next.plot (3)
			compareCurrentPlotWith (plots[[5]])

			dev.set (2)
			rk.goto.plot (2, 1)
			compareCurrentPlotWith (plots[[1]])

			## Removing
			message ("mark 2")
			# The plot should be removed in device 3, too
			rk.removethis.plot (2)
			compareCurrentPlotWith (plots[[2]])
			message ("mark 3")
			dev.set (3)
			rk.first.plot (3)
			compareCurrentPlotWith (plots[[2]])

			message ("mark 4")
			# this time, the plot was shown in both devices. It should not have be removed in the other!
			rk.removethis.plot (3)
			compareCurrentPlotWith (plots[[3]])
			dev.set (2)
			compareCurrentPlotWith (plots[[2]])

			## Reaching the history limit
			message ("mark 5")
			# three plots in history at this time, and one pending in device 2
			dev.set (3)
			rk.first.plot ()
			compareCurrentPlotWith (plots[[3]])
			rk.last.plot ()
			compareCurrentPlotWith (plots[[5]])
			dev.set (2)
			plot (1, 1)
			plot (1, 1)
			# five plots in history at this time, and one pending in device 2
			rk.force.append.plot ()	# first should have been popped, now
			rk.first.plot ()
			compareCurrentPlotWith (plots[[4]])

			## Duplicating plots
			message ("mark 6")
			rk.verify.plot.hist.limits (10)
			rk.duplicate.device ()
			stopifnot (dev.cur() == 4)
			title (main = "plot [[4]]: duplicated")
			plots[[6]] <- recordPlot ()
			rk.first.plot () 
			# at this stage 6 plots are in history, duplicated plot is at pos = 6
			dev.set (2)
			compareCurrentPlotWith (plots[[4]])
			message ("mark 7")
			title (main = "plot [[4]]: altered")
			plots[[7]] <- recordPlot ()
			rk.next.plot (); rk.previous.plot (); # overwrites at pos = 1
			compareCurrentPlotWith (plots[[7]])
			dev.set (4)
			rk.force.append.plot () # original plot 4, is now at position 7
			compareCurrentPlotWith (plots[[4]])
			rk.previous.plot (); # duplicated plot
			compareCurrentPlotWith (plots[[6]])

			graphics.off ()
			rk.clear.plot.history()

			## Manage only screen devices
			message ("mark 8")
			plot (1, 1)
			fname <- rk.get.tempfile.name ()
			message ("mark 8a")
			jpeg (filename = fname)
			plot (2,2)
			RK ()
			plot (3,3)
			stopifnot (identical (c(1,2,4), as.numeric (names (rk.record.plot$histPositions))))
			graphics.off ()
			rk.clear.plot.history()
			file.remove (fname)

			## Switching plot history on/off
			message ("mark 9")
			plots <- list ()
			plot (1,1); plots[[1]] <- recordPlot()
			plot (2,2); plots[[2]] <- recordPlot()
			plot (3,3); plots[[3]] <- recordPlot()
			RK ()
			plot (4,4); plots[[4]] <- recordPlot()
			rk.toggle.plot.history(FALSE)
			stopifnot (rk.record.plot$sP.length == 4)
			plot (5,5)
			plot (6,6); plots[[6]] <- recordPlot()
			dev.set (2)
			plot (7,7); plots[[7]] <- recordPlot()
			stopifnot (rk.record.plot$sP.length == 4)
			rk.toggle.plot.history(TRUE)
			rk.force.append.plot ()
			compareCurrentPlotWith (plots[[7]])
			dev.set (3)
			rk.force.append.plot ()
			compareCurrentPlotWith (plots[[6]])
			stopifnot (rk.record.plot$sP.length == 6)

			graphics.off ()
			rk.clear.plot.history()
			message ("mark 10")
		}, libraries=c ("lattice")),
		new ("RKTest", id="device_capturing_stress_test", call=function () {
			# This test checks for the "figure margins too large" error, that used to occur when plotting on a fresh device, sometimes.
			# Since the error only appeared occasionally, we used to try 100 times to produce it, but that does made the test run annoyingly long...
			graphics.off()
			for (i in 1:10) {
				rk.embed.device (grDevices::x11())
				plot (rnorm (100), main=paste (i, "/ 100"))
				dev.off ()
			}
		}),
		new ("RKTest", id="rk_device_stress_test", call=function () {
			# Somewhat less annyoingly long test for the RK()-device. Stress testing esp. the open/close operations has revealed at least one race-condition bug, previously.
			graphics.off()
			for (i in 1:25) {
				RK ()
				plot (rnorm (100), main=paste (i, "/ 100"))
				dev.off ()
			}
		}),
		new ("RKTest", id="output_capture_interleaving_test", call=function () {
			rk.call.plugin ("rkward::testing_run_code", codetorun.text='
				for (i in 1:20) {
					cat ("A")
					system ("echo B")
				}
			', submit.mode="submit")
		}),
		new ("RKTest", id="rk_output_test", call=function () {
			optwarn <- options("warn"=1) # This test leaks warning messages, otherwise...
			on.exit(options("warn"=optwarn$warn))
			sync_outfile <- function() {
				# Manually notify frontend of current output file.  This is usually suppressed for auto-texting, but important, here, since
				# the frontend uses this to detect, which output is active.
				rkward:::.rk.call.async("set.output.file", c(rk.get.output.html.file()))
			}

			x <- rk.output()            # get active output

			l1 <- length(rk.output(all=TRUE))

			y <- rk.output(create=TRUE) # create new output
			l2 <- length(rk.output(all=TRUE))
			stopifnot(l1 == l2-1)
			stopifnot(x$id != y$id)
			stopifnot(!y$isModified())
			y$activate()
			sync_outfile()
			stopifnot(!y$isModified())  # initialization shall not count as modification
			stopifnot(y$isEmpty())

			rk.print("y")
			stopifnot(y$isModified())
			stopifnot(!y$isEmpty())

			z <- rk.output()            # rk.output() without parameters shall return active output
			stopifnot(y$id == z$id)
			rktest.expectError(z$close(discard=FALSE))
			rktest.expectError(z$revert(discard=FALSE))
			rktest.expectError(z$clear(discard=FALSE))

			f <- tempfile()             # saving shall mark the output as non-modified
			y$save(f)
			stopifnot(!y$isModified())
			stopifnot(!y$isEmpty())
			y$close(discard=FALSE)

			z <- rk.output(f)           # a freshly loaded output shall not count as modified
			stopifnot(!z$isModified())
			stopifnot(!z$isEmpty())

			z$activate()                # exporting shall not mark the output as non-modified
			sync_outfile()
			rk.print("z")
			g <- tempfile()
			z$export(g)
			stopifnot(z$isModified())

			z$revert(discard=TRUE)      # revert shall mark output as non-modified
			stopifnot(!z$isModified())

			rk.print("z2")              # dicard=TRUE shall allow closing modified output
			z$close(discard=TRUE)

			rktest.expectError(z$view(), silent=TRUE) # closed output shall be invalid; silent, because error message will contain id

			sync_outfile()
			a <- rk.output()            # When closing, the previous output shall be re-activated
			stopifnot(a$id == x$id)

			unlink(f)
			unlink(g)
		}, ignore=c("code")),
		new ("RKTest", id="run_again_links", call=function () {
			# usually, commands to generate run-again-links are not included in the command recording, since these can sometimes change at a large scale (e.g. if the plot-options plugin gains a new option), and this is rarely of interest. Here, we do include a test to catch any grave problems. For this purpose, we simply call some complex plugins.
			rk.call.plugin ("rkward::plot_beta_clt", a.real="2.00", b.real="2.00", drawnorm.state="1", function.string="hist", histogram_opt.addtoplot.state="", histogram_opt.barlabels.state="1", histogram_opt.density.real="-1.00", histogram_opt.doborder.state="1", histogram_opt.freq.state="0", histogram_opt.histbordercol.color.string="", histogram_opt.histbreaksFunction.string="Sturges", histogram_opt.histfillcol.color.string="azure", histogram_opt.histlinetype.string="solid", histogram_opt.rightclosed.state="1", histogram_opt.usefillcol.state="1", nAvg.real="10.00", nDist.real="1000.00", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", submit.mode="submit")

			rk.call.plugin ("rkward::plot_normal_clt", dist_stepfun.addtoplot.state="", dist_stepfun.col_hor.color.string="blue", dist_stepfun.col_vert.color.string="blue", dist_stepfun.col_y0.color.string="gold", dist_stepfun.col_y1.color.string="cyan", dist_stepfun.do_points.state="", dist_stepfun.linetype.string="", dist_stepfun.verticals.state="1", drawnorm.state="1", function.string="dist", mean.real="0.00", nAvg.real="10.00", nDist.real="1000.00", normlinecol.color.string="red", normpointtype.string="l", plotoptions.add_grid.state="0", plotoptions.asp.real="0.00", plotoptions.main.text="", plotoptions.pointcolor.color.string="", plotoptions.pointtype.string="", plotoptions.sub.text="", plotoptions.xaxt.state="", plotoptions.xlab.text="", plotoptions.xlog.state="", plotoptions.xmaxvalue.text="", plotoptions.xminvalue.text="", plotoptions.yaxt.state="", plotoptions.ylab.text="", plotoptions.ylog.state="", plotoptions.ymaxvalue.text="", plotoptions.yminvalue.text="", scalenorm.state="0", sd.real="1.00", submit.mode="submit")
		}, record.all.commands=TRUE)
	# postCalls are run *after* all tests. Use this to clean up
	), postCalls = list (
		function () {
		}
	)
)
