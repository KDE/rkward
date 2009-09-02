local({
## Prepare
data <- data.frame (test_table[["A"]],test_table[["B"]],test_table[["C"]],test_table[["D"]], check.names=FALSE)
datadescription <- paste (rk.get.description (test_table[["A"]],test_table[["B"]],test_table[["C"]],test_table[["D"]]), collapse=", ");
## Compute
result <- ftable (data);
## Print result
rk.header ("Crosstabs (n to n)", parameters=list ("Variables"=datadescription))

rk.print (result)
})
.rk.rerun.plugin.link(plugin="rkward::crosstab_multi", settings="exclude_nas.state=1\nx.available=test_table[[\\\"A\\\"]]\\ntest_table[[\\\"B\\\"]]\\ntest_table[[\\\"C\\\"]]\\ntest_table[[\\\"D\\\"]]", label="Run again")
.rk.make.hr()
