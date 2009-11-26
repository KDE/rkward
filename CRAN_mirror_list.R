#!/usr/bin/Rscript
# Read the mirror list file from MIRROR_LIST
# and create two lists suitable for QStringList.
# These two strings are used in rkward/settings/rksettingsmoduler.cpp

#MIRROR_LIST <- '/usr/share/R/doc/CRAN_mirrors.csv'

removeCityFromCountry <- function (x) {
	sub (' *$', '', sub (paste ('(',x[2],')', sep=''), '', x[1], fixed=TRUE))
}

#mirror.list <- read.csv(MIRROR_LIST, stringsAsFactors=FALSE)
mirror.list <- getCRANmirrors()

mirror.names <- paste('"', 
	paste (apply(mirror.list[,c(1,3)], 1, 'removeCityFromCountry'), 
		mirror.list$City, 
		unlist(lapply(strsplit(mirror.list$Host, split=','), function(x) {x[1]})), 
	sep=' - '), '"', sep='')

cat (
 'Host details list:\n', # cran_mirrors_list
 paste (c(' << "Ask everytime"', mirror.names), collapse=' << '), # Host details
 '\nURL list:\n', # cran_url_list
 paste (c(' << "@CRAN@"', paste('"', mirror.list$URL, '"', sep='')), collapse=' << '), # URL
 '\n') 


## If ever a separate .h file is used:
# In fact could combine the two lists into one (i = i+2)
#message (paste (c(' << "Ask everytime"', mirror.names), collapse='\n\t << ')) # Host details
#message (paste (c(' << "@CRAN@"',paste('"', mirror.list$URL, '"', sep='')), collapse='\n\t << ')) # URL
