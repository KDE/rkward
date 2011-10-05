#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.options()

setClass("rk.JS.opt",
	representation=representation(
		option="character",
		ifs="list"
	),
	prototype(
		option=character(),
		ifs=list()
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
