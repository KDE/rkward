local({
## Compute
result <- (ptukey (q = c (0.95), nmeans = 2, df = 5, nranges = 1, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Studentized Range probability", list ("Vector of quantiles", "c (0.95)", "Sample size for range", "2", "Degrees of freedom for s", "5", "Number of groups whose maximum range is considered", "1", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Studentized Range probabilities")
})
