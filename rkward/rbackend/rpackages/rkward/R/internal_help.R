# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
## Internal functions related to help search / display

# retrieve the (expected) "base" url of help files. Most importantly this will be a local port for R 2.10.0 and above, but a local directory for 2.9.x and below. As a side effect, in R 2.10.0 and above, the dynamic help server is started.
#' @importFrom utils compareVersion
#' @export
".rk.getHelpBaseUrl" <- function () {
	port <- NA
	try ({
		port <- tools::startDynamicHelp ()
	})
	if (is.na (port)) {
		try ({
			port <- tools:::httpdPort
			if (is.function(port)) port <- port()
		})
	}
	if (is.na (port)) {
		return (paste ("file://", R.home (), sep=""))
	}
	return (paste ("http://127.0.0.1", port, sep=":"))
}

# a simple wrapper around help() that makes it easier to detect in code, whether help was found or not.
# used from RKHelpSearchWindow::getFunctionHelp
#' @importFrom utils help
#' @export
".rk.getHelp" <- function (topic, package=NULL, ...) {
	res <- help (topic, (package), ..., help_type="html")
	if (!length (as.character (res))) {	# this seems undocumented, but it is what utils:::print.help_files_with_topic checks
		if (!is.null (package)) {
			# if no help found, try once more, without package restriction
			res <- help (topic, package=NULL, ..., help_type="html")
		}
		if (!length (as.character (res))) {
			show (res)
			stop ("No help found")
		}
	}
	show (res)
	invisible (TRUE)
}

# Simple wrapper around help.search. Concatenates the relevant fields of the results in order for passing to the frontend.
#' @importFrom utils help.search
#' @export
".rk.get.search.results" <- function (pattern, ...) {
	H = as.data.frame(help.search(pattern, ...)$matches)
	fields <- names (H)
	c(
		# NOTE: Field header capitalization appears to have changed in some version of R around 3.2.x
		as.character (if ("topic" %in% fields) H$topic else H$Topic),
		as.character (if ("title" %in% fields) H$title else H$Title),
		as.character (if ("package" %in% fields) H$package else H$Package),
		# NOTE: The field "Type" was added in R 2.14.0. For earlier versions of R, only help pages were returned as results of help.search()
		as.character (if ("Type" %in% fields) H$Type else rep ("help", length.out=dim(H)[1]))
	)
}
