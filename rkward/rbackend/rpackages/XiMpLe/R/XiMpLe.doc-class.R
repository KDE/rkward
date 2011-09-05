#' @include XiMpLe.node-class.R
#' @import methods
#' @export
setClass("XiMpLe.doc",
	representation=representation(
		file="character",
		xml="list",
		dtd="list",
		children="list"
	),
	prototype(
		file=character(),
		xml=list(),
		dtd=list(),
		children=list()
	)
)

setValidity("XiMpLe.doc", function(object){
		obj.xml <- object@xml
		obj.dtd <- object@dtd
		obj.children <- object@children

		obj.xml.names <- names(obj.xml)
		obj.dtd.names <- names(obj.dtd)
		# if there are declarations, check that they all have names
		if(length(obj.xml) > 0){
			if(length(obj.xml) != length(obj.xml.names)){
				stop(simpleError("Invalid object: All xml declarations must have names!"))
			} else {}
		} else {}
		if(length(obj.dtd) > 0){
			if(length(obj.dtd) != length(obj.dtd.names)){
				stop(simpleError("Invalid object: All doctype declarations must have names!"))
			} else {}
		} else {}

		# check content of children
		if(length(obj.children) > 0){
			child.nodes <- sapply(obj.children, function(this.child){inherits(this.child, "XiMpLe.node")})
			if(!all(child.nodes)){
				stop(simpleError("Invalid object: All list elements of children must be of class XiMpLe.node!"))
			} else {}
		} else {}
	return(TRUE)
})
