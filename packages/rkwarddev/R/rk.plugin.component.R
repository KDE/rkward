# Copyright 2010-2014 Meik Michalke <meik.michalke@hhu.de>
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


#' Generate RKWard plugin components
#'
#' @param about Either a character string with the name of this plugin component, or an object of class \code{XiMpLe.node}
#'    with further descriptive information on it, like its authors and dependencies (see \code{link[XiMpLe:rk.XML.about]{rk.XML.about}}
#'    for details). This is only useful for information that differs from the \code{<about>} section of the \code{.pluginmap} file.
#' @param xml A named list of options to be forwarded to \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}, to generate the GUI XML file.
#'    Not all options are supported because some don't make sense in this context. Valid options are:
#'    \code{"dialog"}, \code{"wizard"}, \code{"logic"} and \code{"snippets"}.
#'    If not set, their default values are used. See \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}} for details.
#' @param js A named list of options to be forwarded to \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}, to generate the JavaScript file.
#'    Not all options are supported because some don't make sense in this context. Valid options are:
#'    \code{"require"}, \code{"results.header"}, \code{"variables"}, \code{"globals"}, \code{"preprocess"}, \code{"calculate"}, \code{"printout"},
#'    \code{"doPrintout"} and \code{"load.silencer"}.
#'    If not set, their default values are used. See \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}} for details.
#' @param rkh A named list of options to be forwarded to \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}, to generate the help file.
#'    Not all options are supported because some don't make sense in this context. Valid options are:
#'    \code{"summary"}, \code{"usage"}, \code{"sections"}, \code{"settings"}, \code{"related"} and \code{"technical"}.
#'    If not set, their default values are used. See \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} for details.
#' @param provides Character vector with possible entries of \code{"logic"}, \code{"dialog"} or \code{"wizard"}, defining what
#'    sections the GUI XML file should provide even if \code{dialog}, \code{wizard} and \code{logic} are \code{NULL}.
#'    These sections must be edited manually and some parts are therefore commented out.
#' @param scan A character vector to trigger various automatic scans of the generated GUI XML file. Valid enties are:
#'    \describe{
#'      \item{\code{"var"}}{Calls \code{\link{rk.JS.scan}} to define all needed variables in the \code{calculate()} function
#'        of the JavaScript file. These variables will be added to variables defined by the \code{js} option, if any (see below).}
#'      \item{\code{"saveobj"}}{Calls \code{\link{rk.JS.saveobj}} to generate code to save R results in the \code{printout()}
#'        function of the JavaScript file. This code will be added to the code defined by the \code{js} option, if any (see below).}
#'      \item{\code{"settings"}}{Calls \code{\link{rk.rkh.scan}} to generate \code{<setting>} sections for each relevant GUI element in
#'        the \code{<settings>} section of the help file. This option will be overruled if you provide that section manually
#'        by the \code{rkh} option (see below).}
#'    }
#' @param hints Logical, if \code{TRUE} and you leave optional entries empty (like \code{rkh=list()}), dummy sections will be added.
#' @param guess.getter Logical, if \code{TRUE} try to get a good default getter function for JavaScript
#'    variable values (if \code{scan} is active). This will use some functions which were added with RKWard 0.6.1, and therefore
#'    raise the dependencies for your plugin/component accordingly. Nonetheless, it's recommended.
#' @param hierarchy A character vector with instructions where to place this component in the menu hierarchy, one list or string.
#'    Valid single values are \code{"file"}, \code{"edit"}, \code{"view"}, \code{"workspace"}, \code{"run"}, \code{"data"},
#'    \code{"analysis"}, \code{"plots"}, \code{"distributions"}, \code{"windows"}, \code{"settings"} and \code{"help"},
#'    anything else will place it in a "test" menu. If \code{hierarchy} is a list, each entry represents the label of a menu level.
#' @param include Character string or vector, relative path(s) to other file(s), which will then be included in the head of the GUI XML document.
#' @param create A character vector with one or more of these possible entries:
#'    \describe{
#'      \item{\code{"xml"}}{Create the plugin \code{.xml} XML file skeleton.}
#'      \item{\code{"js"}}{Create the plugin \code{.js} JavaScript file skeleton.}
#'      \item{\code{"rkh"}}{Create the plugin \code{.rkh} help file skeleton.}
#'    }
#' @param gen.info Logical, if \code{TRUE} comment notes will be written into the genrated documents,
#'    that they were generated by \code{rkwarddev} and changes should be done to the script.
#' @param indent.by A character string defining the indentation string to use.
#' @return An object of class \code{rk.plug.comp}.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' \dontrun{
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'   list(test.dropdown,
#'     rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'     rk.XML.cbox(label="bar", val="bar2"))
#'   ))
#' test.vars <- rk.XML.vars("select some vars", "vars go here")
#' test.tabbook <- rk.XML.dialog(rk.XML.tabbook("My Tabbook",
#'   tabs=c("First Tab"=test.checkboxes, "Second Tab"=test.vars)))
#' 
#' rk.plugin.component("Square the Circle",
#'   xml=list(dialog=test.tabbook))
#' }

