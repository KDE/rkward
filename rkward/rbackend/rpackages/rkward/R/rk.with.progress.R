# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Run a command with an associated progress dialog
#'
#' \code{rk.with.progress()} evaluates the given R expression, while showing a UI dialog to monitor the progress, and allowing the user
#' to cancel the operation.
#'
#' @param expr Command to run
#' @param text Short user-visible explanation of what is being done. E.g. "Downloading XY..."
#'
#' @return Returns the result of evaluating the expression. The result is always returned invisibly.
#'
#' @note This function should be used sparingly. For code designed to be run in the R Console window, or as part of other R functions, it
#'        should be omitted. Rather it should only be used for top-level operations where progress is not immediately visible. A prime use case
#'        may be a plugin that performs a large download.
#' 
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' 
#' @keywords utilities
#'
#' @examples
#' \dontrun{
#' rk.with.progress({
#'    for (i in 1:80) {
#'       cat('=')
#'       Sys.sleep(.1)
#'    }
#'    cat('\n')
#' }, "Waiting for nothing...")
#' }
#'
#' @export
rk.with.progress <- function(expr, text="") {
	.rk.variables$with.progress.ret <- NULL
	.rk.variables$with.progress.expr <- substitute(expr)
	.rk.variables$with.progress.env <- parent.frame()
	.rk.call.nested("with.progress", text)
	.rk.variables$with.progress.expr <- NULL
	.rk.variables$with.progress.env <- NULL
	ret <- .rk.variables$with.progress.ret
	if (!is.null(.rk.variables$with.progress.err)) stop(.rk.variables$with.progress.err)
	.rk.variables$with.progress.ret <- NULL
	invisible(ret)
}

# Used internally for rk.with.progress
.rk.with.progress.eval <- function() {
	.rk.variables$with.progress.err <- NULL
	.rk.variables$with.progress.ret <- tryCatch(
		expr=eval(.rk.variables$with.progress.expr, .rk.variables$with.progress.env),
		error=function(e) { .rk.variables$with.progress.err <- e }
	)
	# NOTE we need to signal any error both here (to inform the frontend), and in rk.with.progress() (to relay the failure)
	#      it is a known (but thought-to-be-acceptable) problem that this results in duplicate printing of the error message
	if (!is.null(.rk.variables$with.progress.err)) stop(.rk.variables$with.progress.err)
	invisible(NULL)
}
