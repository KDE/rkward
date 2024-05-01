/*
rkrapi - This file is part of RKWard (https://rkward.kde.org). Created: Wed May 01 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

/** @file rkrapi.h

Encapsulate R API calls for abstraction over dl_open. WIP

*/

#define R_INTERFACE_PTRS 1
// for R_CStackStart/Limit
#define CSTACK_DEFNS 1
// keep R from defining tons of aliases
#define R_NO_REMAP 1
// What the...? "Conflicting definitions" between stdint.h and Rinterface.h despite the #if in Rinterface.h
#define uintptr_t uintptr_t

// needed to detect CHARSXP encoding
#define IS_UTF8(x) (Rf_getCharCE(x) == CE_UTF8)
#define IS_LATIN1(x) (Rf_getCharCE(x) == CE_LATIN1)
#ifdef TRUE
#	undef TRUE
#endif
#ifdef FALSE
#	undef FALSE
#endif

// Common includes
#include <Rdefines.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Callbacks.h>
#include <R.h>
#include <Rversion.h>
#include <Rinternals.h>
#include <R_ext/Parse.h>
#include <Rembedded.h>
#include <R_ext/Riconv.h>
#define R_USE_PROTOTPYES 1
#include <R_ext/GraphicsEngine.h>

// rcolor typedef added in R 3.0.0
#ifndef rcolor
#define rcolor unsigned int
#endif

// The following needed only on Windows
#if defined(_WIN32) || defined(_MSC_VER) || defined(Win32)  // Note: no easy access to Q_OS_WIN at this point
#	include <R_ext/RStartup.h>
#	include <R_ext/Utils.h>
#	include <R_ext/libextern.h>
#else
// The following needed only outside of Windows, and Rinterface.h is not even available on Windows
#	include <Rinterface.h>
#	include <R_ext/eventloop.h>
#endif
