## intro
# This should be the first line in each test suite file: Include the
# test framework, unless already included (multiple inclusion would not
# really do any harm either, though
if (!isClass ("RKTestSuite")) source ("test_framework.R")

## definition of the test suite
suite <- new ("RKTestSuite", id="import_export_plugins",
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			library ("R2HTML")
			library ("datasets")
			library ("foreign")
		},
		function () {
			# prepare some different files for loading
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
			rk.call.plugin ("rkward::import_spss", convert_var_labels.state="1", data_frame.state="1", do_locale_conversion.state="0", doedit.state="0", file.selection="../import_export_plugins_testfile.sav", labels_limit.real="1000000.000000", saveto.selection="my.spss.data", trim_labels.state="0", use_labels.state="1", submit.mode="submit")

			# In order to check, whether the import was correct
			rk.print (my.spss.data)
			for (var in my.spss.data) rk.print (rk.get.description(var))

			# WARNING: TODO: We don't use the value labels of the third
			# variable, yet.
		})
	), postCalls = list ()	# like initCalls: run after all tests to clean up. Empty in this case.
)

## always store the result in "results" and print it
results <- rktest.runRKTestSuite (suite)
print (results)
