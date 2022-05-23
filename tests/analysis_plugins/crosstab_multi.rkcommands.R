local({
## Prepare
data <- data.frame (rk.list(test_table[["A"]],test_table[["B"]],test_table[["C"]],test_table[["D"]]), check.names=FALSE)
## Compute
result <- ftable (data, row.vars=1:min(1, length(data)));
## Print result
rk.header ("Crosstabs (n to n)", parameters=list("Variables"=paste(names(data), collapse=", ")))

rk.print (result)
})
