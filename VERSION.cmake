# the IF condition enables manual setting of the version number string
# which allows for different versioning needed for MacPorts
IF(NOT RKVERSION_NUMBER)
	SET(RKVERSION_NUMBER 0.5.7z+0.5.8+devel1)
ENDIF(NOT RKVERSION_NUMBER)
