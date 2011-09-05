#' @import methods
#' @export
setClass("XiMpLe.node",
	representation=representation(
		name="character",
		attributes="list",
		children="list",
		value="character"
	),
	prototype(
		name=character(),
		attributes=list(),
		children=list(),
		value=character()
	)
)

setValidity("XiMpLe.node", function(object){
		obj.name <- object@name
		obj.attributes <- object@attributes
		obj.children <- object@children
#		obj.value <- object@value

 		if(!nchar(obj.name) > 0){
			print(str(object))
 			stop(simpleError("Invalid object: A node must have a name!"))
 		} else {}

		obj.attributes.names <- names(obj.attributes)
		# if there are attributes, check that they all have names
		if(length(obj.attributes) > 0){
			if(length(obj.attributes) != length(obj.attributes.names)){
				stop(simpleError("Invalid object: All attributes must have names!"))
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
