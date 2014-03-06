#' Create XML "caption" node for RKWard help pages
#'
#' This function will create a caption node for settings sections in RKWard help files.
#'
#' @param id Either a character string (the \code{id} of the XML element to explain),
#'    or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param title Character string, title to be displayed. If \code{NULL}, the \code{label}
#'    of the element will be shown.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}},
#'    \code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}}
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a sample frame
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' test.frame <- rk.XML.frame(test.dropdown, label="Some options")
#' # create the caption
#' test.caption <- rk.rkh.caption(test.frame)
#' cat(pasteXML(test.caption))

rk.rkh.caption <- function(id, title=NULL){
  # let's see if we need to extract IDs first
  attr.list <- list(id=check.ID(id))

  if(!is.null(title)){
    attr.list[["title"]] <- title
  } else {}

  node <- XMLNode(name="caption", attrs=attr.list)

  return(node)
}
