# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Print information on the RKWard session
#' 
#' Gathers and prints information on the setup of the current RKWard session.
#' In general, you should always include this information when reporting a bug
#' in RKWard.
#'
#' Typically, when reporting a bug, you should use \code{Help->Report Bug...}
#' from the menu. Internally, this will call \code{rk.sessionInfo()}.
#'
#' The returned information is untranslated, deliberately, as it is meant for
#' pasting into the bug tracker.
#'
#' @return Returns the object created by \code{sessionInfo()}, invisibly. Note
#'   that this includes only the information on the R portion of the session.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{sessionInfo}}
#' @keywords utilities misc
#' @importFrom utils sessionInfo
#' @export
#' @rdname rk.sessionInfo
#' @examples
#' \dontrun{
#' rk.sessionInfo()
#' }
"rk.sessionInfo" <- function() {
	# Non-translatable on purpose. This is meant for posting to the bug tracker, mostly.
	cat("-- Frontend --\n");
	cat(.rk.call("frontendSessionInfo"), sep="\n")
	cat("\n-- Backend --\n");
	cat(.rk.call.backend("backendSessionInfo"), sep="\n")
	cat("\nR runtime session info:\n")
	print(sessionInfo())
}
