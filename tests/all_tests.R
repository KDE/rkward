if (!isClass ("RKTestSuite")) source ("test_framework.R")

## add your test suite files, to this vector:
testsuites <- c ("rkward_application_tests.R", "import_export_plugins.R", "item_response_theory.R")

allresults <- new ("RKTestResult")
for (testsuite in testsuites) {
	source (testsuite)
	allresults <- rktest.appendTestResults (allresults, results)
	rm ("results")
}

print ("Overall results:")
print (allresults)
