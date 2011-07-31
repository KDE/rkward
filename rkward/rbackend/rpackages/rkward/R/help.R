## Public functions related to help search / display
# Like demo(), but opens the demo in a script editor window
"rk.demo" <- function (topic, package=NULL, lib.loc=NULL) {
	if (is.null (package)) {
		package <- .packages (lib.loc=lib.loc)
	}

	loc <- ""
	for (p in package) {
		loc <- system.file ("demo", paste (topic, ".R", sep=""), package=p, lib.loc=lib.loc)
		if (loc != "") break
	}

	if (loc == "") stop ("No demo found for topic'", topic, "'")
	rk.edit.files (loc, prompt=FALSE)
}

## Internal functions related to help search / display

# retrieve the (expected) "base" url of help files. Most importantly this will be a local port for R 2.10.0 and above, but a local directory for 2.9.x and below. As a side effect, in R 2.10.0 and above, the dynamic help server is started.
".rk.getHelpBaseUrl" <- function () {
	port <- NA
	if (compareVersion (as.character (getRversion()), "2.10.0") >= 0) {
		try ({
			port <- tools::startDynamicHelp ()
		})
		if (is.na (port)) {
			try ({
				port <- tools:::httpdPort
			})
		}
	}
	if (is.na (port)) {
		return (paste ("file://", R.home (), sep=""))
	}
	return (paste ("http://127.0.0.1", port, sep=":"))
}

# a simple wrapper around help() that makes it easier to detect in code, whether help was found or not.
# used from RKHelpSearchWindow::getFunctionHelp
".rk.getHelp" <- function (...) {
	if (compareVersion (as.character (getRversion()), "2.10.0") >= 0) {
		res <- help (..., help_type="html")
	} else {
		res <- help (..., chmhelp=FALSE, htmlhelp=TRUE)
	}
	if (!length (as.character (res))) {	# this seems undocumented, but it is what utils:::print.help_files_with_topic checks
		show (res)
		stop ("No help found")
	}
	show (res)
	invisible (TRUE)
}

# Simple wrapper around help.search. Concatenates the relevant fields of the results in order for passing to the frontend.
".rk.get.search.results" <- function (pattern, ...) {
	H=as.data.frame (help.search(pattern, ...)$matches)
	if (is.null (H$Type)) H$Type <- "help"
	c (as.character (H$topic), as.character (H$title), as.character(H$Package), as.character(H$Type))
}
