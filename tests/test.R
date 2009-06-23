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
		representation (id="character", call="function", fuzzy_output="logical", expect_error="logical"),
		prototype(character(0), id=NULL, call=function () { stop () }, fuzzy_output=FALSE, expect_error=FALSE),
		validity=function (object) {
			if (is.null (object@id)) return (FALSE)
			return (TRUE)
		}
	)

setClass ("RKTestResult",
		representation (id = "character", code_match = "logical", output_match = "logical", message_match = "logical", error="logical", passed="logical"),
		prototype(character(0), id = character (0), code_match = NA, output_match = NA, message_match = NA, error = NA, passed=FALSE),
		validity=function (object) {
			return (all.equal (length (object@id), length (object@code_match), length (object@output_match), length (object@message_match), length (object@error), length (object@passed)))
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
		cat (format (if (object@code_match[i]) "true" else "FALSE", width=15))
		cat (format (if (object@output_match[i]) "true" else "FALSE", width=15))
		cat (format (if (object@message_match[i]) "true" else "FALSE", width=15))
		cat (format (if (object@error[i]) "TRUE" else "false", width=15))
		cat (format (if (object@passed[i]) "pass" else "FAIL", width=15))
		cat ("\n")
	}
})

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
	if ((length (output.diff) == 1) && (!nzchar (output.diff))) return (TRUE)

	print (paste ("Differences between", file, "and", standard_file, ":"))
	print (output.diff)
	return (FALSE)
}

rktest.runRKTest.internal <- function (test, output_file, code_file, message_file) {
	# save / restore old output file
	old_output <- rk.get.output.html.file ()
	rk.set.output.html.file (output_file)
	on.exit (rk.set.output.html.file (old_output), add=TRUE)

	message_file_handle <- file (message_file, open="w+")
	sink(message_file_handle, type="message")
	on.exit ({
			sink (NULL, type="message")
			close (message_file_handle)
		}, add=TRUE)

	rk.record.commands (code_file)
	on.exit (rk.record.commands (NULL), add=TRUE)

	failed <- TRUE
	try ({
		test@call ()
		failed <- FALSE
	})
	return (failed)
}

rktest.runRKTest <- function (test) {
	result <- new ("RKTestResult")		# FALSE by default

	if (!inherits (test, "RKTest")) return (result)
	result@id <- test@id
	if (!validObject (test)) return (result)

	output_file <- rktest.file (test@id, ".rkout")
	code_file <- rktest.file (test@id, ".rkcom")
	message_file <- rktest.file (test@id, ".rkwarn")

	# the essence of the test:
	result@error <- rktest.runRKTest.internal (test, output_file, code_file, message_file)

	result@output_match = rktest.compare.against.standard (output_file)
	result@message_match = rktest.compare.against.standard (message_file)
	result@code_match = rktest.compare.against.standard (code_file)

	if ((result@error == test@expect_error) && (result@output_match || test@fuzzy_output) && result@code_match && result@message_match) result@passed = TRUE
	
	result
}

rktest.cleanRKTestSuite <- function (suite, basedir=getwd ()) {
	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (paste (basedir, suite@id, sep="/"))

	files <- list.files ()
	# do not delete the standards!
	files <- grep (".*\\.standard$", files, value=TRUE, invert=TRUE)

	file.remove (files)

	invisible (NULL)
}

rktest.runRKTestSuite <- function (suite, basedir=getwd ()) {
	result <- new ("RKTestResult")		# FALSE by default

	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	# clean any old results
	rktest.cleanRKTestSuite (suite, basedir)

	system (paste ("tar -xzf", suite@id, ".tar.gz", sep=""))
	oldwd = getwd ()
	on.exit (setwd (oldwd))
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

rktest.setSuiteStandards <- function (suite, basedir=getwd ()) {
	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	ok <- readline ("You are about to set new standards for this suite. This means you are certain that ALL tests in this suite have produced the expected/correct result on the last run. If you are absolutely sure, enter \"I am sure\" to proceed.");
	if (ok != "I am sure") stop ("Aborted")

	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (paste (basedir, suite@id, sep="/"))

	files <- list.files ()
	files <- grep (".*\\.(rkwarn|rkcom|rkout)$", files, value=TRUE)
	file.copy (files, paste (files, ".standard", sep=""), overwrite=TRUE)

	# clean anything that is *not* a standard file
	rktest.cleanRKTestSuite (suite, basedir)

	# create package
	setwd (basedir)
	system (paste ("tar -czf ", suite@id, ".tar.gz ", suite@id, sep=""))
}


# You can use this to temporarily replace .rk.rerun.plugin.link.
# This way, after running a plugin, you are shown the call needed to run this
# plugin with those settings, instead of the link.
.rk.rerun.plugin.link.replacement <- function (plugin, settings, label) {
	.rk.cat.output ("<h3>Rerun code:</h3>")
	.rk.cat.output ("<pre>")
	.rk.cat.output ("rk.call.plugin (\"")
	.rk.cat.output (plugin)
	.rk.cat.output ("\", ")
	.rk.cat.output (gsub ("=", "=\"", gsub ("\n", "\", ", gsub ("\"", "\\\"", settings))))
	.rk.cat.output ("\", submit.mode=\"submit\")</pre>")
}

# HACK: Override date, so we don't get a difference for each call of rk.header ()
# TODO: implement a clean solution inside rk.header()
date <- function () {
	return ("DATE")
}
