/*
rksignalsupport - This file is part of the RKWard project. Created: Thu Nov 22 2007
SPDX-FileCopyrightText: 2007-2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSIGNALSUPPORT_H
#define RKSIGNALSUPPORT_H

namespace RKSignalSupport {
	void saveDefaultSignalHandlers ();
	void installSignalProxies ();

	void installSigIntAndUsrHandlers (void (*handler) (void));
	void callOldSigIntHandler ();
};

#endif
