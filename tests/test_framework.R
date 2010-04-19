setClass ("RKTestSuite",
		representation (id="character", libraries="character", initCalls="list", tests="list", postCalls="list"),
		prototype(character(0), id=NULL, libraries=character(0), initCalls=list(), tests=list(), postCalls=list ()),
		validity=function (object) {
			if (length (object@id) != 1) return (FALSE)
			if (length (object@tests) < 1) return (FALSE)
			return (TRUE)
		}
	)

setClass ("RKTest",
		representation (id="character", call="function", fuzzy_output="logical", expect_error="logical", libraries="character"),
		prototype(character(0), id=NULL, call=function () { stop () }, fuzzy_output=FALSE, expect_error=FALSE, libraries=character(0)),
		validity=function (object) {
			if (is.null (object@id)) return (FALSE)
			return (TRUE)
		}
	)

setClass ("RKTestResult",
		representation (id = "character", code_match = "character", output_match = "character", message_match = "character", error="character", passed="logical"),
		prototype(character(0), id = character (0), code_match = character (0), output_match = character (0), message_match = character (0), error = character (0), passed=FALSE),
		validity=function (object) {
			return (all.equal (length (object@id), length (object@code_match), length (object@output_match), length (object@message_match), length (object@error), length (object@passed)))
		}
	)

