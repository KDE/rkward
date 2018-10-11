#' Check for pandoc installation and its features
#' 
#' This function looks for a pandoc installation and if found, gathers information
#' on its path, supported output formats, and the version number. It has no parameters.
#' 
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

"rk.check.for.pandoc" <- function(){
  pandoc <- list(available=FALSE)
  pandoc_path <- Sys.which("pandoc")[["pandoc"]]
  if(!"" %in% pandoc_path){
    pandoc[["path"]] <- pandoc_path
    pandoc[["available"]] <- TRUE
    pandoc[["output_formats"]] <- system("pandoc --list-output-formats", intern=TRUE)
    pandoc[["version"]] <- gsub("[[:space:]]*pandoc[[:space:]]*", "", system("pandoc --version", intern=TRUE)[1])
  } else {}
  return(pandoc)
}
