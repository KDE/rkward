/***************************************************************************
                          rkgraphicsdevice_protocol_shared  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier 
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKGRAPHICSDEVICE_PROTOCOL_SHARED_H
#define RKGRAPHICSDEVICE_PROTOCOL_SHARED_H

/** @page RKGraphicsDeviceProtocol
 * 
 * The key feature of the RKWard Graphics Device is that it serializes all drawing operations, so they
 * can be sent to a separate process (the frontend) or even computer (well, not yet). Some notes on the protocol:
 * 
 * This is not the same protocol as used for other communication between frontend and backend, and not even
 * the same connection. The key idea behind this protocol here, is that it should have very low overhead,
 * even when sending many @em small requests.
 * 
 * All communication is initiated from the backend. The backend sends a request, starting with the size of the
 * request in bytes (quint32), then an opcode (quint8), then the device number (quint8), then all applicable parameters.
 * Most requests are asynchronous, but a few await a reply from the frontend.
 * 
 * At any time, there can only be one request waiting for a reply, and the request waiting for a reply is always the most
 * recent one. This makes the protocol very simple.
 * 
 * If the frontend has spontaneous need for communication, it will have to use some separate channel.
 *
 * How do we handle cancellation of interactive ops (e.g. locator()) from the backend? If an interrupt is pending
 * in the backend, _while waiting for the reply_, we push an RKD_Cancel request down the line. This tells the frontend to
 * send a reply to the last request ASAP (if the frontend has already sent the reply, it will ignore the RKD_Cancel). From
 * there, we simply process the reply as usual, and leave it to R to actually do the interrupt. If the frontend takes more than
 * fives seconds to respond at this point, the connection will be killed.
 * 
 */

/** This enum simply repeats R's line end definitions. It is used to ensure compatibility, without the need to include
 * any R headers in the frontend. */
enum RKLineEndStyles {
	RoundLineCap = 1,
	ButtLineCap = 2,
	SquareLineCap = 3
};

/** This enum simply repeats R's line join definitions. It is used to ensure compatibility, without the need to include
 * any R headers in the frontend. */
enum RKLineJoinStyles {
	RoundJoin = 1,
	MitreJoin = 2,
	BevelJoin = 3
};

enum RKDFillType {
	ColorFill,
	PatternFill
};

enum RKDPatternType {
	LinearPattern,
	RadialPattern,
	TilingPattern,
	UnknonwnPattern
};

enum RKDGradientExtend {
	GradientExtendNone,
	GradientExtendPad,
	GradientExtendReflect,
	GradientExtendRepeat
};

enum RKDFillRule {
	NonZeroWindingRule,
	EvenOddRule
};

enum RKDOpcodes {
	// NOTE: the only point of the assigned int values is to ease debugging in case of trouble
	// Asynchronous operations
	RKDCreate               = 0,
	RKDCircle,
	RKDLine,
	RKDPolygon,
	RKDPolyline,
	RKDPath,               // 5
	RKDRect,
	RKDTextUTF8,
	RKDNewPage,
	RKDStartGettingEvents,
	RKDActivate,           // 10
	RKDDeActivate,
	RKDClip,
	RKDMode,
	RKDRaster,
	RKDSetSize,            // 15
	RKDStopGettingEvents,
	RKDReleasePattern,
	RKDStartRecordTilingPattern,      // part of setPattern in R
	RKDReleaseClipPath,
	RKDStartRecordClipPath,//20
	RKDReleaseMask,
	RKDStartRecordMask,

	// Synchronous operations
	RKDFetchNextEvent      = 100,
	RKDStrWidthUTF8,
	RKDMetricInfo,
	RKDLocator,
	RKDNewPageConfirm,
	RKDCapture,           // 105
	RKDQueryResolution,
	RKDGetSize,
	RKDSetPattern,
	RKDEndRecordTilingPattern,       // part of setPattern in R
	RKDSetClipPath,       // 110
	RKDEndRecordClipPath,
	RKDSetMask,
	RKDEndRecordMask,
	RKDClose,

	// Protocol operations
	RKDCancel              = 200
};

enum RKDEventCodes {
	RKDMouseUp = 0,
	RKDMouseDown = 1,
	RKDMouseMove = 2,
	RKDKeyPress = 3,
	RKDNothing = 4,
	RKDFrontendCancel = 5,

// Mouse buttons, or-able, identical to the corresponding R defines. Note: x1 and x2 buttons are not handled by R
	RKDMouseLeftButton = 1,
	RKDMouseMiddleButton = 2,
	RKDMouseRightButton = 4
//	RKDMouseX1Button = 8,
//	RKDMouseX2Button = 16
};

