## This is a Headline
# This is a Comment

myBoolean <- TRUE
myNa <- NA

myString <- "This is a String"
"String 2" -> myString2
myString3 <<- "global assignment"
myString4 <- as.character( 3 + 4 )

myMatrix  <- matrix( 1:6, nrow = 3, ncol = 2 )
myMatrix2 <- myMatrix + 4
myMatrix3 <- t( myMatrix ) %*% myMatrix
myMatrix[ myMatrix < 4 ] <- 0
myMatrix[ myMatrix == 4 ] <- 10
myFloat = 1.234
myBooleanVector <- c( 1, 5 ) %in% c( 1, 2, 3, 4 )

if( 3 > 4 ) {
   stop( "Error: 3 is not graeter than 4!!!" )
} else if( 3 == 4 ) {
   stop( "Error: 3 is not equal to 4!!!" )
}

for( i in 2:5 ) {
   print( paste( "No. ", as.character( i + ( 3 + i ) ), sep = "" ) )
}

try ( {
	x <<- 1
	x <<<- 1		# syntax error
	x += 1 -= x1 *= x2	# syntax error
	x =+ 1 =- x1 =* x2	# syntax error
	"%my 1st infix%" <- function (x, y) { x + y }		# legal infix
	3 %my 1st infix% 4
	"%my%infix%" <- function (x, y) { x + y }		# illegal infix
	3 %my%infix% 4						# syntax error
	)		# unexpected closing ')'
	(})		# unexpected closing '}'
},
silent=FALSE)

myList <- list( a = 1, a2 = 2, a2_b = 3 )

x <- 1
print ("hi", quote=(x==1))

# quotes:
cat ("normal quote", 'single quoted', "with escapes \" ' ", 'with escapes 2 \' " ')
`backquoted symbol name` <- 1
"backquoted symbol name"	# This will print the string
`backquoted symbol name`	# This will print the value (1)

cat( "\nNo Errors!\n" )
