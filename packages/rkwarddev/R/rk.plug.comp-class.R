#' @include rk.XML.plugin.R
#' @include rk.JS.doc.R
#' @include rk.rkh.doc.R
#' @export

# this class holds plugin components, i.e. single dialogs, to add to a plugin skeleton
# produced by rk.plugin.component()

setClass("rk.plug.comp",
	representation=representation(
		name="character",
		create="vector",
		xml="XiMpLe.doc",
		js="character",
		rkh="XiMpLe.doc",
		hierarchy="list"
	),
	prototype(
		name=character(),
		create=c(),
		xml=new("XiMpLe.doc"),
		js=character(),
		rkh=new("XiMpLe.doc"),
		hierarchy=list()
	)
)

# setValidity("rk.plug.comp", function(object){
# 	return(TRUE)
# })
