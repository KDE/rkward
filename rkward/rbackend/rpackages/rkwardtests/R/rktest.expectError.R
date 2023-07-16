# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Helper functions to check for expected errors
#'
#' @description
#' \code{rktest.commandFailed()} runs the given statement(s), which is expected to produce an error. Returns TRUE, if the command failed, FALSE, if it succeeded.
#' \code{rktest.exptectError()} runs the given statement(s), which is expected to produce an error. If the command succeeds, and error is produced.
#'
#' @note In both cases, the error message of the original command will still be printed, unless silent=TRUE (as a warning), but execution will continue afterwards.
#' 
#' @aliases rktest.commandFailed rktest.expectError
#' @param expr Expression to evaluate
#' @param message Error message to generate, if the expected error failed to occur. If NULL, the deparsed expression itself is cited.
#' @param silent if TRUE suppress the error message itself (and any warnings).
#' @rdname rktest.expectError
#' @return TRUE if the command failed. rktest.commandFailed() returns FALSE, if the command did not fail.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @examples
#' if(!rktest.commandFailed(stop("Expected error"))) stop("Failed to generate error")
#' @export
rktest.commandFailed <- function(expr, silent=FALSE){
  failed = TRUE
  try({
    eval.parent(expr)
    failed = FALSE
  }, silent=silent)
  failed
}

#' @export
#' @rdname rktest.expectError
rktest.expectError <- function(expr, message=NULL, silent=FALSE){
  failed = TRUE
  try({
    eval.parent(expr)
    failed = FALSE
  }, silent=silent)
  if(!failed) {
    if(is.null(message)) {
      message = deparse(substitute(expr))
    }
    stop(paste("Failed to generate expected error for: ", message))
  }
  invisible(TRUE)
}
