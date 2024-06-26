// Copyright (C) 1988-1994 Sun Microsystems, Inc. 2550 Garcia Avenue
// Mountain View, California  94043 All rights reserved.
//
// Any person is hereby authorized to download, copy, use, create bug fixes,
// and distribute, subject to the following conditions:
//
// 	1.  the software may not be redistributed for a fee except as
// 	    reasonable to cover media costs;
// 	2.  any copy of the software must include this notice, as well as
// 	    any other embedded copyright notices; and
// 	3.  any distribution of this software or derivative works thereof
// 	    must comply with all applicable U.S. export control laws.
//
// THE SOFTWARE IS MADE AVAILABLE "AS IS" AND WITHOUT EXPRESS OR IMPLIED
// WARRANTY OF ANY KIND, INCLUDING BUT NOT LIMITED TO THE IMPLIED
// WARRANTIES OF DESIGN, MERCHANTIBILITY, FITNESS FOR A PARTICULAR
// PURPOSE, NON-INFRINGEMENT, PERFORMANCE OR CONFORMANCE TO
// SPECIFICATIONS.
//
// BY DOWNLOADING AND/OR USING THIS SOFTWARE, THE USER WAIVES ALL CLAIMS
// AGAINST SUN MICROSYSTEMS, INC. AND ITS AFFILIATED COMPANIES IN ANY
// JURISDICTION, INCLUDING BUT NOT LIMITED TO CLAIMS FOR DAMAGES OR
// EQUITABLE RELIEF BASED ON LOSS OF DATA, AND SPECIFICALLY WAIVES EVEN
// UNKNOWN OR UNANTICIPATED CLAIMS OR LOSSES, PRESENT AND FUTURE.
//
// IN NO EVENT WILL SUN MICROSYSTEMS, INC. OR ANY OF ITS AFFILIATED
// COMPANIES BE LIABLE FOR ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL,
// INDIRECT AND CONSEQUENTIAL DAMAGES, EVEN IF IT HAS BEEN ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGES.
//
// This file is provided with no support and without any obligation on the
// part of Sun Microsystems, Inc. ("Sun") or any of its affiliated
// companies to assist in its use, correction, modification or
// enhancement.  Nevertheless, and without creating any obligation on its
// part, Sun welcomes your comments concerning the software and requests
// that they be sent to fdlibm-comments@sunpro.sun.com.
// log10s(+fmax) is finite
T(RN, 0x1.fffffep+127,   0x1.344136p+5,   0x1.f3039ep-2, INEXACT)
T(RZ, 0x1.fffffep+127,   0x1.344134p+5,   -0x1.067e3p-1, INEXACT)
T(RU, 0x1.fffffep+127,   0x1.344136p+5,   0x1.f3039ep-2, INEXACT)
T(RD, 0x1.fffffep+127,   0x1.344134p+5,   -0x1.067e3p-1, INEXACT)
// log10s(10**n) == n (n=1,...,10)
T(RN,        0x1.4p+3,          0x1p+0,          0x0p+0, 0)
T(RN,        0x1.9p+6,          0x1p+1,          0x0p+0, 0)
T(RN,       0x1.f4p+9,        0x1.8p+1,          0x0p+0, 0)
T(RN,     0x1.388p+13,          0x1p+2,          0x0p+0, 0)
T(RN,     0x1.86ap+16,        0x1.4p+2,          0x0p+0, 0)
T(RN,    0x1.e848p+19,        0x1.8p+2,          0x0p+0, 0)
T(RN,    0x1.312dp+23,        0x1.cp+2,          0x0p+0, 0)
T(RN,   0x1.7d784p+26,          0x1p+3,          0x0p+0, 0)
T(RN,   0x1.dcd65p+29,        0x1.2p+3,          0x0p+0, 0)
T(RN,  0x1.2a05f2p+33,        0x1.4p+3,          0x0p+0, 0)
// log10s(1+tiny) is (tiny - tiny*tiny)/log10
T(RN,    0x1.00004p+0,  0x1.bcb77ap-20,   0x1.11fa56p-3, INEXACT)
T(RN,          0x1p+0,          0x0p+0,          0x0p+0, 0)
T(RN,    0x1.ffff8p-1, -0x1.bcb7e8p-20,   0x1.d2db7ep-2, INEXACT)
T(RZ,    0x1.00004p+0,  0x1.bcb778p-20,  -0x1.bb816ap-1, INEXACT)
T(RZ,          0x1p+0,          0x0p+0,          0x0p+0, 0)
T(RZ,    0x1.ffff8p-1, -0x1.bcb7e8p-20,   0x1.d2db7ep-2, INEXACT)
T(RU,    0x1.00004p+0,  0x1.bcb77ap-20,   0x1.11fa56p-3, INEXACT)
T(RU,          0x1p+0,          0x0p+0,          0x0p+0, 0)
T(RU,    0x1.ffff8p-1, -0x1.bcb7e8p-20,   0x1.d2db7ep-2, INEXACT)
T(RD,    0x1.00004p+0,  0x1.bcb778p-20,  -0x1.bb816ap-1, INEXACT)
T(RD,          0x1p+0,          0x0p+0,          0x0p+0, 0)
T(RD,    0x1.ffff8p-1, -0x1.bcb7eap-20,   -0x1.16924p-1, INEXACT)
// log10s(min) is finite
T(RN,        0x1p-126,   -0x1.2f703p+5,   0x1.ae7e0cp-4, INEXACT)
T(RN,        0x1p-149,  -0x1.66d3e8p+5,    -0x1.0997p-3, INEXACT)
T(RZ,        0x1p-126,   -0x1.2f703p+5,   0x1.ae7e0cp-4, INEXACT)
T(RZ,        0x1p-149,  -0x1.66d3e6p+5,    0x1.bd9a4p-1, INEXACT)
T(RU,        0x1p-126,   -0x1.2f703p+5,   0x1.ae7e0cp-4, INEXACT)
T(RU,        0x1p-149,  -0x1.66d3e6p+5,    0x1.bd9a4p-1, INEXACT)
T(RD,        0x1p-126,  -0x1.2f7032p+5,  -0x1.ca303ep-1, INEXACT)
T(RD,        0x1p-149,  -0x1.66d3e8p+5,    -0x1.0997p-3, INEXACT)
// random arguments between 0 100
T(RN,   0x1.24844cp+5,    0x1.9024cp+0,  -0x1.7605eap-2, INEXACT)
T(RN,     0x1.5672p+6,   0x1.eeba5ep+0,   0x1.aaa14cp-6, INEXACT)
T(RN,     0x1.7817p+6,   0x1.f925b6p+0,   0x1.fb3242p-2, INEXACT)
T(RN,    0x1.09b75p+4,    0x1.38657p+0,  -0x1.0801cap-3, INEXACT)
T(RN,   0x1.23a38ep+5,   0x1.8fcf34p+0,   -0x1.9ea21p-3, INEXACT)
T(RN,   0x1.804882p+5,    0x1.ae7adp+0,    0x1.60441p-3, INEXACT)
T(RN,   0x1.3baa8ep+6,    0x1.e5acdp+0,   0x1.e6b71ep-3, INEXACT)
T(RN,   0x1.730484p+6,   0x1.f7a33cp+0,  -0x1.ecfb84p-2, INEXACT)
T(RN,   0x1.4cc5b2p+5,   0x1.9e7a86p+0,  -0x1.ca6cb4p-2, INEXACT)
T(RN,   0x1.0ca4d4p+1,   0x1.49b1b4p-2,  -0x1.9af518p-4, INEXACT)
// log10s(nan) is nan
T(RN,             nan,             nan,          0x0p+0, 0)
// log10s(+inf) is inf
T(RN,             inf,             inf,          0x0p+0, 0)
// log10s(+-0) is -inf
T(RN,          0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RN,         -0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
// log10s(-ve) is nan
T(RN,       -0x1p-149,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p-126,             nan,          0x0p+0, INVALID)
T(RN,-0x1.fffffep+127,             nan,          0x0p+0, INVALID)
T(RN,            -inf,             nan,          0x0p+0, INVALID)
T(RD,             inf,             inf,          0x0p+0, 0)
T(RD,          0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RD,         -0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RD,             nan,             nan,          0x0p+0, 0)
T(RD,             nan,             nan,          0x0p+0, 0)
T(RD,       -0x1p-149,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p-148,             nan,          0x0p+0, INVALID)
T(RD,     -0x1.cp-147,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p-128,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p-127,             nan,          0x0p+0, INVALID)
T(RD,-0x1.fffff8p-127,             nan,          0x0p+0, INVALID)
T(RD,-0x1.fffffcp-127,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p-126,             nan,          0x0p+0, INVALID)
T(RD,-0x1.000002p-126,             nan,          0x0p+0, INVALID)
T(RD,-0x1.000004p-126,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p-125,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p-124,             nan,          0x0p+0, INVALID)
T(RD,        -0x1p-23,             nan,          0x0p+0, INVALID)
T(RD,        -0x1p-21,             nan,          0x0p+0, INVALID)
T(RD,         -0x1p-2,             nan,          0x0p+0, INVALID)
T(RD,         -0x1p-1,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.fffff4p-1,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.fffff8p-1,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.fffffcp-1,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.fffffep-1,             nan,          0x0p+0, INVALID)
T(RD,         -0x1p+0,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.000002p+0,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.000004p+0,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.000008p+0,             nan,          0x0p+0, INVALID)
T(RD,         -0x1p+1,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.000004p+1,             nan,          0x0p+0, INVALID)
T(RD,  -0x1.fffff6p+1,             nan,          0x0p+0, INVALID)
T(RD,         -0x1p+2,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p+126,             nan,          0x0p+0, INVALID)
T(RD,-0x1.000004p+126,             nan,          0x0p+0, INVALID)
T(RD,-0x1.000008p+126,             nan,          0x0p+0, INVALID)
T(RD,       -0x1p+127,             nan,          0x0p+0, INVALID)
T(RD,-0x1.000004p+127,             nan,          0x0p+0, INVALID)
T(RD,-0x1.fffffcp+127,             nan,          0x0p+0, INVALID)
T(RD,-0x1.fffffep+127,             nan,          0x0p+0, INVALID)
T(RD,            -inf,             nan,          0x0p+0, INVALID)
T(RD,        0x1.4p+3,          0x1p+0,          0x0p+0, 0)
T(RD,        0x1.9p+6,          0x1p+1,          0x0p+0, 0)
T(RD,       0x1.f4p+9,        0x1.8p+1,          0x0p+0, 0)
T(RD,     0x1.388p+13,          0x1p+2,          0x0p+0, 0)
T(RN,             nan,             nan,          0x0p+0, 0)
T(RN,       -0x1p-148,             nan,          0x0p+0, INVALID)
T(RN,     -0x1.cp-147,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p-128,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p-127,             nan,          0x0p+0, INVALID)
T(RN,-0x1.fffff8p-127,             nan,          0x0p+0, INVALID)
T(RN,-0x1.fffffcp-127,             nan,          0x0p+0, INVALID)
T(RN,-0x1.000002p-126,             nan,          0x0p+0, INVALID)
T(RN,-0x1.000004p-126,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p-125,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p-124,             nan,          0x0p+0, INVALID)
T(RN,        -0x1p-23,             nan,          0x0p+0, INVALID)
T(RN,        -0x1p-21,             nan,          0x0p+0, INVALID)
T(RN,         -0x1p-2,             nan,          0x0p+0, INVALID)
T(RN,         -0x1p-1,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.fffff4p-1,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.fffff8p-1,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.fffffcp-1,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.fffffep-1,             nan,          0x0p+0, INVALID)
T(RN,         -0x1p+0,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.000002p+0,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.000004p+0,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.000008p+0,             nan,          0x0p+0, INVALID)
T(RN,         -0x1p+1,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.000004p+1,             nan,          0x0p+0, INVALID)
T(RN,  -0x1.fffff6p+1,             nan,          0x0p+0, INVALID)
T(RN,         -0x1p+2,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p+126,             nan,          0x0p+0, INVALID)
T(RN,-0x1.000004p+126,             nan,          0x0p+0, INVALID)
T(RN,-0x1.000008p+126,             nan,          0x0p+0, INVALID)
T(RN,       -0x1p+127,             nan,          0x0p+0, INVALID)
T(RN,-0x1.000004p+127,             nan,          0x0p+0, INVALID)
T(RN,-0x1.fffffcp+127,             nan,          0x0p+0, INVALID)
T(RU,             inf,             inf,          0x0p+0, 0)
T(RU,          0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RU,         -0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RU,             nan,             nan,          0x0p+0, 0)
T(RU,             nan,             nan,          0x0p+0, 0)
T(RU,       -0x1p-149,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p-148,             nan,          0x0p+0, INVALID)
T(RU,     -0x1.cp-147,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p-128,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p-127,             nan,          0x0p+0, INVALID)
T(RU,-0x1.fffff8p-127,             nan,          0x0p+0, INVALID)
T(RU,-0x1.fffffcp-127,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p-126,             nan,          0x0p+0, INVALID)
T(RU,-0x1.000002p-126,             nan,          0x0p+0, INVALID)
T(RU,-0x1.000004p-126,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p-125,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p-124,             nan,          0x0p+0, INVALID)
T(RU,        -0x1p-23,             nan,          0x0p+0, INVALID)
T(RU,        -0x1p-21,             nan,          0x0p+0, INVALID)
T(RU,         -0x1p-2,             nan,          0x0p+0, INVALID)
T(RU,         -0x1p-1,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.fffff4p-1,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.fffff8p-1,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.fffffcp-1,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.fffffep-1,             nan,          0x0p+0, INVALID)
T(RU,         -0x1p+0,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.000002p+0,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.000004p+0,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.000008p+0,             nan,          0x0p+0, INVALID)
T(RU,         -0x1p+1,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.000004p+1,             nan,          0x0p+0, INVALID)
T(RU,  -0x1.fffff6p+1,             nan,          0x0p+0, INVALID)
T(RU,         -0x1p+2,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p+126,             nan,          0x0p+0, INVALID)
T(RU,-0x1.000004p+126,             nan,          0x0p+0, INVALID)
T(RU,-0x1.000008p+126,             nan,          0x0p+0, INVALID)
T(RU,       -0x1p+127,             nan,          0x0p+0, INVALID)
T(RU,-0x1.000004p+127,             nan,          0x0p+0, INVALID)
T(RU,-0x1.fffffcp+127,             nan,          0x0p+0, INVALID)
T(RU,-0x1.fffffep+127,             nan,          0x0p+0, INVALID)
T(RU,            -inf,             nan,          0x0p+0, INVALID)
T(RU,        0x1.4p+3,          0x1p+0,          0x0p+0, 0)
T(RU,        0x1.9p+6,          0x1p+1,          0x0p+0, 0)
T(RU,       0x1.f4p+9,        0x1.8p+1,          0x0p+0, 0)
T(RU,     0x1.388p+13,          0x1p+2,          0x0p+0, 0)
T(RZ,             inf,             inf,          0x0p+0, 0)
T(RZ,          0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RZ,         -0x0p+0,            -inf,          0x0p+0, DIVBYZERO)
T(RZ,             nan,             nan,          0x0p+0, 0)
T(RZ,             nan,             nan,          0x0p+0, 0)
T(RZ,       -0x1p-149,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p-148,             nan,          0x0p+0, INVALID)
T(RZ,     -0x1.cp-147,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p-128,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p-127,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.fffff8p-127,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.fffffcp-127,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p-126,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.000002p-126,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.000004p-126,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p-125,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p-124,             nan,          0x0p+0, INVALID)
T(RZ,        -0x1p-23,             nan,          0x0p+0, INVALID)
T(RZ,        -0x1p-21,             nan,          0x0p+0, INVALID)
T(RZ,         -0x1p-2,             nan,          0x0p+0, INVALID)
T(RZ,         -0x1p-1,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.fffff4p-1,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.fffff8p-1,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.fffffcp-1,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.fffffep-1,             nan,          0x0p+0, INVALID)
T(RZ,         -0x1p+0,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.000002p+0,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.000004p+0,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.000008p+0,             nan,          0x0p+0, INVALID)
T(RZ,         -0x1p+1,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.000004p+1,             nan,          0x0p+0, INVALID)
T(RZ,  -0x1.fffff6p+1,             nan,          0x0p+0, INVALID)
T(RZ,         -0x1p+2,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p+126,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.000004p+126,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.000008p+126,             nan,          0x0p+0, INVALID)
T(RZ,       -0x1p+127,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.000004p+127,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.fffffcp+127,             nan,          0x0p+0, INVALID)
T(RZ,-0x1.fffffep+127,             nan,          0x0p+0, INVALID)
T(RZ,            -inf,             nan,          0x0p+0, INVALID)
T(RZ,        0x1.4p+3,          0x1p+0,          0x0p+0, 0)
T(RZ,        0x1.9p+6,          0x1p+1,          0x0p+0, 0)
T(RZ,       0x1.f4p+9,        0x1.8p+1,          0x0p+0, 0)
T(RZ,     0x1.388p+13,          0x1p+2,          0x0p+0, 0)
