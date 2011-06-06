# these functions are all used internally 

# create an internal environment for temporary information storage
#' @export .rktest.tmp.storage
.rktest.tmp.storage <- new.env()

.rk.rerun.plugin.link.replacement <- function (plugin, settings, label) {
	.rk.cat.output ("<h3>Rerun code:</h3>")
	.rk.cat.output ("<pre>")
	.rk.cat.output ("rk.call.plugin (\"")
	.rk.cat.output (plugin)
	.rk.cat.output ("\", ")
	.rk.cat.output (gsub ("^\"", "", gsub ("=", "=\"", gsub ("\n", "\", ", settings))))
	.rk.cat.output ("\", submit.mode=\"submit\")</pre>")
}

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
		objecta@missing_libs[index+i] = objectb@missing_libs[i]
		objecta@passed[index+i] = objectb@passed[i]
	}

	objecta
}

rktest.file <- function (id, extension, suite.id) {
	# get or create a temporary directory
	temp.suite.dir <- rktest.createTempSuiteDir(suite.id)
	file.path(temp.suite.dir, paste (id, extension, sep=""))
}

# returns true, if file corresponds to standard.
rktest.compare.against.standard <- function (file, standard.path, fuzzy=FALSE) {
	standard_file <- file.path(standard.path, gsub ("^(.*\\/)([^\\/]*)$", "RKTestStandard\\.\\2", file))
	if (file.exists (file)) {
		# purge empty files
		info <- file.info (file)
		if (info$size[1] == 0) file.remove (file)
	}
	if (!file.exists (file)) {
		# if neither exists, that means both files are empty
		if (!file.exists (standard_file)) return ("match (empty)")
	}

	output.diff <- suppressWarnings (system(paste("diff", shQuote(file), shQuote(standard_file), "--strip-trailing-cr", "--new-file"), intern=TRUE))
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

rktest.runRKTest <- function (test, standard.path, suite.id) {
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
		result@missing_libs <- missing_libs
		result@passed <- NA
		cat ("\nSkipping test \"", test@id, "\" due to missing libraries: \"", paste (missing_libs, collapse="\", \""), "\"\n", sep="")
		return (result)
	}

	for (testfile in test@files) {
		file.copy(file.path(standard.path, testfile), getwd())
	}

	output_file <- rktest.file (test@id, ".rkout", suite.id)
	code_file <- rktest.file (test@id, ".rkcommands.R", suite.id)
	message_file <- rktest.file (test@id, ".messages.txt", suite.id)

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

	result@output_match = rktest.compare.against.standard (output_file, standard.path, test@fuzzy_output)
	if (result@output_match == "MISMATCH") passed <- FALSE
	result@message_match = rktest.compare.against.standard (message_file, standard.path)
	if (result@message_match == "MISMATCH") passed <- FALSE
	result@code_match = rktest.compare.against.standard (code_file, standard.path)
	if (result@code_match == "MISMATCH") passed <- FALSE

	result@passed <- passed

	result
}

rktest.cleanRKTestSuite <- function (suite) {
      # kept for backwards compatibility ;-)
      rktest.removeTempSuiteDir(suite@id)
}

## Convenience functions for replacing / restoring functions for the test runs
rktest.replace <- function (name, replacement, envir=as.environment ("package:rkward"), backup.name=name) {
	if (exists (backup.name, envir=.rktest.tmp.storage, inherits=FALSE)) {
		message ("It looks like ", name, " has already been replaced. Not replacing it again.")
	} else {
		assign (backup.name, get (name, envir), envir=.rktest.tmp.storage)
		assign (name, replacement, envir)
	}
}

rktest.restore <- function (name, envir=as.environment ("package:rkward"), backup.name=name) {
	if (exists (backup.name, envir=.rktest.tmp.storage, inherits=FALSE)) {
		assign (name, get (backup.name, envir=.rktest.tmp.storage), envir=envir)
	} else {
		message ("No backup available for ", name, ". Already restored?")
	}
	rm (list=backup.name, envir=.rktest.tmp.storage)
}

