## R syntax test/demo file
# constructs that are (syntactically) illegal are commented as "syntax error")
# instances of known wrong highlighting (at the time of this writing) are commented as "BUG"
# instances of questionable highlighting, without an obvious desirable bevhior are commented as "CORNER CASE"

## This is a Headline
# This is a Comment

# data types
myBoolean <- c(TRUE, FALSE)
myNa <- NA
myFloat <- 1.234
myInt <- 1l

# quotes:
cat ("normal quote", 'single quoted', "with escapes \" and newline \n' ", 'with escapes and newline 2 \'\n" ')
`backquoted symbol name` <- 1
"backquoted symbol name"        # This will print the string
`backquoted symbol name`        # This will print the value (1)
cat( "\nNo Errors!\n" )

# assignments and operators
myString <- "regular assignment"
"assignment to the right" -> myString2
myString3 <<- "global assignment"
"global assignment to the right" ->> myString4
myString5 <- as.character( 3 + 4 )
myMatrix  <- matrix( 1:6, nrow = 3, ncol = 2 )
myMatrix2 <- myMatrix + 4
myMatrix3 <- t( myMatrix ) %*% myMatrix
myMatrix[ myMatrix < 4 ] <- 0
myMatrix[ myMatrix == 4 ] <- 10
myFloat = 1.234
myBooleanVector <- c( 1, 5 ) %in% c( 1, 2, 3, 4 )
formula(a~b+c)

# subsetting
myClass@mySlot
myList$myItem
myList[["myItem"]]
myList[varHoldingItemName, ]

# named arguments
myList <- list( a = 1, a2 = 2, a2_b = 3, _a3=4 )
print ("hi", quote=(x==1))
x = list(x=1)        # note the different highlighting between '=' used for assignment and for named arugment
x <- list(a <- 1)    # this is syntactically legal (but both '<-'s will creeate local objects; 'a' will not become an argument name)
myList <- list(2a = 2) # illegal argument name
myList <- list(_a = 2) # syntax error: illegal argument name (BUG)
myList <- list(a=1, 
               b=2)    # function call spanning multiple lines

0# Base R pipeline (since R 4.1.0)
x <- rnorm(1000) |> round() |> max()
# CORNER CASE: as of R 4.2.0, "=>" has to be enabled, explicitly
rnorm(1000) |> round() |> max() |> d => seq(from = 1, to = d)

# magrittr pipes (no special highlighting needed, as these are just infix operators)
rnorm(1000) %>% round() %>% max() %>% seq(from = 1, to = .)

if( 3 > 4 ) {
   stop( "Error: 3 is not graeter than 4!!!" )
} else if( 3 == 4 ) {
   stop( "Error: 3 is not equal to 4!!!" )
}

for( i in 2:5 ) {
   x <- i
   print( paste( "No. ", as.character( i + ( 3 + i ) ), sep = "" ) )
   while(x == i) {
     print(paste("Nested", "expression"))
     x <- i+1
   }
}
x in 1:3  # CORNER CASE: "in" is only valid inside for statement

# corner cases and illegal constructs
try ( {
	x <<<- 1                 # syntax error
	x <- * 1                 # syntax error
	x += 1 -= x1 *= x2       # syntax error
	x =* 1 =/ x1 // x2       # syntax error
	x +!= y +-= z +!/ 2      # syntax error
	x |< print()             # syntax error
	x =+ 1; x =- x1; x <-- 1 # These look misleading, but are legal
	1 +-!-++--!!+ 1          # Crazy, but legal

	x <- ~1                 # BUG: Actually, this is legal (if uncommon), but it's marked up as an error

	"%my 1st infix%" <- function (x, y) { x + y }   # create a legal (uncommon) infix
	3 %my 1st infix% 4
	"%my%infix%" <- function (x, y) { x + y }       # create an illegal infix
	3 %my%infix% 4                                  # syntax error
	3 %my%infix%other% 4                            # but could be continued to something syntactically legal
	)                       # syntax error: unexpected closing ')'
	(})                     # syntax error: unexpected closing '}'
	]                       # syntax error: unexpected closing ']' (BUG)
},
silent=FALSE)
