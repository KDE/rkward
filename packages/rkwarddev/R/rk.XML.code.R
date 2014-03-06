#' Create XML node "code" for RKWard plugins
#'
#' @param file A character string, the JavaScript file name to be included.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.code <- rk.XML.code("some_file.js")
#' cat(pasteXML(test.code))

rk.XML.code <- function(file){
  node <- XMLNode("code", attrs=list(file=as.character(file)))
  return(node)
}
