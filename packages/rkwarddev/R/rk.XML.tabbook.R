#' Create XML node "tabbook" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param tabs An optional named list with objects of class \code{XiMpLe.node} (or a list of these objects).
#'    You must provide one named element for each tab. Use \code{NULL} for tabs without predefined children.
#' @param id.name Character vector, unique IDs for the tabbook (first entry) and all tabs.
#'    If \code{"auto"}, IDs will be generated automatically from the labels.
#'    If \code{NULL}, no IDs will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'   rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'   rk.XML.cbox(label="bar", val="bar2")))
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' # combine the above into a tabbook
#' test.tabbook <- rk.XML.tabbook("My Tabbook",
#'   tabs=list("First Tab"=test.checkboxes, "Second Tab"=test.dropdown))
#' cat(pasteXML(test.tabbook))

rk.XML.tabbook <- function(label=NULL, tabs=list(), id.name="auto"){
  tab.labels <- names(tabs)
  num.tabs <- length(tabs)

  # check if number of children fits
  if("" %in% tab.labels & num.tabs > 0){
    stop(simpleError("All tabs must have a label (named list)!"))
  } else {}

  if(identical(id.name, "auto")){
    tab.ids <- auto.ids(tab.labels, prefix=ID.prefix("tab", length=3))
  } else {}
  tabs <- sapply(1:num.tabs, function(this.num){
      this.tab <- tab.labels[[this.num]]
      attr.list <- list(label=this.tab)
      if(identical(id.name, "auto")){
        attr.list[["id"]] <- tab.ids[[this.num]]
      } else if(!is.null(id.name)){
        attr.list[["id"]] <- id.name[[this.num + 1]]
      } else {}
      if(!is.null(tabs[[this.num]])){
        child <- tabs[[this.num]]
      } else {
        child <- list()
      }
      return(XMLNode("tab",
          attrs=attr.list,
          .children=child.list(child, empty=FALSE))
        )
    })


  if(identical(id.name, "auto")){
    if(!is.null(label)){
      tb.id <- auto.ids(label, prefix=ID.prefix("tabbook", length=4))
    } else {
      # try autogenerating some id
      tb.id <- auto.ids(tab.labels, prefix=ID.prefix("tabbook", length=4))
    }
  } else if(!is.null(id.name)){
    tb.id <- id.name[[1]]
  } else {
    tb.id <- NULL
  }

  tbk.attr.list <- list(id=tb.id)
  if(!is.null(label)){
    tbk.attr.list[["label"]] <- label
  } else {}

  tabbook <- XMLNode("tabbook",
      attrs=tbk.attr.list,
      .children=child.list(tabs, empty=FALSE)
    )

  return(tabbook)
}
