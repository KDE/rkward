# Copyright 2010-2015 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package rkwarddev.
#
# rkwarddev is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rkwarddev is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rkwarddev.  If not, see <http://www.gnu.org/licenses/>.


#' Create JavaScript saveobject code from plugin XML
#'
#' @param pXML Either an object of class \code{XiMpLe.doc} or \code{XiMpLe.node}, or path to a plugin XML file.
#' @param R.objects Character vector, the names of the internal R objects to be saved. If not empty must have
#'    the same length as <saveobject> nodes in the document, or be the keyword "initial", in which case the
#'    \code{intital} attribute values of the nodes are used.
#' @param vars Logocal, whether the variables needed should also be defined in the JavaScript code.
#' @param add.abbrev Logical, if \code{TRUE} the JavaScript variables will all have a prefix with an
#'    three letter abbreviation of the XML tag type to improve the readability of the code. But it's
#'    probably better to add this in the XML code in the first place.
#' @param indent.by Character string used to indent each entry if \code{js=TRUE}.
#' @return A character vector.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.JS.saveobj <- function(pXML, R.objects="initial", vars=TRUE, add.abbrev=FALSE, indent.by=rk.get.indent()){

  single.tags <- get.single.tags(XML.obj=pXML, drop=c("comments","cdata", "declarations", "doctype"))

  main.indent <- indent(2, by=indent.by)

  # filter for relevant tags
  cleaned.tags <- list()
  for(this.tag in child.list(single.tags)){
    this.tag.name <- tolower(XiMpLe:::XML.tagName(this.tag))
    # we're only interested in entries with an ID
    if(identical(this.tag.name, "saveobject")){
      if("id" %in% names(XiMpLe:::parseXMLAttr(this.tag))){
        cleaned.tags[length(cleaned.tags)+1] <- this.tag
      } else {}
    } else {}
  }

  num.tags <- length(cleaned.tags)

  if(!is.null(R.objects)){
    num.obj <- length(R.objects)
    if(num.obj != num.tags & !identical(R.objects, "initial")){
      stop(simpleError(paste0("Length of 'R.objects' (",num.obj,") is unequal to saveobject nodes found:\n  ",
        paste(unlist(cleaned.tags), collapse="\n  "))))
    } else {}
  } else {}
  
  if(length(cleaned.tags) > 0){
    if(isTRUE(vars)){
      JS.vars <- paste(unlist(sapply(1:num.tags, function(this.tagnum){
          this.tag <- cleaned.tags[this.tagnum]
          if(XiMpLe:::parseXMLAttr(this.tag)[["checkable"]] %in% c("T", "true", "TRUE", "1")){
            modifiers=c("active", "parent")
          } else {
            modifiers="parent"
          }
          JS.id <- get.IDs(single.tags=this.tag, relevant.tags="saveobject", add.abbrev=add.abbrev)
          return(rk.paste.JS(get.JS.vars(
            JS.var=JS.id[1,"abbrev"],
            XML.var=JS.id[1,"id"],
            modifiers=as.list(modifiers),
            default=TRUE),
            level=2,
            indent.by=indent.by))
        })), collapse="")
      # clean up: remove empty elements
      JS.vars <- JS.vars[!grepl("^[[:space:]]*$", JS.vars)]
    } else {
      JS.vars <- NULL
    }

    JS.assign <- paste(unlist(sapply(1:num.tags, function(this.tagnum){
        this.tag <- cleaned.tags[this.tagnum]
        JS.id <- get.IDs(single.tags=this.tag, relevant.tags="saveobject", add.abbrev=add.abbrev)
        JS.var <- camelCode(JS.id[1,"abbrev"])
        JS.var.parent <- camelCode(c(JS.id[1,"abbrev"], "parent"))
        if(is.null(R.objects)){
          this.obj <- "REPLACE.ME.obj"
        } else {
          if(identical(R.objects, "initial")){
            this.obj <- XiMpLe:::parseXMLAttr(this.tag)[["initial"]]
          } else {
            this.obj <- R.objects[this.tagnum]
          }
        }
        # this can't be done by echo() because of the substitution
        echo.code <- id("echo(\".GlobalEnv$\" + ", JS.var, " + \" <- ", this.obj, "\\n\");")
        if(XiMpLe:::parseXMLAttr(this.tag)[["checkable"]] %in% c("T", "true", "TRUE", "1")){
          JS.var.active <- camelCode(c(JS.id[1,"abbrev"], "active"))
          JS.code <- ite(JS.var.active, echo.code)
        } else {
          JS.code <- echo.code
        }
        return(rk.paste.JS(JS.code, level=2, indent.by=indent.by))
      })), collapse="\n")

    results <- paste0(main.indent, "//// save result object\n",
      if(!is.null(JS.vars)) {
        paste0(main.indent, "// read in saveobject variables\n", JS.vars, "\n")
      } else {}, main.indent, "// assign object to chosen environment\n", JS.assign)
    return(results)
  } else {
    return(invisible(NULL))
  }
}
