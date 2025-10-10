symb.first
# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
Symbol00 + 1
Symbol01 <- Symbol2(Symbol03="String1", function(Symbol04) {
	for (Symbol05 in Symbol06) {
		Symbol07
		Symbol08$Symbol09[1:10]
		Symbol10(Symbol11=Symbol12, Symbol13 + Symbol14)
		Symbol15; Symbol16 <- Symbol17 + 1
		Symbol18
	}
})
Symbol19

# Comment 4
Symbol.x <- Symbol.y + Symbol.z +
            Symbol.y |>

            cont_after_newline() |>
            # a comment

            # a comment
            cont_after_comments()

FunctionList <- list(
	Argname=function() { statement },
	makeFunction({ aaa; bbb; ccc; },
	{
		nest1
		{
			nest2(n2_inner + 1)
			nest3()
			nest4
		}
		nest5
	}, ddd, eee <- fff(ggg + hhh) + iii, jjj)
)

symb.last # comment last, no newline at EOF
