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
#' @return Returns the object created by \code{sessionInfo()}, invisibly. Note
#'   that this includes only the information on the R portion of the session.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{sessionInfo}}
#' @keywords utilities misc
#' @importFrom utils sessionInfo
#' @export
#' @rdname rk.sessionInfo
#' @examples
#' 
#' rk.sessionInfo()

"rk.sessionInfo" <- function () {
	cat (.rk.do.plain.call ("getSessionInfo"), sep="\n")
	cat ("R runtime session info:\n")
	print (sessionInfo())
}
