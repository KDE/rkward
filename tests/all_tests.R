# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

require (rkwardtests)

## add your test suite files, to this vector:
testsuites <- c (
	"rkward_application_tests.R",
	"import_export_plugins.R",
	"item_response_theory.R",
	"analysis_plugins.R",
	"distributions.R",
	"plots.R",
	"data_plugin_tests.R"
)

rktest.makeplugintests (testsuites=testsuites, outfile="make_plugintests.txt")

#rktest.makeplugintests (testsuites="analysis_plugins.R", outfile="make_plugintests.txt", test.id="crosstab_multi")
