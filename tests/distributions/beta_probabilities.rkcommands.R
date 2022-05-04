local({
## Compute
q <- seq (0, 1, length.out=20)
p <- pbeta (q, shape1=1.0, shape2=1.0, ncp=0.0)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="1.0",
	"non-centrality parameter (ncp)"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
local({
## Compute
q <- seq (0, 1, length.out=20)
p <- pbeta (q, shape1=1.0, shape2=1.0, ncp=0.0, log.p=TRUE, lower.tail=FALSE)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="1.0",
	"non-centrality parameter (ncp)"="0.0",
	"Tail"="Upper tail: P[X > x]"))
rk.results (data.frame ("Quantile"=q, "log (Probability)"=p, check.names=FALSE))
})
local({
## Compute
q <- seq (0, 1, length.out=20)
d <- dbeta (q, shape1=2.0, shape2=1.0, ncp=0.0, log=TRUE)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="2.0",
	"Shape 2"="1.0",
	"non-centrality parameter (ncp)"="0.0"))
rk.results (data.frame ("Quantile"=q, "log (Density)"=d, check.names=FALSE))
})
local({
## Compute
q <- seq (0, 1, length.out=20)
d <- dbeta (q, shape1=1.0, shape2=2.0, ncp=0.0)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="2.0",
	"non-centrality parameter (ncp)"="0.0"))
rk.results (data.frame ("Quantile"=q, "Density"=d, check.names=FALSE))
})
local({
## Compute
p <- -20:0
q <- qbeta (p, shape1=1.0, shape2=2.0, ncp=0.0, log.p=TRUE)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="2.0",
	"non-centrality parameter (ncp)"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("log (Probability)"=p, "Quantile"=q, check.names=FALSE))
})
local({
## Compute
p <- seq (0, 1, length.out=20)
q <- qbeta (p, shape1=1.0, shape2=2.0, ncp=0.0)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="2.0",
	"non-centrality parameter (ncp)"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Probability"=p, "Quantile"=q, check.names=FALSE))
})
local({
## Compute
p <- c (.1, .2, .3)
q <- qbeta (p, shape1=1.0, shape2=2.0, ncp=0.0)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="2.0",
	"non-centrality parameter (ncp)"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Probability"=p, "Quantile"=q, check.names=FALSE))
})
local({
## Compute
q <- 1
p <- pbeta (q, shape1=1.0, shape2=2.0, ncp=0.0)
## Print result
rk.header ("Beta distribution", parameters=list("Shape 1"="1.0",
	"Shape 2"="2.0",
	"non-centrality parameter (ncp)"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
