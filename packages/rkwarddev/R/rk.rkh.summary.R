#' Create XML "summary" node for RKWard help pages
#'
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' plugin.summary <- rk.rkh.summary("This plugin folds space, using the spice package.")
#' cat(pasteXML(plugin.summary))

rk.rkh.summary <- function(text=NULL){

  if(is.null(text)){
    text <- ""
  } else {}

  node <- XMLNode("summary", text)

  return(node)
}
