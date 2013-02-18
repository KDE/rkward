#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.vars()

setClass("rk.JS.var",
	representation=representation(
		JS.var="character",
		XML.var="character",
		prefix="character",
		modifiers="list",
		default="logical",
		join="character",
		vars="list",
		getter="character"
	),
	prototype(
		JS.var=character(),
		XML.var=character(),
		prefix=character(),
		modifiers=list(),
		default=FALSE,
		join="",
		vars=list(),
		getter="getValue" # for compatibility with earlier releases
	)
)

setValidity("rk.JS.var", function(object){
		# vars in this object must be of the same class
		sapply(object@vars, function(this.var){
			if(!inherits(this.var, "rk.JS.var")){
				stop(simpleError("Slot 'vars' can only have a list of elements of class 'rk.JS.var'!"))
			} else {}
		})
	return(TRUE)
})
