# gather debug information.
# Note: subsequent browser() calls should be suppressed while inside this function!
.rk.callstack.info <- function () {
	nframes <- sys.nframe() - 1	# strip this function call
	calls <- character (0)
	funs <- character (0)
	envs <- character (0)
	locals <- character (0)

	if (nframes > 0) {
		for (i in 1:nframes) {
			calls[i] <- as.character (try (paste (deparse (sys.call (i)), collapse="\n")), silent=TRUE)
			funs[i] <- as.character (try (paste (deparse (sys.function (i), control="all"), collapse="\n"), silent=TRUE))
			envs[i] <- as.character (try (capture.output (print (environment (sys.function (i)))), silent=TRUE))
			locals[i] <- as.character (try (paste (ls (sys.frame (i), all.names=TRUE), collapse="\n"), silent=TRUE))
		}
	}
	list ("calls"=calls, "functions"=funs, "environments"=envs, "locals"=locals)
}