/** Common problem is that we need to map an R parameter enum onto the corresponding Qt parameter enum, e.g. line ending style, etc.
 * While in most cases there _is_ a direct correspondence, the underlying int values cannot be assumed to be the same (and in many cases differ).
 * Thus we need a lot of code along the lines "if(value = r_enum_value) return qt_enum_value;".
 *
 * As a further complication, we cannot easily include R headers in frontend code, and not easily (arbitray) Qt headers in backend code, thus the
 * above pseudo code will lack either the Qt or the R defintion. At some point we need to map to a value we set ourselves, either a third, rk-specific, enum,
 * or a naked int.
 *
 * To make this readable and easy to handle, the MapEnum() macro helps with the mapping. Since only _either_ the left hand side (R) or the right hand side (Qt)
 * is actually expanded, it allows us to write a readable mapping that can be included in both frontend and backend code.
 *
 * We still need to provide an interim value (naked int). By convention this uses the same value as the Qt enum, and this is checked by a static_assert, helping
 * to catch conceivable mistakes at compile time.
 */
#if defined(RKD_BACKEND_CODE)
#define MapEnum(Rval,Ival,Qval) case Rval: return Ival;
#define MapDefault(Ival,Qval) return Ival;
#else
#define MapEnum(Rval,Ival,Qval) case Ival: static_assert(Ival == (int) Qval, "Enum mismatch"); return Qval;
#define MapDefault(Ival,Qval) return Qval;
#define RKD_RGE_VERSION 99999
#endif

#if RKD_RGE_VERSION >= 15
static int mapCompostionModeEnum(int from) {
	switch(from) {
		MapEnum(R_GE_compositeClear, 2, QPainter::CompositionMode_Clear);
		MapEnum(R_GE_compositeSource, 3, QPainter::CompositionMode_Source);
		MapEnum(R_GE_compositeOver, 0, QPainter::CompositionMode_SourceOver);
		MapEnum(R_GE_compositeIn, 5, QPainter::CompositionMode_SourceIn);
		MapEnum(R_GE_compositeOut, 7, QPainter::CompositionMode_SourceOut);
		MapEnum(R_GE_compositeAtop, 9, QPainter::CompositionMode_SourceAtop);
		MapEnum(R_GE_compositeDest, 4, QPainter::CompositionMode_Destination);
		MapEnum(R_GE_compositeDestOver, 1, QPainter::CompositionMode_DestinationOver);
		MapEnum(R_GE_compositeDestIn, 6, QPainter::CompositionMode_DestinationIn);
		MapEnum(R_GE_compositeDestOut, 8, QPainter::CompositionMode_DestinationOut);
		MapEnum(R_GE_compositeDestAtop, 10, QPainter::CompositionMode_DestinationAtop);
		MapEnum(R_GE_compositeXor, 11, QPainter::CompositionMode_Xor);
		MapEnum(R_GE_compositeAdd, 12, QPainter::CompositionMode_Plus);
		MapEnum(R_GE_compositeMultiply, 13, QPainter::CompositionMode_Multiply);
		MapEnum(R_GE_compositeScreen, 14, QPainter::CompositionMode_Screen);
		MapEnum(R_GE_compositeOverlay, 15, QPainter::CompositionMode_Overlay);
		MapEnum(R_GE_compositeDarken, 16, QPainter::CompositionMode_Darken);
		MapEnum(R_GE_compositeLighten, 17, QPainter::CompositionMode_Lighten);
		MapEnum(R_GE_compositeColorDodge, 18, QPainter::CompositionMode_ColorDodge);
		MapEnum(R_GE_compositeColorBurn, 19, QPainter::CompositionMode_ColorBurn);
		MapEnum(R_GE_compositeHardLight, 20, QPainter::CompositionMode_HardLight);
		MapEnum(R_GE_compositeSoftLight, 21, QPainter::CompositionMode_SoftLight);
		MapEnum(R_GE_compositeDifference, 22, QPainter::CompositionMode_Difference);
		MapEnum(R_GE_compositeExclusion, 23, QPainter::CompositionMode_Exclusion);
// Unsupported in Qt:
// MapEnum(R_GE_compositeSaturate, xx, yy)
	}
	MapDefault(0, QPainter::CompositionMode_SourceOver);
}
#endif

#endif