## Initialize test environment
rktest.initializeEnvironment <- function () {
	# Almost all tests depend on R2HTML, indirectly, so we should really assume it (or have the user install it) at the start
	stopifnot (require (R2HTML))

	# By default .rk.rerun.plugin.link() and .rk.make.hr() are silenced during the test runs
	rktest.replace (".rk.rerun.plugin.link", function (...) list (...))
	rktest.replace (".rk.make.hr", function (...) list (...))

	# This should make the output of rk.graph.on() fixed
	rktest.replace ("rk.get.tempfile.name", function (prefix="image", extension=".jpg") paste (prefix, extension, sep=""))
	options (rk.graphics.type="PNG", rk.graphics.width=480, rk.graphics.height=480)

	# HACK: Override date, so we don't get a difference for each call of rk.header ()
	# TODO: implement a clean solution inside rk.header()
	# Note: date is in baseenv() and we cannot easily replace it there, so placing an override in globalenv(), instead
	assign ("date", function () return ("DATE"), envir=globalenv())

	# numerical precision is often a problem. To work around this in many places, reduce default printed precision to 5 digits
	options (digits=5)

	# Make sure i18n does not get in the way
	invisible (Sys.setenv (LANGUAGE="C"))
	if (.Platform$OS.type == "unix") invisible (Sys.setlocale ("LC_MESSAGES", "C"))
	options (useFancyQuotes=FALSE)

	# This version of rk.set.output.html.file does not notify the frontend of the change. Without this, you'll get lots of output windows.
	rktest.replace ("rk.set.output.html.file", function (x) {
		stopifnot(is.character(x))
		assign(".rk.output.html.file", x, as.environment("package:rkward"))
		.rk.do.plain.call ("set.output.file", c (x, "SILENT"), synchronous=FALSE)
	})
	assign("initialized", TRUE, envir=.rktest.tmp.storage)
}

# counterpart to rktest.initializeEnvironment. Restores the most important settings
rktest.resetEnvironment <- function () {
	# return to previously dumped state
	rktest.restore (".rk.rerun.plugin.link")
	rktest.restore (".rk.make.hr")
	rktest.restore ("rk.get.tempfile.name")
	rktest.restore ("rk.set.output.html.file")
	rm (date, envir=globalenv())
	assign("initialized", FALSE, envir=.rktest.tmp.storage)
}

## handling of temporary directories
# create a temporary directory for the test results
# the path to it will be stored in an object in globalenv() and returned
rktest.createTempDir <- function(){
  temp.dir <- rktest.getTempDir()
  # if a temp.dir already exists, we will use it!
  if(is.character(temp.dir)){
    return(temp.dir)
  } else{}
  new.temp.dir <- tempfile("rktests.")

  if(!dir.create(new.temp.dir, recursive=TRUE)) {
    stop(simpleError("Couldn't create temporary directory!"))
  }
  else {
    assign(".rktest.temp.dir", new.temp.dir, envir=.rktest.tmp.storage)
    return(new.temp.dir)
  }
}

# remove the temporary directory that is defined in globalenv()
rktest.removeTempDir <- function(){
  temp.dir <- rktest.getTempDir()
  if(is.character(temp.dir)){
    unlink(temp.dir, recursive=TRUE)
    # should the function stop here if unlink() failed?
    rm(".rktest.temp.dir", envir=.rktest.tmp.storage)
    return(TRUE)
  }
  else {
    return(FALSE)
  }
}

# create a suite directory inside the temp dir
# for the actual test files
rktest.createTempSuiteDir <- function(suite.id){
  # create or get the temp base dir to use
  temp.dir <- rktest.createTempDir()
  temp.suite.dir <- file.path(temp.dir, suite.id)
  # check if this dir already exists, then just return its path
  if(file_test("-d", temp.suite.dir)){
    return(temp.suite.dir)
  }
  # if not, try to create it and again return its path
  else {
    if(!dir.create(temp.suite.dir, recursive=TRUE)) {
      stop(simpleError("Couldn't create temporary suite directory!"))
    }
    else {
      return(temp.suite.dir)
    }
  }
}

# remove just the suite temp dir
rktest.removeTempSuiteDir <- function(suite.id){
  temp.dir <- rktest.getTempDir()
  if(is.character(temp.dir)){
    temp.suite.dir <- file.path(temp.dir, suite.id)
    # check if this dir exists
    if(file_test("-d", temp.suite.dir)){
      unlink(temp.suite.dir, recursive=TRUE)
      # if nothing is left in the base tempdir now, remove it as well
      if(length(list.files(temp.dir)) == 0) {
	rktest.removeTempDir()
      } else {}
      return(TRUE)
    }
    # if not, return FALSE
    else {
      return(FALSE)
    }
  }
  else {
    return(FALSE)
  }
}
