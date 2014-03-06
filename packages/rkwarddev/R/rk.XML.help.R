#' Create XML node "help" for RKWard plugins
#'
#' @param file A character string, the file name to be included as reference.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.help <- rk.XML.help("some_file.rkh")
#' cat(pasteXML(test.help))

rk.XML.help <- function(file){
  node <- XMLNode("help", attrs=list(file=as.character(file)))
  return(node)
}
