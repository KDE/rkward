## intro
# This should be the first line in each test suite file: Include the
# test framework, unless already included (multiple inclusion would not
# really do any harm either, though
if (!isClass ("RKTestSuite")) source ("test_framework.R")

## definition of the test suite
suite <- new ("RKTestSuite", id="import_export_plugins",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("datasets"),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			# prepare some different files for loading
			library ("datasets")
			women <- datasets::women

			save (women, file="women.RData")
			write.csv (women, file="women.csv")

			suppressWarnings (rm ("women"))
		}
	## the tests
	), tests = list (
		new ("RKTest", id="load_r_object", call=function () {
			rk.call.plugin ("rkward::load_r_object", file.selection="women.RData", other_env.state="0", submit.mode="submit")

			stopifnot (all.equal (.GlobalEnv$women, datasets::women))
		}),
		new ("RKTest", id="import_csv", call=function () {
			rk.call.plugin ("rkward::import_csv", allow_escapes.state="", blanklinesskip.state="TRUE", checkname.state="TRUE", colclass.string="", colname.string="", dec.string="'.'", doedit.state="0", file.selection="women.csv", flush.state="", isrow.state="true", na.text="NA", name.selection="women", nrows.text="-1", quick.string="csv", quote.string="'\\\"'", sep.string="','", skip.text="0", strings_as_factors.string="", stripwhite.state="FALSE", rowname.string="rowcol", nomrow.text="1", submit.mode="submit")

			stopifnot (all.equal (.GlobalEnv$women, datasets::women))
		}),
		new ("RKTest", id="import_csv_overwrite", call=function () {
			assign ("women", datasets::women, envir=globalenv ())
			rk.sync.global ()

			# this one is expected to fail, as it would overwrite the existing "women" in globalenv()
			rk.call.plugin ("rkward::import_csv", file.selection="women.csv", name.selection="women", submit.mode="submit")
		}, expect_error=TRUE),
		new ("RKTest", id="setworkdir", call=function () {
			oldwd <- getwd ()
			on.exit (setwd (oldwd))

			# we can only use relative paths, here, to make sure the tests produce identical commands on all systems
			rk.call.plugin ("rkward::setworkdir", dir.selection="..", submit.mode="submit")
			stopifnot (oldwd != getwd ())

			rk.call.plugin ("rkward::setworkdir", dir.selection="import_export_plugins", submit.mode="submit")
			stopifnot (oldwd == getwd ())
		}),
		new ("RKTest", id="import_spss", call=function () {
			rk.call.plugin ("rkward::import_spss", convert_var_labels.state="1", data_frame.state="1", do_locale_conversion.state="0", doedit.state="0", file.selection="../import_export_plugins_testfile.sav", labels_limit.real="1.00", saveto.selection="my.spss.data", trim_labels.state="0", use_labels.state="1", submit.mode="submit")

			# In order to check, whether the import was correct
			rk.print (my.spss.data)
			for (var in my.spss.data) rk.print (rk.get.description(var))

			# WARNING: TODO: We don't use the value labels of the third
			# variable, yet.
		}, libraries=c("foreign")),
		new ("RKTest", id="import_stata", call=function () {
			rk.call.plugin ("rkward::import_stata", convert_dates.state="1", convert_factors.state="1", convert_underscore.state="0", doedit.state="0", file.selection="../import_export_plugins_testfile.dta", missing_type.state="0", saveto.selection="my.stata.data", submit.mode="submit")

			# In order to check, whether the import was correct
			rk.print (my.stata.data)
			for (var in my.stata.data) rk.print (rk.get.description(var))
		}, libraries=c("foreign")),
		new ("RKTest", id="load_source", call=function () {
			stopifnot (!exists ("testx", globalenv ()))

			cat ("testx <- c (20:30)\nprint (\"ok\")\n", file="source.R")

			rk.call.plugin ("rkward::load_source", chdir.state="FALSE", echo.state="0", file.selection="source.R", local.state="TRUE", printeval.state="FALSE", submit.mode="submit")

			stopifnot (!exists ("testx", globalenv ()))

			rk.call.plugin ("rkward::load_source", chdir.state="FALSE", echo.state="1", file.selection="source.R", local.state="FALSE", printeval.state="FALSE", submit.mode="submit")

			stopifnot (globalenv()$testx == c (20:30))
		}),
		new ("RKTest", id="save_r_object", call=function () {
			# in this test we try to save to object with different settings, then reload them.
			assign ("testx", datasets::warpbreaks, envir=globalenv())
			assign ("testy", datasets::volcano, envir=globalenv())
			rk.sync.global()

			rk.call.plugin ("rkward::save_r", ascii.state="TRUE", compress.state="TRUE", data.available="testx", file.selection="x.RData", submit.mode="submit")
			rk.call.plugin ("rkward::save_r", ascii.state="TRUE", compress.state="TRUE", data.available="testy", file.selection="y.RData", submit.mode="submit")

			rm (testx, testy, envir=globalenv())
			load ("x.RData")
			stopifnot (testx == datasets::warpbreaks)
			load ("y.RData")
			stopifnot (testy == datasets::volcano)
		}),
		new ("RKTest", id="write_vector_matrix", call=function () {
			assign ("testx", c (1:10), globalenv())
			rk.sync.global()

			rk.call.plugin ("rkward::save_variables", append.state="FALSE", data.available="testx", file.selection="data", ncolumns.real="2.", sep.string=",", submit.mode="submit")

			x <- readLines ("data")
			for (line in x) rk.print (line)
		}),
		new ("RKTest", id="write_table", call=function () {
			assign ("women", datasets::women, globalenv())
			rk.sync.global()

			rk.call.plugin ("rkward::save_table", append.state="FALSE", columns.string="TRUE", data.available="women", dec.string="'.'", eol.text="\\n", file.selection="data", na.text="NA", qmethod.string="'escape'", quote.state="TRUE", rows.string="FALSE", sep.string="'\\t'", submit.mode="submit")

			x <- readLines ("data")
			for (line in x) rk.print (line)
		}),
		new ("RKTest", id="package_skeleton", call=function () {
			rk.call.plugin ("rkward::save_skeleton", data.available="rktest.setSuiteStandards\nrktest.runRKTestSuite", force.state="TRUE", name.text="anRpackage", path.selection=".", submit.mode="submit")
		})
	), postCalls = list ()	# like initCalls: run after all tests to clean up. Empty in this case.
)

## always store the result in "results" and print it
results <- rktest.runRKTestSuite (suite)
print (results)
