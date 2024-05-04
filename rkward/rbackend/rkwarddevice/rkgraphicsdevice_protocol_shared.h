/*
rkgraphicsdevice_protocol_shared - This file is part of the RKWard project. Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

enum RKDCachedResourceType {
	RKDPattern,
	RKDClipPath,
	RKDMask,
	RKDGroup
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
	RKDReleaseCachedResource,
	RKDStartRecordTilingPattern,      // part of setPattern in R
	RKDStartRecordClipPath,
	RKDStartRecordMask,    // 20
	RKDFillStrokePathBegin,
	RKDFillStrokePathEnd,
	RKDDefineGroupBegin,
	RKDDefineGroupStep2,
	RKDUseGroup,           // 25

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
	RKDDefineGroupEnd,
	RKDClose,             // 115

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
 * As a further complication, we cannot easily include R headers in frontend code, or (arbitray) Qt headers in backend code, thus the
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
#define MapDefault(Message,Ival,Qval) Message; return Ival;
#define RKD_IN_FRONTEND false
#else
#define MapEnum(Rval,Ival,Qval) case Ival: static_assert(Ival == (int) Qval, "Enum mismatch"); return Qval;
#define MapDefault(Message,Ival,Qval) return Qval;
#define RKD_IN_FRONTEND true
#define RKD_RGE_VERSION 99999
#endif

static inline quint8 mapLineEndStyle(quint8 from) {
	if (RKD_IN_FRONTEND) return from;
	switch(from) {
		MapEnum(R_GE_lineend::GE_BUTT_CAP, 0x00, Qt::FlatCap);
		MapEnum(R_GE_lineend::GE_SQUARE_CAP, 0x10, Qt::SquareCap);
		MapEnum(R_GE_lineend::GE_ROUND_CAP, 0x20, Qt::RoundCap);
	}
	MapDefault({}, 0x00, Qt::FlatCap);
}

static inline quint8 mapLineJoinStyle(quint8 from) {
	if (RKD_IN_FRONTEND) return from;
	switch(from) {
		MapEnum(R_GE_linejoin::GE_MITRE_JOIN, 0x00, Qt::MiterJoin);
		MapEnum(R_GE_linejoin::GE_BEVEL_JOIN, 0x40, Qt::BevelJoin);
		MapEnum(R_GE_linejoin::GE_ROUND_JOIN, 0x80, Qt::RoundJoin);
		//MapEnum(GE_ROUND_JOIN, 0x100, Qt::SvgMiterJoin);  // not available in R, and wouldn't fit in quint8
	}
	MapDefault({}, 0x00, Qt::MiterJoin);
}

#if RKD_RGE_VERSION >= 15
static inline int mapCompositionModeEnum(int from) {
	if (RKD_IN_FRONTEND) return from;
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
	MapDefault(RFn::Rf_warning("Unsupported enumeration value %d", from), 0, QPainter::CompositionMode_SourceOver);
}

static inline quint8 mapFillRule(quint8 from) {
	if (RKD_IN_FRONTEND) return from;
	switch(from) {
		MapEnum(R_GE_evenOddRule, 0, Qt::OddEvenFill);
		MapEnum(R_GE_nonZeroWindingRule, 1, Qt::WindingFill);
	}
	MapDefault({}, 0x00, Qt::OddEvenFill);
}
#endif

#endif
