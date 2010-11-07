/***************************************************************************
                          rkrbackendprotocol  -  description
                             -------------------
    begin                : Thu Nov 04 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkrbackendprotocol_shared.h"

#include "../debug.h"

RCommandProxy::RCommandProxy (const QString &command, int type) {
	RK_TRACE (RBACKEND);

	RCommandProxy::command = command;
	RCommandProxy::type = type;
	id = -1;
	status = 0;
}

RCommandProxy::~RCommandProxy () {
	RK_TRACE (RBACKEND);

	RK_ASSERT ((type & RCommand::Internal) || (getDataType () == RData::NoData));
}
