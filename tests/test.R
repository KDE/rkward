setClass ("RKTestSuite",
		representation (id="character", initCalls="list", tests="list", postCalls="list"),
		prototype(character(0), id=NULL, initCalls=list(), tests=list(), postCalls=list ()),
		validity=function (object) {
			if (length (object@id) != 1) return (FALSE)
			if (length (object@tests) < 1) return (FALSE)
			return (TRUE)
		}
	)

setClass ("RKTest",
		representation (id="character", call="function", compare_code="logical", compare_output="logical", fuzzy_output="logical", expect_error="logical"),
		prototype(character(0), id=NULL, call=function () { stop () }, compare_code=TRUE, compare_output=TRUE, fuzzy_output=FALSE, expect_error=FALSE),
		validity=function (object) {
			if (is.null (object@id)) return (FALSE)
			return (TRUE)
		}
	)

setMethod ("show", "RKTestResult", function (object) {
	stopifnot (inherits (object, "RKTestResult"))

	cat (format ("ID", width=20))
	cat (format ("code match", width=15))
	cat (format ("output match", width=15))
	cat (format ("message match", width=15))
	cat (format ("error", width=15))
	cat (format ("result", width=15))
	cat ("\n")

	for (i in 1:length (object@id)) {
		cat (format (object@id[i], width=20))
		cat (format (as.character (object@code_match[i]), width=15))
		cat (format (as.character (object@output_match[i]), width=15))
		cat (format (as.character (object@message_match[i]), width=15))
		cat (format (as.character (object@error[i]), width=15))
		cat (format (if (object@passed[i]) "PASS" else "FAIL", width=15))
		cat ("\n")
	}
})

setClass ("RKTestResult",
		representation (id = "character", code_match = "logical", output_match = "logical", message_match = "logical", error="logical", passed="logical"),
		prototype(character(0), id = character (0), code_match = NA, output_match = NA, message_match = NA, error = NA, passed=FALSE),
		validity=function (object) {
			return (all.equal (length (object@id), length (object@code_match), length (object@output_match), length (object@message_match), length (object@error), length (object@passed)))
		}
	)

rktest.appendTestResults <- function (objecta, objectb) {
	stopifnot (inherits (objecta, "RKTestResult") && validObject (objecta))
	stopifnot (inherits (objectb, "RKTestResult") && validObject (objectb))

	index <- length (objecta@id)
	for (i in 1:length (objectb@id)) {
		objecta@id[index+i] = objectb@id[i]
		objecta@code_match[index+i] = objectb@code_match[i]
		objecta@output_match[index+i] = objectb@output_match[i]
		objecta@message_match[index+i] = objectb@message_match[i]
		objecta@error[index+i] = objectb@error[i]
		objecta@passed[index+i] = objectb@passed[i]
	}

	objecta
}

rktest.file <- function (id, extension) {
	file.path(getwd(), paste (id, extension, sep=""))
}

# returns true, if file corresponds to standard.
rktest.compare.against.standard <- function (file) {
	standard_file <- paste (file, ".standard", sep="")
	if (!file.exists (file)) {
		# if neither exists, that means both files are empty
		if (!file.exists (standard_file)) return (TRUE)
	}

	output.diff <- system(paste("diff", shQuote(file), shQuote(standard_file), "2>&1"), intern=TRUE)
	if (!length (output.diff)) return (TRUE)
	return (!nzchar (output.diff))
}

rktest.runRKTest <- function (test) {
	result <- new ("RKTestResult")		# FALSE by default

	if (!inherits (test, "RKTest")) return (result)
	result@id <- test@id
	if (!validObject (test)) return (result)

	# save / restore old output file
	old_output <- rk.get.output.html.file ()
	rk.set.output.html.file (rk.testrun.file (test@id, ".rkout"))
	on.exit (rk.set.output.html.file (old_output), add=TRUE)

	message_file <- rktest.file (test@id, ".rkwarn")
	#sink(message_file, type="message")
	#on.exit (sink (NULL, type="message"))

	#code_file <- rk.testrun.file (test@id, ".rkcom")
	#rk.record.user.commands (code_file)
	#on.exit (rk.record.user.commands (NULL))

	result@error <- TRUE
	try ({
		test@call ()
		result@error <- FALSE
	})

	result@output_match = rktest.compare.against.standard (rk.get.output.html.file ())
	result@message_match = rktest.compare.against.standard (message_file)
	#result@code_match = rktest.compare.against.standard (code_file)
	result@code_match = TRUE		# TODO: only for now!

	if ((result@error == test@expect_error) && (result@output_match || test@fuzzy_output) && result@code_match && result@message_match) result@passed = TRUE
	
	result
}

rktest.runRKTestSuite <- function (suite, basedir=getwd ()) {
	result <- new ("RKTestResult")		# FALSE by default

	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	system (paste ("tar -xf", suite@id, ".tar", sep=""))
	oldwd = getwd ()
	on.exit (setwd (oldwd))

	# clean any old files
	setwd (paste (basedir, suite@id, sep="/"))
	system ("find . -name '*.standard' -o -exec rm {} \\;")#

	setwd (paste (basedir, suite@id, sep="/"))

	if (length (suite@initCalls) > 0) {
		for (i in 1:length (suite@initCalls)) try (suite@initCalls[[i]]())
	}

	for (i in 1:length (suite@tests)) {
		try (res <- rktest.runRKTest(suite@tests[[i]]))
		result <- rktest.appendTestResults (result, res)
	}

	if (length (suite@postCalls) > 0) {
		for (i in 1:length (suite@postCalls)) try (suite@postCalls[[i]]())
	}

	result
}

rktest.packageSuiteStandards <- function (suite, basedir=getwd ()) {
	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	oldwd = getwd ()
	on.exit (setwd (oldwd))

	# create package
	setwd (basedir)
	system (paste ("tar -cf ", suite@id, ".tar ", suite@id, sep=""))
}

rktest.setSuiteStandards <- function (suite, basedir=getwd ()) {
	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (paste (basedir, suite@id, sep="/"))

	system ("find . -name '*.standard' -o -exec cp {} {}.standard \\;")#
}

x <- new ("RKTest", id="firsttest", call=function () rk.print (1))

suite <- new ("RKTestSuite", id="testsuite",
	initCalls = list (
		function () {
			library ("R2HTML")
		}
	), tests = list (
		new ("RKTest", id="firsttestb", call=function () rk.print (1)),
		new ("RKTest", id="secondtest", call=function () rk.print (2)),
		x
	), postCalls = list ()
)

y <- rktest.runRKTestSuite (suite)

y
