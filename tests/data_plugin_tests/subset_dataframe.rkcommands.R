local({
## Compute
	sset.result <- subset(
		sleep,
		((extra >= 0) & (extra < 3)) & (group == 1),
		select=c (extra, ID)
	)

## Print result
.GlobalEnv$sset.result <- sset.result
})
