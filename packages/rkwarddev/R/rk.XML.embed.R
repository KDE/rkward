#' Create XML node "embed" for RKWard plugins
#'
#' @param component A character string, registered name (\code{id} in pluginmap file) of the component to be embedded.
#' @param button Logical, whether the plugin should be embedded as a button and appear if it's pressed.
#' @param label A character string, text label for the button (only used if \code{button=TRUE}).
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from the label and component strings.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.embed <- rk.XML.embed("someComponent")
#' cat(pasteXML(test.embed))

rk.XML.embed <- function(component, button=FALSE, label="Options", id.name="auto"){
  attr.list <- list(component=component)

  if(isTRUE(button)){
    attr.list[["as_button"]] <- "true"
  } else {}
  
  if(!identical(label, "Options") && isTRUE(button)){
    attr.list[["label"]] <- label
  } else {}

  if(identical(id.name, "auto")){
    attr.list[["id"]] <- auto.ids(paste0(label, component), prefix=ID.prefix("embed"))
  } else {
    attr.list[["id"]] <- id.name
  }

  node <- XMLNode("embed", attrs=attr.list)

  return(node)
}
