## intro
# This should be the first line in each test suite file: Include the
# test framework, unless already included (multiple inclusion would not
# really do any harm either, though
if (!isClass ("RKTestSuite")) source ("test_framework.R")

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
			# In R 2.11.0, str (x) produces an error "subecript out of bounds"
			# The main concern is that we should handle this object gracefully, i.e. do not crash while syncing it.
			load ("../rkward_application_tests_strange_object.RData")
			rk.sync.global ()
			rk.sync (x)
		})
	# postCalls are run *after* all tests. Use this to clean up
	), postCalls = list (
		function () {
		}
	)
)

## always store the result in "results" and print it
results <- rktest.runRKTestSuite (suite)
print (results)