setMethod ("show", "RKTestResult", function (object) {
	stopifnot (inherits (object, "RKTestResult"))

	cat (format ("ID", width=30))
	cat (format ("code match", width=15))
	cat (format ("output match", width=15))
	cat (format ("message match", width=15))
	cat (format ("error", width=15))
	cat (format ("result", width=15))
	cat ("\n", rep ("-", 96), "\n", sep="")

	for (i in 1:length (object@id)) {
		cat (format (object@id[i], width=30))
		cat (format (object@code_match[i], width=15))
		cat (format (object@output_match[i], width=15))
		cat (format (object@message_match[i], width=15))
		cat (format (object@error[i], width=15))
		cat (format (if (is.na (object@passed[i])) "--skipped--" else if (object@passed[i]) "pass" else "FAIL", width=15))
		cat ("\n")
	}

	cat (rep ("-", 96), "\n", sep="")
	cat (as.character (sum (object@passed, na.rm=TRUE)), " / ", as.character (sum (!is.na (object@passed))), " tests passed\n")
	if (sum (is.na (object@passed)) > 0) cat ("(", as.character (sum (is.na (object@passed))), " / ", as.character (length (object@passed)), " tests skipped due to missing libraries)", sep="");
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
rktest.compare.against.standard <- function (file, fuzzy=FALSE) {
	standard_file <- gsub ("^(.*\\/)([^\\/]*)$", "\\1RKTestStandard\\.\\2", file)
	if (file.exists (file)) {
		# purge empty files
		info <- file.info (file)
		if (info$size[1] == 0) file.remove (file)
	}
	if (!file.exists (file)) {
		# if neither exists, that means both files are empty
		if (!file.exists (standard_file)) return ("match (empty)")
	}

	output.diff <- system(paste("diff", shQuote(file), shQuote(standard_file), "--strip-trailing-cr", "--new-file"), intern=TRUE)
	if (!length (output.diff)) return ("match")
	if ((length (output.diff) == 1) && (!nzchar (output.diff))) return ("match")

	# below: there are *some* differences
	if (fuzzy) {
		size <- if (file.exists (file)) file.info (file)$size[1] else 0
		s_size <- if (file.exists (standard_file)) file.info (standard_file)$size[1] else 0

		# crude test: files should at least have a similar size
		if ((size < (s_size + 20)) && (size > (s_size - 20))) return ("fuzzy match")
	}

	print (paste ("Differences between", file, "and", standard_file, ":"))
	print (output.diff)

	return ("MISMATCH")
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

	old.symbols <- ls (envir=globalenv (), all.names=TRUE)
	on.exit ({
			# clean up any new objects created by the test
			new.symbols <- ls (envir=globalenv (), all.names=TRUE)
			new.symbols <- new.symbols[!(new.symbols %in% old.symbols)]
			rm (list=new.symbols, envir=globalenv ())
			rk.sync.global ()
		}, add=TRUE)

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

	missing_libs <- character(0)
	for (lib in test@libraries) {
		if (!suppressWarnings (base::require (lib, character.only=TRUE, quietly=TRUE))) {
			missing_libs <- c (missing_libs, lib)
		}
	}
	if (length (missing_libs) > 0) {
		result@output_match <- result@message_match <- result@code_match <- NA_character_
		result@error <- "missing lib(s)"
		result@passed <- NA
		cat ("\nSkipping test \"", test@id, "\" due to missing libraries: \"", paste (missing_libs, collapse="\", \""), "\"\n", sep="")
		return (result)
	}

	output_file <- rktest.file (test@id, ".rkout")
	code_file <- rktest.file (test@id, ".rkcommands.R")
	message_file <- rktest.file (test@id, ".messages.txt")

	# the essence of the test:
	res.error <- rktest.runRKTest.internal (test, output_file, code_file, message_file)
	passed <- (res.error == test@expect_error)
	if (res.error) {
		if (test@expect_error) result@error <- "expected error"
		else result@error <- "ERROR"
	} else {
		if (test@expect_error) result@error <- "MISSING ERROR"
		else result@error <- "no"
	}

	result@output_match = rktest.compare.against.standard (output_file, test@fuzzy_output)
	if (result@output_match == "MISMATCH") passed <- FALSE
	result@message_match = rktest.compare.against.standard (message_file)
	if (result@message_match == "MISMATCH") passed <- FALSE
	result@code_match = rktest.compare.against.standard (code_file)
	if (result@code_match == "MISMATCH") passed <- FALSE

	result@passed <- passed

	result
}

rktest.cleanRKTestSuite <- function (suite, basedir=getwd ()) {
	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (paste (basedir, suite@id, sep="/"))

	files <- list.files ()
	# do not delete the standards!
	files <- grep ("^RKTestStandard\\..*\\.(messages.txt|rkcommands.R|rkout)$", files, value=TRUE, invert=TRUE)

	unlink (files)

	invisible (NULL)
}

rktest.runRKTestSuite <- function (suite, basedir=getwd ()) {
	rktest.initializeEnvironment ()
	result <- new ("RKTestResult")		# FALSE by default

	if (!inherits (suite, "RKTestSuite")) return (result)
	if (!validObject (suite)) return (result)

	# clean any old results
	rktest.cleanRKTestSuite (suite, basedir)

	oldwd = getwd ()
	on.exit (setwd (oldwd))
	setwd (paste (basedir, suite@id, sep="/"))

	if (length (suite@initCalls) > 0) {
		for (i in 1:length (suite@initCalls)) try (suite@initCalls[[i]]())
	}
	rk.sync.global ()	# objects might have been added/changed in the init calls

	for (i in 1:length (suite@tests)) {
		suite@tests[[i]]@libraries <- c(suite@libraries, suite@tests[[i]]@libraries)
		try (res <- rktest.runRKTest(suite@tests[[i]]))
		result <- rktest.appendTestResults (result, res)
	}

	if (length (suite@postCalls) > 0) {
		for (i in 1:length (suite@postCalls)) try (suite@postCalls[[i]]())
	}

	rktest.resetEnvironment ()

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
	files <- grep ("\\.(messages.txt|rkcommands.R|rkout)$", files, value=TRUE)
	files <- grep ("^RKTestStandard", files, value=TRUE, invert=TRUE)
	file.copy (files, paste ("RKTestStandard.", files, sep=""), overwrite=TRUE)

	# clean anything that is *not* a standard file
	rktest.cleanRKTestSuite (suite, basedir)
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
	.rk.cat.output (gsub ("^\"", "", gsub ("=", "=\"", gsub ("\n", "\", ", settings))))
	.rk.cat.output ("\", submit.mode=\"submit\")</pre>")
}
# Run this line to do the replacement:
# .rk.rerun.plugin.link <- .rk.rerun.plugin.link.replacement

## Initialize test environment
rktest.initializeEnvironment <- function () {
	# Almost all tests depend on R2HTML, indirectly, so we should really assume it (or have the user install it) at the start
	stopifnot (require (R2HTML))

	# By default .rk.rerun.plugin.link() and .rk.make.hr() are silenced during the test runs
	.rk.rerun.plugin.link <<- .rk.make.hr <<- function (...) { list (...) }

	# This should make the output of rk.graph.on() fixed
	rk.get.tempfile.name <<- function (prefix, extension) paste (prefix, extension, sep="")
	options (rk.graphics.type="PNG", rk.graphics.width=480, rk.graphics.height=480)

	# HACK: Override date, so we don't get a difference for each call of rk.header ()
	# TODO: implement a clean solution inside rk.header()
	assign ("date", function () {
		return ("DATE")
	}, envir=globalenv())

	# numerical precision is often a problem. To work around this in many places, reduce default printed precision to 5 digits
	options (digits=5)

	# Make sure i18n does not get in the way
	invisible (Sys.setenv (LANGUAGE="C"))
	if (.Platform$OS.type == "unix") invisible (Sys.setlocale ("LC_MESSAGES", "C"))
	options (useFancyQuotes=FALSE)

	# This version of rk.set.output.html.file does not notify the frontend of the change. Without this, you'll get lots of output windows.
	rk.set.output.html.file <<- function (x) {
		stopifnot(is.character(x))
		assign(".rk.output.html.file", x, as.environment("package:rkward"))
	}
}
rktest.initializeEnvironment ()

# counterpart to rktest.initializeEnvironment. Restores the most important settings
rktest.resetEnvironment <- function () {
	rm (list=c ("rk.set.output.html.file", "rk.get.tempfile.name", ".rk.make.hr"), envir=globalenv ())
	.rk.rerun.plugin.link <<- .rk.rerun.plugin.link.replacement
}
