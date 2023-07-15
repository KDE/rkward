# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

## definition of the test suite
suite <- new ("RKTestSuite", id="import_export_plugins",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("datasets"),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			# prepare some different files for loading
			library ("datasets")
			assign("women.data", datasets::women, pos=globalenv())
		}
	## the tests
	), tests = list (
		new ("RKTest", id="load_r_object", call=function () {
			save (women.data, file="women.RData")

			rk.call.plugin ("rkward::load_r_object", file.selection=file.path (getwd(), "women.RData"), envir.active="0", submit.mode="submit")

			stopifnot (all.equal (.GlobalEnv$women.data, datasets::women))
		}),
		new ("RKTest", id="import_csv", call=function () {
			write.csv (women.data, file="women.csv")

			rk.call.plugin ("rkward::import_csv", allow_escapes.state="0", blanklinesskip.state="1", checkname.state="1", colclass.string="", colname.string="", commentchar.text="", dec.string="'.'", doedit.state="0", file.selection=file.path (getwd(), "women.csv"), fill.state="1", flush.state="0", header.state="1", na.text="NA", name.objectname="women", name.parent=".GlobalEnv", nomrow.text="1", nrows.text="-1", quick.string="csv", quote.string="'\\\"'", rowname.string="rowcol", sep.string="','", skip.text="0", strings_as_factors.string="", stripwhite.state="0", submit.mode="submit")

			stopifnot (all.equal (.GlobalEnv$women, datasets::women))
		}),
		new ("RKTest", id="import_csv_overwrite", call=function () {
			assign ("women", datasets::women, envir=globalenv ())
			rk.sync.global ()

			# this one is expected to fail, as it would overwrite the existing "women" in globalenv()
			rk.call.plugin ("rkward::import_csv", allow_escapes.state="0", blanklinesskip.state="1", checkname.state="1", colclass.string="", colname.string="", commentchar.text="", dec.string="'.'", doedit.state="0", file.selection=file.path (getwd(), "women.csv"), fill.state="1", flush.state="0", header.state="1", na.text="NA", name.objectname="women", name.parent=".GlobalEnv", nomrow.text="1", nrows.text="-1", quick.string="csv", quote.string="'\\\"'", rowname.string="rowcol", sep.string="','", skip.text="0", strings_as_factors.string="", stripwhite.state="0", submit.mode="submit")
		}, expect_error=TRUE),
		new ("RKTest", id="setworkdir", call=function () {
			oldwd <- getwd ()
			on.exit (setwd (oldwd))

			newwd = file.path (getwd(), "testdir")
			dir.create(newwd)
			rk.call.plugin ("rkward::setworkdir", dir.selection=newwd, submit.mode="submit")
			stopifnot (oldwd != getwd ())

			rk.call.plugin ("rkward::setworkdir", dir.selection=oldwd, submit.mode="submit")
			stopifnot (oldwd == getwd ())
			unlink(newwd)
		}),
		new ("RKTest", id="import_spss", call=function () {
# NOTE: read.spss currently failing when run in non iso8859-1 locale. See http://r.789695.n4.nabble.com/read-spss-locale-and-encodings-td881149.html
			rk.call.plugin ("rkward::import_spss", convert_var_labels.state="1", data_frame.state="1", do_locale_conversion.state="0", doedit.state="0", file.selection=file.path (getwd(), "import_export_plugins_testfile.sav"), labels_limit.real="1.00", saveto.objectname="my.spss.data", trim_labels.state="0", use_labels.state="1", submit.mode="submit")

			# In order to check, whether the import was correct
			rk.print (my.spss.data)
			for (var in my.spss.data) rk.print (rk.get.description(var))

			# WARNING: TODO: We don't use the value labels of the third
			# variable, yet.
		}, libraries=c("foreign"), files=c("../import_export_plugins_testfile.sav")),
		new ("RKTest", id="import_generic_rio", call=function () {
			rk.call.plugin ("rkward::import_generic_rio", do_locale_conversion.state="0", doedit.state="0", file.selection=file.path (getwd(), "import_export_plugins_testfile.sav"), saveto.objectname="my.rio.data", use_labels.state="1", submit.mode="submit")

			# In order to check, whether the import was correct
			rk.print (my.rio.data)
			for (var in my.rio.data) rk.print (rk.get.description(var))
		}, libraries=c("rio"), files=c("../import_export_plugins_testfile.sav")),
		new ("RKTest", id="import_stata", call=function () {
			rk.call.plugin ("rkward::import_stata", convert_dates.state="1", convert_factors.state="1", convert_underscore.state="0", do_locale_conversion.state="1", doedit.state="0", encoding.string="ISO8859-1", file.selection=file.path (getwd(), "import_export_plugins_testfile.dta"), missing_type.state="0", saveto.objectname="my.stata.data", saveto.parent=".GlobalEnv", submit.mode="submit")

			# In order to check, whether the import was correct
			rk.print (my.stata.data)
			for (var in my.stata.data) rk.print (rk.get.description(var))
		}, libraries=c("foreign"), files=c("../import_export_plugins_testfile.dta")),
		new ("RKTest", id="import_xls_xlconnect", call=function () {
			rk.call.plugin ("rkward::import_xls_xlconnect", autofitcol.state="1", autofitrow.state="1", coltypes.tsv="", doedit.state="0", endcol.real="0.00", endrow.real="7.00", file.selection="/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xlsx", header.state="1", range.text="", rownames.text="", saveto.objectname="my.xls.data", saveto.parent=".GlobalEnv", sheet.text="1", startcol.real="0.00", startrow.real="0.00", submit.mode="submit")
			rk.print (my.xls.data)
			rk.call.plugin ("rkward::import_xls_xlconnect", autofitcol.state="1", autofitrow.state="1", coltypes.tsv="", doedit.state="0", file.selection="/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xls", header.state="0", range.text="A6:B9", rownames.text="", saveto.objectname="my.xlsx.data", saveto.parent=".GlobalEnv", sheet.text="1", submit.mode="submit")
			rk.print (my.xlsx.data)
		}, libraries=c("XLConnect"), files=c("../import_export_plugins_testfile.xls", "../import_export_plugins_testfile.xlsx")),
		new ("RKTest", id="import_xls", call=function () {
			rk.call.plugin ("rkward::import_xls", doedit.state="0", file.selection="/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xls", fill.state="0", header.state="1", na.text="NA", nrows.text="-1", saveto.objectname="my.xls.data", saveto.parent=".GlobalEnv", sheetname.text="1", skip.text="0", strings_as_factors.string="", stripwhite.state="0", verbose.state="0", submit.mode="submit")
			rk.print (my.xls.data)
			# TODO: known bug: does not import numeric columns from xlsx-files correctly
		}, libraries=c("gdata"), files=c("../import_export_plugins_testfile.xls", "../import_export_plugins_testfile.xlsx")),
		new ("RKTest", id="load_source", call=function () {
			stopifnot (!exists ("testx", globalenv ()))

			cat ("testx <- c (20:30)\nprint (\"ok\")\n", file="source.R")

			rk.call.plugin ("rkward::load_source", chdir.state="FALSE", echo.state="0", file.selection=file.path (getwd(), "source.R"), local.state="TRUE", printeval.state="FALSE", submit.mode="submit")

			stopifnot (!exists ("testx", globalenv ()))

			rk.call.plugin ("rkward::load_source", chdir.state="FALSE", echo.state="1", file.selection=file.path (getwd(), "source.R"), local.state="FALSE", printeval.state="FALSE", submit.mode="submit")

			stopifnot (globalenv()$testx == c (20:30))
		}),
		new ("RKTest", id="save_r_object", call=function () {
			# in this test we try to save to object with different settings, then reload them.
			assign ("testx", datasets::warpbreaks, envir=globalenv())
			assign ("testy", datasets::volcano, envir=globalenv())
			rk.sync.global()

			rk.call.plugin ("rkward::save_r", ascii.state="true", complevel.real="6.00", compress.string="gzip", data.available="testx", file.selection=file.path (getwd(), "x.RData"), submit.mode="submit")
			rk.call.plugin ("rkward::save_r", ascii.state="", compress.string="xz", complevel.real="9.00", xzextreme.state="true", data.available="testy", file.selection=file.path (getwd(), "y.RData"), submit.mode="submit")

			rm (testx, testy, envir=globalenv())
			load ("x.RData")
			stopifnot (testx == datasets::warpbreaks)
			load ("y.RData")
			stopifnot (testy == datasets::volcano)
		}),
		new ("RKTest", id="write_vector_matrix", call=function () {
			assign ("testx", c (1:10), globalenv())
			rk.sync.global()
			file <- file.path (getwd(), "data")

			rk.call.plugin ("rkward::save_variables", append.state="FALSE", data.available="testx", file.selection=file, ncolumns.real="2.", sep.string=",", submit.mode="submit")

			x <- readLines ("data")
			for (line in x) rk.print (line)
			unlink(file)
		}),
		new ("RKTest", id="write_csv", call=function () {
			assign ("women", datasets::women, globalenv())
			rk.sync.global()
			file <- file.path (getwd(), "data")

			rk.call.plugin ("rkward::save_csv", dec.string=".", encoding.string="", eol.string="\\n", file.selection=file, na.text="NA", qmethod.string="double", quick.string="csv", quote.state="1", rowname.string="TRUE", sep.string=",", x.available="women", submit.mode="submit")

			x <- readLines ("data")
			for (line in x) rk.print (line)
			unlink(file)
		}),
		new ("RKTest", id="git_install", call=function () {
			# Intentionally using a non-existent path, as we do not want to install a package. This test still allows to check that the generated code remains as expected
			# (note: plugin linked from internal help pages!)
			rk.call.plugin ("rkward::install_from_git", fullURL.text="https://github.com/does/not/exist/404.git", submit.mode="submit")
		}, libraries=c("devtools", "usethis")),
		new ("RKTest", id="package_skeleton", call=function () {
			# create two functions to use
			assign ("skel.func1", rkwardtests::rktest.getTempDir, envir=globalenv())
			assign ("skel.func2", rkwardtests::rktest.getTempDir, envir=globalenv())
			rk.sync.global()

			rk.call.plugin ("rkward::save_skeleton", data.available="skel.func1\nskel.func2", force.state="TRUE", name.text="anRpackage", path.selection=getwd(), submit.mode="submit")
			rm (skel.func1, skel.func2, envir=globalenv())
		})
	), postCalls = list (
			function(){rm("women.data", pos=globalenv())}
			)	# like initCalls: run after all tests to clean up. Empty in this case.
)
