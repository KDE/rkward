library (rkwardtests)

## add your test suite files, to this vector:
testsuites <- c (
	"rkward_application_tests.R",
	"import_export_plugins.R",
	"item_response_theory.R",
	"analysis_plugins.R",
	"distributions.R",
	"plots.R")

rktest.makeplugintests (testsuites, getwd (), outfile="make_plugintests.txt")
