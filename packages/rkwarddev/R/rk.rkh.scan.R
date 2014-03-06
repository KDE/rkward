#' Create RKWard help nodes from plugin XML
#'
#' @param pXML Either an object of class \code{XiMpLe.doc} or \code{XiMpLe.node}, or path to a plugin XML file.
#' @param help Logical, if \code{TRUE} a list of XiMpLe.node objects will be returned, otherwise a character
#'    vector with only the relevant ID names.
#' @param captions Logical, if \code{TRUE} captions will be generated for all "page", "tab" and "frame" nodes.
#' @return A character vector or a list of XiMpLe.node objects.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.rkh.scan <- function(pXML, help=TRUE, captions=TRUE){

  settings.tags <- c("radio", "varslot", "browser", "dropdown",
    "checkbox", "saveobject", "input", "spinbox")
  if(isTRUE(captions)){
    caption.tags <- c("page", "tab", "frame")
  } else {
    caption.tags <- NULL
  }
  help.relevant.tags <- c(settings.tags, caption.tags)

  single.tags <- get.single.tags(XML.obj=pXML, drop=c("comments","cdata", "declarations", "doctype"))

  help.id <- get.IDs(single.tags=single.tags, relevant.tags=help.relevant.tags, add.abbrev=FALSE, tag.names=TRUE)

  if("id" %in% colnames(help.id)){
    if(isTRUE(help)){
      help.nodes <- unlist(sapply(1:nrow(help.id), function(this.id){
          if(help.id[this.id,"tag"] %in% caption.tags){
            return(rk.rkh.caption(id=help.id[this.id,"id"]))
          } else {
            return(rk.rkh.setting(id=help.id[this.id,"id"]))
          }
        }))
    } else {
      help.nodes <- help.id[,"id"]
    }
  } else {
    help.nodes <- NULL
  }

  return(help.nodes)
}
