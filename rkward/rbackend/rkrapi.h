/*
rkrapi - This file is part of RKWard (https://rkward.kde.org). Created: Wed May 01 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

/** @file rkrapi.h

Encapsulate R API calls for abstraction over dlopen().

Problem description:

1. The rkward.rbackend executable needs R functions (obviously) and Qt functions (for interfacing with the frontend).
2. Both will pull in additional dependencies, some of which are the same, but some of which may be conflicting with each other.
2a. Conflicts are especially to be expected in "bundle" cases, such as AppImage, when trying to use an rkward.rbackend in the bundle
    with an installation of R on the system. Here, the AppImage, and the system may very likely sport different versions of the same libraries.
(2aa. Why not just rely on a bundled R inside the AppImage? Because it will essentially be unable to build packages, again due to trying to
      mix libraries from inside the AppImage with those outside.)
2b. Another - limited - scenario is the (currently non-maintained) qtbase R package, which expects to be able to create a QApplication singleton.
    Here, there is no clash of conflicting libraries, but a clash with two users of the library accessing the same singletons invalidly.
3. The problem is aggravated by the fact that R may load essentially abitrary further libraries from packages. In the 2a) case, we want to load
   all those from system locations, while the AppImage location is only for the Qt functions in rkward.rbackend.
4. R itself (time of writing: R 4.4, but unchanged in this respect since ages) tries to ensure correct lib search path by setting LD_LIBRARY_PATH to:
   /Rs/own/libdir:/Rs/idea/of/where/system/libs/might/be/at:/Rs/java/libdir:/anything/previously/in/LD_LIBRARY_PATH
   Now this puts several directories in front of the search path which may well contain libraries conflicting with the Qt libs we need.
   E.g. it may contain /usr/lib/, while we'd hope to load Qt libs from inside an Appimage.
5. Note that LD_LIBRARY_PATH overrides RUNPATH. RPATH is deprecated, and while it will probably be around for a long time to come, it is not so easy
   to set. In fact, linuxdeployqt (the tool used to create Appimages in KDE infastructure) foolishly overwrites any RPATH we may set, manually.

So how do we get two sets of libraries to load from two different search paths, without getting into the way of each other? Keeping them compartimentalised is
done by loading them using dlopen() with the RTLD_LOCAL flag set. Now, dlopen() is "easy" enough for dealing with a plain C library like libR.so. (That's essentially
what is done in this file). We shove everything else into another shared lib (librkward.rbackend.lib.so), so we can similarily dlopen it with RTLD_LOCAL. This lib
exports a single (C) entry-point ("do_main()"), and can simply be handed a pointer to the libR.so, from where to resolve all required symbols.

As for loading the two from different search paths, having already compartimentalised our two sets of libs, we somehow need to make sure, the two sets are loaded
with different effective search paths. RUNPATH and even RPATH won't help us here (see above), and changes done to LD_LIBRARY_PATH do not take
effect during the lifetime of an executable.

1. Frontend creates a temporary dir with a symlink pointing to the path of the qt libs we want to load
2. Frontend sets up environment, removing any LD_LIBRARY_PATH pointing to inside the AppImage, and sets some further envs vars (see below). Calls rkward.rbackend
2. rkward.rbackend is just a tiny binary that dlopen()s two further things (each with RTLD_LOCAL):
3. libR.so - this is easy enough to get from the right place, as it is not in any default path, and R CMD has added the appropriate stuff to LD_LIBRARY_PATH
4. For loading librkward.rbackend.lib.so, we need a more complex approach:
4a. First, we prepend the name of the symlink from step 1 to LD_LIBRARY_PATH as a *relative* path (relative to the temporary dir).
4b. We execv() rkward.rbackend, again, for this to take effect.
4c. We cd to the temporary dir: From this dir - and *only* from this dir - our addition to LD_LIBRARY_PATH will point to our desired path of qt libs
4d. We dlopen() librkward.rbackend.lib.so
4e. We cd back to where we were before 4c.
5. do_main() is resolved from librkward.rbackend.lib.so, and called with a handle to libR.so to resolve all required R symbols.

Now it may stand out that most of this is really just about Linux, and in particular about AppImage. Is it even worth it?
- On Mac, while libs-references are usually absolute, this same fact prevents us from bundling R, which means we need to rely on libs outside of our
  bundle. Signing gets throun by this. dlopen()ing, instead, helps (I think).
- On all platforms, this allows us more flexibility to support more than just one version of R. E.g. when running with an R version older than that
  used a compile-time, we can simply skip setting up certain graphics engine callbacks. The same would not be possible using regular linking.

---

All this written, I don't quite trust this scheme to be perfect, yet, and so one design goal is to keep the old (link all shared libs to rkward.rbackend, directly)
available as a config option.

*/

