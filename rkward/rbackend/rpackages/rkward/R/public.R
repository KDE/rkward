"rk.get.label" <- function (x) {
	as.vector (attr (x, ".rk.meta")[row.names (attr (x, ".rk.meta")) == "label",1])
}
