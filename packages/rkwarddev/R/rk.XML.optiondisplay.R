#' Create XML node "optiondisplay" for RKWard plugins
#' 
#' This node is only allowed once insinde the \code{<content>} node of an \code{<optionset>}.
#'
#' @note The \code{<optionset>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param index Logical, whether to show a column with a numeric index in the optiondisplay.
#' @param id.name Character string, a unique ID for this plugin element (optional).
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.optionset]{rk.XML.optionset}},
#'    \code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
rk.XML.optiondisplay <- function(index=TRUE, id.name=NULL){

  if(is.null(id.name)){
    attr.list <- list()
  } else {
    attr.list <- list(id=id.name)
  }

  if(!isTRUE(index)){
    attr.list[["index"]] <- "false"
  } else {}

  node <- XMLNode("optiondisplay",
    attrs=attr.list)

  return(node)
}