#ifndef RKRAPI_H
#define RKRAPI_H

#include <QObject>
#include <QMetaProperty>
#if defined(Q_OS_WIN)
#define Win32   // R assumes this on Windows
#endif

#define R_INTERFACE_PTRS 1
// for R_CStackStart/Limit
#define CSTACK_DEFNS 1
// keep R from defining tons of aliases
#define R_NO_REMAP 1
// What the...? "Conflicting definitions" between stdint.h and Rinterface.h despite the #if in Rinterface.h
#define uintptr_t uintptr_t

// needed to detect CHARSXP encoding
#define IS_UTF8(x) (RFn::Rf_getCharCE(x) == CE_UTF8)
#define IS_LATIN1(x) (RFn::Rf_getCharCE(x) == CE_LATIN1)
#ifdef TRUE
#	undef TRUE
#endif
#ifdef FALSE
#	undef FALSE
#endif

#include <Rversion.h>

#if defined(Win32) && (R_VERSION >= R_Version(4, 3, 0))
// We don't use Complex.h ourselves, but it is included from Rinternals.h and comes with a non-standard definition
// that trips up MSVC. Fortunately, we can disable that via a define.
#define R_LEGACY_RCOMPLEX
#include <R_ext/Complex.h>
#undef R_LEGACY_RCOMPLEX
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
#if defined(Win32)
#	include <R_ext/RStartup.h>
#	include <R_ext/Utils.h>
#	include <R_ext/libextern.h>
extern "C" {
	// why oh why isn't Rinterface.h available on Windows?
	LibExtern void* R_GlobalContext;
	LibExtern uintptr_t R_CStackLimit;
	LibExtern void R_SaveGlobalEnvToFile(char*);
}
#else
// The following needed only outside of Windows, and Rinterface.h is not even available on Windows
#	include <Rinterface.h>
#	include <R_ext/eventloop.h>
#endif

// some functions we need that are not declared
extern "C" void run_Rmainloop(void);

#ifdef RK_DLOPEN_LIBRSO
// Using Qt Meta-Property system for introspection, in order to automate the dlsym-calls.
// Only the _set ## X function is actually used (for initialization). The _get ## X functions are just to keep the MOC happy.
// The actual access happens directly via the member (X)
#define IMPORT_R_API(X) Q_PROPERTY(void* X READ _get ## X WRITE _set ## X) \
                        public: static inline decltype(::X) *X = nullptr; \
                        void _set ## X (void* v) { X = (decltype(X)) v; } \
                        void* _get ## X () { return (void*) X; }
#define ROb(X) *(RFn::X)
#else
// For classic dynamic linking, set up the required members simply as aliases to the real thing
#define IMPORT_R_API(X) static constexpr decltype(::X) &X = ::X
#define ROb(X) (RFn::X)
#endif

