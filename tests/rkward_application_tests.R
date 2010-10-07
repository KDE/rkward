## definition of the test suite
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
			makeActiveBinding ("active.binding", function () { message ("active.binding"); .GlobalEnv$active.binding.value }, .GlobalEnv)

			rk.sync.global ()
			message ("after sync")

			stopifnot (.GlobalEnv$active.binding == .GlobalEnv$active.binding.value)

			.GlobalEnv$active.binding.value <- 123
			stopifnot (.GlobalEnv$active.binding == 123)

			stopifnot (bindingIsActive ("active.binding", rkward::.rk.watched.symbols))

			# NOTE: the message "active.binding" should be displayed in the message output
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

			options (rk.graphics.type="JPG", rk.graphics.width=500, rk.graphics.height=500, rk.graphics.jpg.quality=34)
			rk.graph.on(); plot (1, 1); rk.graph.off()
			rktest.initializeEnvironment ()	# restore options

			rk.graph.on (device.type="SVG", width=300); plot (1, 1); rk.graph.off ()
		}),
		new ("RKTest", id="no_crash_on_strange_objects", call=function () {
			# This object was created by library (XML) v. 3.1-0
			# xmlTreeParse ("<log><description>An unclosed quote\"</description></log>")
			#
			# In R 2.11.0, str (x) produces an error "subecript out of bounds" (only if library (XML) is loaded!
			# The main concern is that we should handle this object gracefully, i.e. do not crash while syncing it.
			load ("../rkward_application_tests_strange_object.RData")
			rk.sync.global ()
			rk.sync (x)
		}, libraries=c ("XML")),
		new ("RKTest", id="dev_off_bug", call=function () {
			graphics.off()
			stopifnot (is.null (dev.list ()))

			plot (1, 1); x11(); plot (2, 2)

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
			x11 ()
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
			x11 ()
			plot (3,3)
			stopifnot (identical (c(1,2,4), as.numeric (rk.record.plot$.hP.names)))
			graphics.off ()
			rk.clear.plot.history()
			file.remove (fname)

			## Switching plot history on/off
			message ("mark 9")
			plots <- list ()
			plot (1,1); plots[[1]] <- recordPlot()
			plot (2,2); plots[[2]] <- recordPlot()
			plot (3,3); plots[[3]] <- recordPlot()
			x11 ()
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
			# Since the error only appeared occasionally, we try 100 times to produce it. Unfortunately, that does make the test run annoyingly long...
			graphics.off()
			for (i in 1:100) {
				rk.screen.device ()
				plot (rnorm (100), main=paste (i, "/ 100"))
				dev.off ()
			}
		})
	# postCalls are run *after* all tests. Use this to clean up
	), postCalls = list (
		function () {
		}
	)
)