rk.plugin.component <- function(about, xml=list(), js=list(), rkh=list(),
  provides=c("logic", "dialog"), scan=c("var", "saveobj", "settings"), guess.getter=FALSE,
  hierarchy="test", include=NULL, create=c("xml", "js", "rkh"), hints=TRUE, gen.info=TRUE, indent.by="\t"){

  if(is.XiMpLe.node(about)){
    # check if this is *really* a about section, otherwise quit and go dancing
    valid.parent(parent="about", node=about, warn=FALSE, see="rk.XML.about")
    # fetch the plugin name
    name <- XMLAttrs(about, "attributes")[["name"]]
    about.node <- about
  } else if(is.character(about) & length(about) == 1) {
    name <- about
    about.node <- NULL
    # also stop creation of DESCRIPTION file
    create <- create[!create %in% "desc"]
  } else {
    stop(simpleError("'about' must be a character string or XiMpLe.node, see ?rk.XML.about()!"))
  }

  # to besure, remove all non-character symbols from name
  name.orig <- name
  name <- clean.name(name)

  # check hierarchy
  if(is.null(hierarchy)){
    hierarchy <- list()
  } else {
    hierarchy <- as.list(hierarchy)
  }

  ## create the full component
  this.component <- new("rk.plug.comp",
    name=name.orig,
    create=create,
    hierarchy=hierarchy
  )

  ## create plugin.xml
  if("xml" %in% create & length(xml) > 0){
    got.XML.options <- names(xml)
    for (this.opt in c("dialog", "wizard", "logic", "snippets")){
      if(!this.opt %in% got.XML.options) {
        xml[[this.opt]] <- eval(formals(rk.XML.plugin)[[this.opt]])
      } else {}
    }
    XML.plugin <- rk.XML.plugin(
      name=name,
      label=name.orig,
      dialog=xml[["dialog"]],
      wizard=xml[["wizard"]],
      logic=xml[["logic"]],
      snippets=xml[["snippets"]],
      provides=provides,
      include=include,
      about=about.node,
      gen.info=gen.info)
    # make sure there's no duplicate IDs
    stopifnot(rk.uniqueIDs(XML.plugin, bool=TRUE))
  } else {
    XML.plugin <- rk.XML.plugin("")
  }
  slot(this.component, "xml") <- XML.plugin

  ## create plugin.js
  js.try.scan <- function(XML.plugin, scan, js, guess.getter){
      if("var" %in% scan){
      var.scanned <- rk.JS.scan(XML.plugin, guess.getter=guess.getter)
      if(!is.null(var.scanned)){
        js[["variables"]] <- paste0(
          ifelse(is.null(js[["variables"]]), "", paste0(js[["variables"]], "\n")),
          var.scanned)
      } else {}
    } else {}
    if("saveobj" %in% scan){
      saveobj.scanned <- rk.JS.saveobj(XML.plugin)
      if(!is.null(saveobj.scanned)){
        js[["printout"]] <- paste(js[["printout"]], saveobj.scanned, sep="\n")
      } else {}
    } else {}
    return(js)
  }
  if("js" %in% create & length(js) > 0){
    got.JS.options <- names(js)
    for (this.opt in c("require", "globals", "variables", "preprocess", "calculate", "printout", "doPrintout", "load.silencer")){
      if(!this.opt %in% got.JS.options) {
        js[[this.opt]] <- eval(formals(rk.JS.doc)[[this.opt]])
      } else {}
    }
    if(!"results.header" %in% got.JS.options) {
      js[["results.header"]] <- paste0("\"", name.orig, " results\"")
    } else {}
    js <- js.try.scan(XML.plugin=XML.plugin, scan=scan, js=js, guess.getter=guess.getter)
    JS.code <- rk.JS.doc(
      require=js[["require"]],
      variables=js[["variables"]],
      globals=js[["globals"]],
      results.header=js[["results.header"]],
      preprocess=js[["preprocess"]],
      calculate=js[["calculate"]],
      printout=js[["printout"]],
      doPrintout=js[["doPrintout"]],
      gen.info=gen.info,
      load.silencer=js[["load.silencer"]],
      indent.by=indent.by)
    slot(this.component, "js") <- JS.code
  } else {
    if("js" %in% create){
      js <- js.try.scan(XML.plugin=XML.plugin, scan=scan, js=js, guess.getter=guess.getter)
    } else {}
    slot(this.component, "js") <- rk.JS.doc(variables=js[["variables"]], printout=js[["printout"]])
  }

  ## create plugin.rkh
  if("rkh" %in% create & length(rkh) > 0){
    got.rkh.options <- names(rkh)
    # if settings were defined manually, this overwrites the scan
    if(!"settings" %in% got.rkh.options){
      if("settings" %in% scan){
        this.settings <- rk.rkh.scan(XML.plugin, component=name.orig)
      } else {
        this.settings <- NULL
      }
      if(!is.null(this.settings)){
        rkh[["settings"]] <- rk.rkh.settings(this.settings)
      } else {
        rkh[["settings"]] <- eval(formals(rk.rkh.doc)[["settings"]])
      }
    } else {}
    for (this.opt in c("summary", "usage", "sections", "related", "technical")){
      if(!this.opt %in% got.rkh.options) {
        rkh[[this.opt]] <- eval(formals(rk.rkh.doc)[[this.opt]])
      } else {}
    }
    rkh.doc <- rk.rkh.doc(
      summary=rkh[["summary"]],
      usage=rkh[["usage"]],
      sections=rkh[["sections"]],
      settings=rkh[["settings"]],
      related=rkh[["related"]],
      technical=rkh[["technical"]],
      title=rk.rkh.title(name.orig),
      hints=hints,
      gen.info=gen.info)
    slot(this.component, "rkh") <- rkh.doc
  } else {
    if("rkh" %in% create & "settings" %in% scan){
      this.settings <- rk.rkh.scan(XML.plugin, component=name.orig)
      if(!is.null(this.settings)){
        rkh[["settings"]] <- rk.rkh.settings(this.settings)
      } else {}
    } else {}
    slot(this.component, "rkh") <- rk.rkh.doc(settings=rkh[["settings"]], hints=hints)
  }

  return(this.component)
}