class RFn : public QObject {
Q_OBJECT
public:
// TODO: This list should be generated, automatically, at compile time
IMPORT_R_API(CDR);
IMPORT_R_API(CDDR);
IMPORT_R_API(CAR);
IMPORT_R_API(SETCAR);
IMPORT_R_API(GEaddDevice2);
IMPORT_R_API(GEcreateDevDesc);
IMPORT_R_API(GEgetDevice);
IMPORT_R_API(GEplayDisplayList);
IMPORT_R_API(R_CHAR);
IMPORT_R_API(FORMALS);
IMPORT_R_API(INTEGER);
IMPORT_R_API(LOGICAL);
IMPORT_R_API(LENGTH);
IMPORT_R_API(PRCODE);
IMPORT_R_API(PRENV);
IMPORT_R_API(PRINTNAME);
IMPORT_R_API(PRVALUE);
IMPORT_R_API(REAL);
IMPORT_R_API(R_CheckDeviceAvailable);
IMPORT_R_API(R_CheckStack);
IMPORT_R_API(R_CheckUserInterrupt);
IMPORT_R_API(R_CStackLimit);
IMPORT_R_API(R_CleanTempDir);
IMPORT_R_API(R_ClearExternalPtr);
IMPORT_R_API(R_ExternalPtrTag);
IMPORT_R_API(R_ExternalPtrAddr);
IMPORT_R_API(R_GE_getVersion);
#if R_VERSION >= R_Version(4, 1, 0)
IMPORT_R_API(R_GE_linearGradientColour);
IMPORT_R_API(R_GE_linearGradientExtend);
IMPORT_R_API(R_GE_linearGradientNumStops);
IMPORT_R_API(R_GE_linearGradientStop);
IMPORT_R_API(R_GE_linearGradientX1);
IMPORT_R_API(R_GE_linearGradientX2);
IMPORT_R_API(R_GE_linearGradientY1);
IMPORT_R_API(R_GE_linearGradientY2);
IMPORT_R_API(R_GE_patternType);
IMPORT_R_API(R_GE_radialGradientColour);
IMPORT_R_API(R_GE_radialGradientCX1);
IMPORT_R_API(R_GE_radialGradientCX2);
IMPORT_R_API(R_GE_radialGradientCY1);
IMPORT_R_API(R_GE_radialGradientCY2);
IMPORT_R_API(R_GE_radialGradientExtend);
IMPORT_R_API(R_GE_radialGradientNumStops);
IMPORT_R_API(R_GE_radialGradientR1);
IMPORT_R_API(R_GE_radialGradientR2);
IMPORT_R_API(R_GE_radialGradientStop);
IMPORT_R_API(R_GE_tilingPatternExtend);
IMPORT_R_API(R_GE_tilingPatternFunction);
IMPORT_R_API(R_GE_tilingPatternHeight);
IMPORT_R_API(R_GE_tilingPatternWidth);
IMPORT_R_API(R_GE_tilingPatternX);
IMPORT_R_API(R_GE_tilingPatternY);
#endif
IMPORT_R_API(R_GE_str2col);
IMPORT_R_API(R_MakeExternalPtr);
IMPORT_R_API(R_ParseVector);
IMPORT_R_API(R_RunExitFinalizers);
IMPORT_R_API(R_SaveGlobalEnvToFile);
IMPORT_R_API(R_ToplevelExec);
IMPORT_R_API(R_chk_calloc);
IMPORT_R_API(R_chk_free);
IMPORT_R_API(R_dot_Last);
IMPORT_R_API(R_getEmbeddingDllInfo);
IMPORT_R_API(R_lsInternal3);
IMPORT_R_API(R_registerRoutines);
IMPORT_R_API(R_removeVarFromFrame);
IMPORT_R_API(R_tryEval);
IMPORT_R_API(Rf_GetOption);
IMPORT_R_API(Rf_GetOption1);
IMPORT_R_API(Rf_KillAllDevices);
IMPORT_R_API(Rf_ScalarInteger);
IMPORT_R_API(Rf_addTaskCallback);
IMPORT_R_API(Rf_allocList);
IMPORT_R_API(Rf_allocVector);
IMPORT_R_API(Rf_asChar);
IMPORT_R_API(Rf_asInteger);
IMPORT_R_API(Rf_asLogical);
IMPORT_R_API(Rf_asReal);
IMPORT_R_API(Rf_coerceVector);
IMPORT_R_API(Rf_curDevice);
IMPORT_R_API(Rf_defineVar);
IMPORT_R_API(Rf_doesIdle);
IMPORT_R_API(Rf_doIdle);
IMPORT_R_API(Rf_doKeybd);
IMPORT_R_API(Rf_doMouseEvent);
IMPORT_R_API(Rf_error);
IMPORT_R_API(Rf_eval);
IMPORT_R_API(Rf_findFun);
IMPORT_R_API(Rf_findVar);
IMPORT_R_API(Rf_findVarInFrame);
IMPORT_R_API(Rf_getAttrib);
IMPORT_R_API(Rf_getCharCE);
IMPORT_R_API(Rf_initialize_R);
IMPORT_R_API(Rf_install);
IMPORT_R_API(Rf_installChar);
IMPORT_R_API(Rf_isEnvironment);
IMPORT_R_API(Rf_isList);
IMPORT_R_API(R_IsNA);
IMPORT_R_API(R_IsNaN);
IMPORT_R_API(Rf_isNewList);
IMPORT_R_API(Rf_isNull);
IMPORT_R_API(Rf_isNumeric);
IMPORT_R_API(Rf_isPairList);
IMPORT_R_API(Rf_isPrimitive);
IMPORT_R_API(Rf_isS4);
IMPORT_R_API(Rf_isString);
IMPORT_R_API(Rf_isVectorList);
IMPORT_R_API(Rf_lang1);
IMPORT_R_API(Rf_length);
IMPORT_R_API(Rf_list3);
IMPORT_R_API(Rf_mkChar);
IMPORT_R_API(Rf_mkCharCE);
IMPORT_R_API(Rf_onintr);
IMPORT_R_API(Rf_protect);
IMPORT_R_API(R_Reprotect);
IMPORT_R_API(R_ProtectWithIndex);
IMPORT_R_API(Rf_setAttrib);
IMPORT_R_API(Rf_unprotect);
IMPORT_R_API(Rf_warning);
IMPORT_R_API(Riconv);
IMPORT_R_API(Riconv_close);
IMPORT_R_API(Riconv_open);
IMPORT_R_API(SET_TAG);
IMPORT_R_API(SET_TYPEOF);
IMPORT_R_API(STRING_ELT);
IMPORT_R_API(SET_STRING_ELT);
IMPORT_R_API(SET_PRENV);
IMPORT_R_API(SET_PRVALUE);
IMPORT_R_API(TYPEOF);
IMPORT_R_API(VECTOR_ELT);
IMPORT_R_API(run_Rmainloop);
IMPORT_R_API(setup_Rmainloop);

// data NOTE / TODO: Some of these are essentially const, should be treated as such
IMPORT_R_API(R_BaseEnv);
IMPORT_R_API(R_ClassSymbol);
IMPORT_R_API(R_DimSymbol);
IMPORT_R_API(R_DirtyImage);
IMPORT_R_API(R_EmptyEnv);
IMPORT_R_API(R_GlobalContext);
IMPORT_R_API(R_GlobalEnv);
IMPORT_R_API(R_NaInt);
IMPORT_R_API(R_NaReal);
IMPORT_R_API(R_NaString);
IMPORT_R_API(R_NamesSymbol);
IMPORT_R_API(R_NilValue);
IMPORT_R_API(R_interrupts_pending);
IMPORT_R_API(R_interrupts_suspended);
IMPORT_R_API(R_UnboundValue);

#ifndef Q_OS_WIN
IMPORT_R_API(ptr_R_Busy);
IMPORT_R_API(ptr_R_ChooseFile);
IMPORT_R_API(ptr_R_CleanUp);
IMPORT_R_API(ptr_R_ClearerrConsole);
IMPORT_R_API(ptr_R_EditFile);
IMPORT_R_API(ptr_R_FlushConsole);
IMPORT_R_API(ptr_R_ReadConsole);
IMPORT_R_API(ptr_R_ResetConsole);
IMPORT_R_API(ptr_R_ShowFiles);
IMPORT_R_API(ptr_R_ShowMessage);
IMPORT_R_API(ptr_R_Suicide);
IMPORT_R_API(ptr_R_WriteConsole);
IMPORT_R_API(ptr_R_WriteConsoleEx);

IMPORT_R_API(R_InputHandlers);
IMPORT_R_API(R_PolledEvents);
IMPORT_R_API(R_checkActivityEx);
IMPORT_R_API(R_runHandlers);
IMPORT_R_API(addInputHandler);

// data
IMPORT_R_API(R_wait_usec);
IMPORT_R_API(R_CStackStart);
IMPORT_R_API(R_Interactive);
IMPORT_R_API(R_Consolefile);
IMPORT_R_API(R_Outputfile);
#else
IMPORT_R_API(R_ProcessEvents);
IMPORT_R_API(R_DefParams);
IMPORT_R_API(R_SetParams);
IMPORT_R_API(R_setStartTime);
IMPORT_R_API(R_set_command_line_arguments);
IMPORT_R_API(getRUser);
IMPORT_R_API(get_R_HOME);
IMPORT_R_API(UserBreak);
#endif

#if R_VERSION >= R_Version(4, 2, 0)
IMPORT_R_API(R_GE_clipPathFillRule);
IMPORT_R_API(R_GE_maskType);
#endif

public:
	static void init(void* libr_dll_handle, void* (*dlsym_fun)(void*, const char*));
};

#endif
