#' Create XML node "text" for RKWard plugins
#'
#' @param text Character string, the text to be displayed.
#' @param type One value of either "normal", "warning" or "error".
#' @param id.name Character string, a unique ID for this plugin element.
#'    If \code{"auto"}, an ID will be generated automatically from \code{text}.
#'    If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.text <- rk.XML.text("Added this text.")
#' cat(pasteXML(test.text))

rk.XML.text <- function(text, type="normal", id.name="auto"){
  if(identical(id.name, "auto")){
    # try autogenerating some id
    attr.list <- list(id=auto.ids(text, prefix=ID.prefix("text")))
  } else if(is.null(id.name)){
    attr.list <- list()
  } else {
    attr.list <- list(id=id.name)
  }

  if(identical(type, "warning") | identical(type, "error")){
    attr.list[["type"]] <- type
  } else {}

  # preserve markup in the text
  if(grepl("<(.*)>", text)){
    textAsTree <- parseXMLTree(text, object=TRUE)
    node <- XMLNode("text",
        .children=slot(textAsTree, "children"),
        attrs=attr.list)
  } else {
    node <- XMLNode("text",
        text,
        attrs=attr.list)
  }


  return(node)
}
