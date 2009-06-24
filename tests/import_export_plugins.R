# This should be the first line in each test suite file: Include the
# test framework, multiple inclusion should do no harm
source ("test.R")

suite <- new ("RKTestSuite", id="import_export_plugins",
	initCalls = list (
		function () {
			library ("R2HTML")
		},
		function () {
			# prepare some different files for loading
			women <- datasets::women

			save (women, file="women.RData")
			write.csv (women, file="women.csv")

			suppressWarnings (rm ("women"))
		}
	), tests = list (
		new ("RKTest", id="load_r_object", call=function () {
			rk.call.plugin ("rkward::load_r_object", file.selection="women.RData", other_env.state="0", submit.mode="submit")

			stopifnot (all.equal (women, datasets::women))
		}),
		new ("RKTest", id="import_csv", call=function () {
			rk.call.plugin ("rkward::import_csv", allow_escapes.state="", blanklinesskip.state="TRUE", checkname.state="TRUE", colclass.string="", colname.string="", dec.string="'.'", doedit.state="0", file.selection="women.csv", flush.state="", isrow.state="false", na.text="NA", name.selection="women", nrows.text="-1", quick.string="csv", quote.string="'\\\"'", sep.string="','", skip.text="0", strings_as_factors.string="", stripwhite.state="FALSE", submit.mode="submit")

			stopifnot (all.equal (women, datasets::women))
		}),
		new ("RKTest", id="import_csv_overwrite", call=function () {
			assign ("women", datasets::women, envir=globalenv ())
			rk.sync.global ()

			# this one is expected to fail, as it would overwrite the existing "women" in globalenv()
			rk.call.plugin ("rkward::import_csv", file.selection="women.csv", name.selection="women", submit.mode="submit")
		}, expect_error=TRUE)
	), postCalls = list ()
)

y <- rktest.runRKTestSuite (suite)

print (y)
