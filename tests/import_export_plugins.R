# This should be the first line in each test suite file: Include the
# test framework, multiple inclusion should do no harm
source ("test.R")

x <- new ("RKTest", id="firsttest", call=function () rk.print (1))

suite <- new ("RKTestSuite", id="import_export_plugins",
	initCalls = list (
		function () {
			library ("R2HTML")
		},
		function () {
			women <- datasets::women
			save (women, file="women.RData")
		}
	), tests = list (
		new ("RKTest", id="load_r_object", call=function () {
			suppressWarnings (rm ("women"))

			rk.call.plugin ("rkward::load_r_object", file.selection="women.RData", other_env.state="0", submit.mode="submit")

			stopifnot (all.equal (women, datasets::women))
		}),
		new ("RKTest", id="secondtest", call=function () rk.print (2)),
		x
	), postCalls = list ()
)

y <- rktest.runRKTestSuite (suite)

print (y)
