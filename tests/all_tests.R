if (!isClass ("RKTestSuite")) source ("test_framework.R")

## add your test suite files, to this vector:
testsuites <- c ("rkward_application_tests.R", "import_export_plugins.R", "item_response_theory.R", "analysis_plugins.R", "distributions.R", "plots.R")

plugintest.outfile <- 'make_plugintests.txt'
sink (file = plugintest.outfile, append=FALSE, type="output", split=TRUE)
cat ("RKWard Version:\n")
print (.rk.app.version)
cat ("\n\nR-Version:\n")
print (R.version)
cat ("\n\nInstalled packages:\n")
print (subset(installed.packages(),select=c(LibPath,Version)))

allresults <- new ("RKTestResult")
for (testsuite in testsuites) {
	source (testsuite)
	allresults <- rktest.appendTestResults (allresults, results)
	rm ("results")
}

cat ("\n\nOverall results:\n")
print (allresults)

if (any (is.na (allresults@passed))) {
	cat ("\nNOTE: Skipped tests due to missing libaries are not an indication of problems.")
	cat ("\nCurrently, the following R packages are needed in order to run all available tests:")
	# TODO: Make this list dynamic and / or print only the missing libs
	cat ('\n"R2HTML", "tseries", "nortest", "outliers", "exactRankTests", "moments", "car", "hdrcde", "qcc", "xtable", "eRm", "ltm"')
}

sink()

cat (paste ("\n\nThese output are saved in: ", paste (getwd(), plugintest.outfile, sep=.Platform$file.sep), ".\nIf needed, send them to rkward-devel@lists.sourceforge.net\n", sep=""))

