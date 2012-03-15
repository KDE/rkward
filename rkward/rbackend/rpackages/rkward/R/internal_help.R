## Internal functions related to help search / display

# retrieve the (expected) "base" url of help files. Most importantly this will be a local port for R 2.10.0 and above, but a local directory for 2.9.x and below. As a side effect, in R 2.10.0 and above, the dynamic help server is started.
#' @export
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
#' @export
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
#' @export
".rk.get.search.results" <- function (pattern, ...) {
	H=as.data.frame (help.search(pattern, ...)$matches)
	# NOTE: The field "Type" was added in R 2.14.0. For earlier versions of R, only help pages were returned as results of help.search()
	if ((dim (H)[1] > 0) && (is.null (H$Type))) H$Type <- "help"
	c (as.character (H$topic), as.character (H$title), as.character(H$Package), as.character(H$Type))
}
