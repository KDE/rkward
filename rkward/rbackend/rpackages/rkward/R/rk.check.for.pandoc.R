# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Check for pandoc installation and its features
#' 
#' This function looks for a pandoc installation and if found, gathers information
#' on its path, supported output formats, and the version number.
#' 
#' @param stop_if_missing Logical, whether an error should be thrown if either pandoc can't be found at all
#'    or a requested output format is not supported.
#' @param output_format A character string, name of a requested output format. If not \code{NA},
#'    the function will check if the format is supported and return a boolean value.
#' @return A named list with at least the boolean element \code{available}, and if
#'    that is \code{TRUE}, also the character vectors \code{path}, \code{output_formats}, and \code{version}.
#' @author Meik Michalke \email{rkward-devel@@kde.org}
#' @seealso \code{\link{rk.sessionInfo}}
#' @keywords utilities misc
#' @export
#' @rdname rk.check.for.pandoc
#' @examples
#'
#' rk.check.for.pandoc()

"rk.check.for.pandoc" <- function(stop_if_missing=FALSE, output_format=NA){
  pandoc <- list(available=FALSE)
  pandoc_path <- Sys.which("pandoc")[["pandoc"]]
  if(!"" %in% pandoc_path){
    pandoc[["path"]] <- pandoc_path
    pandoc[["available"]] <- TRUE
    pandoc[["output_formats"]] <- system("pandoc --list-output-formats", intern=TRUE)
    pandoc[["version"]] <- gsub("[[:space:]]*pandoc[[:space:]]*", "", system("pandoc --version", intern=TRUE)[1])

    if(!is.na(output_format)){
      pandoc <- output_format %in% pandoc[["output_formats"]]
      if(all(isTRUE(stop_if_missing), !isTRUE(pandoc))){
        stop(simpleError(
          paste0("The requested output format '", output_format, "' is not supported by your pandoc installation.")
        ))
      } else {}
    } else {}
  } else if(isTRUE(stop_if_missing)){
      stop(simpleError("'pandoc' was not found, please check your installation!"))
  } else {}

  return(pandoc)
}
