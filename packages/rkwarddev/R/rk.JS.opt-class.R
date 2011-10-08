#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.options()

setClass("rk.JS.opt",
	representation=representation(
		var.name="character",
		opt.name="character",
		collapse="character",
		ifs="list",
		array="logical",
		funct="character"
	),
	prototype(
		var.name=character(),
		opt.name=character(),
		collapse=character(),
		ifs=list(),
		array=NULL,
		funct=character()
	)
)

setValidity("rk.JS.opt", function(object){
	sapply(object@ifs, function(thisIf){
		if(!inherits(thisIf, "rk.JS.ite")){
			stop(simpleError("All option rules in rk.JS.opt objects must be of class rk.JS.ite!"))
		}
	})
	return(TRUE)
})
