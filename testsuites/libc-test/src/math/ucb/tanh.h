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
// tanhd(tiny)~tiny, tanhd(+-max or inf) = +-1
T(RN,                  0x0p+0,                  0x0p+0,          0x0p+0, 0)
T(RN,                 -0x0p+0,                 -0x0p+0,          0x0p+0, 0)
T(RN,               0x1p-1074,               0x1p-1074,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,              -0x1p-1074,              -0x1p-1074,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,               0x1p-1042,               0x1p-1042,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,              -0x1p-1042,              -0x1p-1042,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,               0x1p-1022,               0x1p-1022,          0x0p+0, INEXACT)
T(RN,              -0x1p-1022,              -0x1p-1022,          0x0p+0, INEXACT)
T(RN,                 0x1p-67,                 0x1p-67,          0x0p+0, INEXACT)
T(RN,                -0x1p-67,                -0x1p-67,          0x0p+0, INEXACT)
T(RN, 0x1.fffffffffffffp+1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.fffffffffffffp+1023,                 -0x1p+0,          0x0p+0, INEXACT)
T(RN,                     inf,                  0x1p+0,          0x0p+0, 0)
T(RN,                    -inf,                 -0x1p+0,          0x0p+0, 0)
// random arguments between -9 9
T(RN,   -0x1.358d5b2b5006dp+1,   -0x1.f7f0d680d659fp-1,   0x1.fcb574p-2, INEXACT)
T(RN,    0x1.9a3defb825911p+2,     0x1.ffff4a686706p-1,  -0x1.911f74p-3, INEXACT)
T(RN,    0x1.fb23a09de7505p+2,    0x1.fffff73581933p-1,  -0x1.a85578p-7, INEXACT)
T(RN,   -0x1.80af4fba96889p+2,   -0x1.fffe6c66ce5c3p-1,  -0x1.726dacp-2, INEXACT)
T(RN,   -0x1.38148e27084ddp+1,   -0x1.f84024aed09edp-1,  -0x1.03eefep-3, INEXACT)
T(RN,   -0x1.6a1d61b093c41p-2,   -0x1.5bbd2db600bb7p-2,   -0x1.954e3p-5, INEXACT)
T(RN,    0x1.4d1e6d18455f5p+2,    0x1.fff81a052883ap-1,   0x1.37ae22p-5, INEXACT)
T(RN,    0x1.ec87f4a51b239p+2,    0x1.fffff21f7f28dp-1,     0x1.0b18p-4, INEXACT)
T(RN,   -0x1.833b11079de4dp+0,   -0x1.d0971d00e2766p-1,  -0x1.5ecb56p-2, INEXACT)
T(RN,   -0x1.13e933103b871p+3,   -0x1.fffffdd2ff3acp-1,   0x1.b5f834p-3, INEXACT)
T(RZ,   -0x1.358d5b2b5006dp+1,   -0x1.f7f0d680d659fp-1,   0x1.fcb574p-2, INEXACT)
T(RZ,    0x1.9a3defb825911p+2,     0x1.ffff4a686706p-1,  -0x1.911f74p-3, INEXACT)
T(RZ,    0x1.fb23a09de7505p+2,    0x1.fffff73581933p-1,  -0x1.a85578p-7, INEXACT)
T(RZ,   -0x1.80af4fba96889p+2,   -0x1.fffe6c66ce5c2p-1,   0x1.46c92ap-1, INEXACT)
T(RZ,   -0x1.38148e27084ddp+1,   -0x1.f84024aed09ecp-1,    0x1.bf044p-1, INEXACT)
T(RZ,   -0x1.6a1d61b093c41p-2,   -0x1.5bbd2db600bb6p-2,   0x1.e6ab1cp-1, INEXACT)
T(RZ,    0x1.4d1e6d18455f5p+2,    0x1.fff81a0528839p-1,  -0x1.ec851ep-1, INEXACT)
T(RZ,    0x1.ec87f4a51b239p+2,    0x1.fffff21f7f28cp-1,    -0x1.de9dp-1, INEXACT)
T(RZ,   -0x1.833b11079de4dp+0,   -0x1.d0971d00e2765p-1,   0x1.509a56p-1, INEXACT)
T(RZ,   -0x1.13e933103b871p+3,   -0x1.fffffdd2ff3acp-1,   0x1.b5f834p-3, INEXACT)
T(RU,   -0x1.358d5b2b5006dp+1,   -0x1.f7f0d680d659fp-1,   0x1.fcb574p-2, INEXACT)
T(RU,    0x1.9a3defb825911p+2,    0x1.ffff4a6867061p-1,   0x1.9bb824p-1, INEXACT)
T(RU,    0x1.fb23a09de7505p+2,    0x1.fffff73581934p-1,   0x1.f95eaap-1, INEXACT)
T(RU,   -0x1.80af4fba96889p+2,   -0x1.fffe6c66ce5c2p-1,   0x1.46c92ap-1, INEXACT)
T(RU,   -0x1.38148e27084ddp+1,   -0x1.f84024aed09ecp-1,    0x1.bf044p-1, INEXACT)
T(RU,   -0x1.6a1d61b093c41p-2,   -0x1.5bbd2db600bb6p-2,   0x1.e6ab1cp-1, INEXACT)
T(RU,    0x1.4d1e6d18455f5p+2,    0x1.fff81a052883ap-1,   0x1.37ae22p-5, INEXACT)
T(RU,    0x1.ec87f4a51b239p+2,    0x1.fffff21f7f28dp-1,     0x1.0b18p-4, INEXACT)
T(RU,   -0x1.833b11079de4dp+0,   -0x1.d0971d00e2765p-1,   0x1.509a56p-1, INEXACT)
T(RU,   -0x1.13e933103b871p+3,   -0x1.fffffdd2ff3acp-1,   0x1.b5f834p-3, INEXACT)
T(RD,   -0x1.358d5b2b5006dp+1,    -0x1.f7f0d680d65ap-1,  -0x1.01a546p-1, INEXACT)
T(RD,    0x1.9a3defb825911p+2,     0x1.ffff4a686706p-1,  -0x1.911f74p-3, INEXACT)
T(RD,    0x1.fb23a09de7505p+2,    0x1.fffff73581933p-1,  -0x1.a85578p-7, INEXACT)
T(RD,   -0x1.80af4fba96889p+2,   -0x1.fffe6c66ce5c3p-1,  -0x1.726dacp-2, INEXACT)
T(RD,   -0x1.38148e27084ddp+1,   -0x1.f84024aed09edp-1,  -0x1.03eefep-3, INEXACT)
T(RD,   -0x1.6a1d61b093c41p-2,   -0x1.5bbd2db600bb7p-2,   -0x1.954e3p-5, INEXACT)
T(RD,    0x1.4d1e6d18455f5p+2,    0x1.fff81a0528839p-1,  -0x1.ec851ep-1, INEXACT)
T(RD,    0x1.ec87f4a51b239p+2,    0x1.fffff21f7f28cp-1,    -0x1.de9dp-1, INEXACT)
T(RD,   -0x1.833b11079de4dp+0,   -0x1.d0971d00e2766p-1,  -0x1.5ecb56p-2, INEXACT)
T(RD,   -0x1.13e933103b871p+3,   -0x1.fffffdd2ff3adp-1,  -0x1.9281f4p-1, INEXACT)
// tanhd(nan) is nan
T(RN,                     nan,                     nan,          0x0p+0, 0)
T(RD,                  0x0p+0,                  0x0p+0,          0x0p+0, 0)
T(RD,                     inf,                  0x1p+0,          0x0p+0, 0)
T(RD,                 -0x0p+0,                 -0x0p+0,          0x0p+0, 0)
T(RD,                    -inf,                 -0x1p+0,          0x0p+0, 0)
T(RD,                     nan,                     nan,          0x0p+0, 0)
T(RD,                     nan,                     nan,          0x0p+0, 0)
T(RD, 0x1.0000000000001p-1022,               0x1p-1022,         -0x1p+0, INEXACT)
T(RD, 0x1.0000000000002p-1022, 0x1.0000000000001p-1022,         -0x1p+0, INEXACT)
T(RD,               0x1p-1021, 0x1.fffffffffffffp-1022,         -0x1p+0, INEXACT)
T(RD,               0x1p-1020, 0x1.fffffffffffffp-1021,         -0x1p+0, INEXACT)
T(RD,                 0x1p-28,   0x1.fffffffffffffp-29,  -0x1.eaaaaap-1, INEXACT)
T(RD,                 0x1p-27,   0x1.fffffffffffffp-28,  -0x1.aaaaaap-1, INEXACT)
T(RD,               0x1.8p-27,   0x1.7ffffffffffffp-27,       -0x1.7p-1, INEXACT)
T(RD,                 0x1p-26,   0x1.fffffffffffffp-27,  -0x1.555556p-2, INEXACT)
T(RD,               0x1.4p-26,   0x1.3ffffffffffffp-26,  -0x1.655556p-2, INEXACT)
T(RD,               0x1.8p-26,   0x1.7fffffffffffep-26,       -0x1.cp-1, INEXACT)
T(RD,              0x1.634p+9,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RD,               0x1p+1022,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RD,               0x1p+1023,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RD, 0x1.ffffffffffffep+1023,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RD, 0x1.fffffffffffffp+1023,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RD,-0x1.0000000000001p-1022,-0x1.0000000000001p-1022,          0x0p+0, INEXACT)
T(RD,-0x1.0000000000002p-1022,-0x1.0000000000002p-1022,          0x0p+0, INEXACT)
T(RD,              -0x1p-1021,              -0x1p-1021,          0x0p+0, INEXACT)
T(RD,              -0x1p-1020,              -0x1p-1020,          0x0p+0, INEXACT)
T(RD,                -0x1p-28,                -0x1p-28,  -0x1.555556p-6, INEXACT)
T(RD,                -0x1p-27,                -0x1p-27,  -0x1.555556p-4, INEXACT)
T(RD,              -0x1.8p-27,              -0x1.8p-27,       -0x1.2p-2, INEXACT)
T(RD,                -0x1p-26,                -0x1p-26,  -0x1.555556p-2, INEXACT)
T(RD,              -0x1.4p-26,              -0x1.4p-26,  -0x1.4d5556p-1, INEXACT)
T(RD,              -0x1.8p-26,  -0x1.7ffffffffffffp-26,         -0x1p-3, INEXACT)
T(RD,             -0x1.634p+9,                 -0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p+1022,                 -0x1p+0,          0x0p+0, INEXACT)
T(RD,              -0x1p+1023,                 -0x1p+0,          0x0p+0, INEXACT)
T(RD,-0x1.ffffffffffffep+1023,                 -0x1p+0,          0x0p+0, INEXACT)
T(RD,-0x1.fffffffffffffp+1023,                 -0x1p+0,          0x0p+0, INEXACT)
T(RD,               0x1p-1074,                  0x0p+0,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD,               0x1p-1073,               0x1p-1074,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD,               0x1p-1024, 0x1.ffffffffffff8p-1025,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD,               0x1p-1023, 0x1.ffffffffffffcp-1024,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD, 0x1.ffffffffffffcp-1023, 0x1.ffffffffffffap-1023,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD, 0x1.ffffffffffffep-1023, 0x1.ffffffffffffcp-1023,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD,               0x1p-1022, 0x1.ffffffffffffep-1023,         -0x1p+0, INEXACT|UNDERFLOW)
T(RD,              -0x1p-1074,              -0x1p-1074,          0x0p+0, INEXACT|UNDERFLOW)
T(RD,              -0x1p-1073,              -0x1p-1073,          0x0p+0, INEXACT|UNDERFLOW)
T(RD,              -0x1p-1024,              -0x1p-1024,          0x0p+0, INEXACT|UNDERFLOW)
T(RD,              -0x1p-1023,              -0x1p-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RD,-0x1.ffffffffffffcp-1023,-0x1.ffffffffffffcp-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RD,-0x1.ffffffffffffep-1023,-0x1.ffffffffffffep-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RD,              -0x1p-1022,              -0x1p-1022,          0x0p+0, INEXACT)
T(RN, 0x1.0000000000001p-1022, 0x1.0000000000001p-1022,          0x0p+0, INEXACT)
T(RN, 0x1.0000000000002p-1022, 0x1.0000000000002p-1022,          0x0p+0, INEXACT)
T(RN,               0x1p-1021,               0x1p-1021,          0x0p+0, INEXACT)
T(RN,               0x1p-1020,               0x1p-1020,          0x0p+0, INEXACT)
T(RN,                 0x1p-28,                 0x1p-28,   0x1.555556p-6, INEXACT)
T(RN,                 0x1p-27,                 0x1p-27,   0x1.555556p-4, INEXACT)
T(RN,               0x1.8p-27,               0x1.8p-27,        0x1.2p-2, INEXACT)
T(RN,                 0x1p-26,   0x1.fffffffffffffp-27,  -0x1.555556p-2, INEXACT)
T(RN,               0x1.4p-26,   0x1.3ffffffffffffp-26,  -0x1.655556p-2, INEXACT)
T(RN,               0x1.8p-26,   0x1.7ffffffffffffp-26,          0x1p-3, INEXACT)
T(RN,              0x1.634p+9,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p+1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p+1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN, 0x1.ffffffffffffep+1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.0000000000001p-1022,-0x1.0000000000001p-1022,          0x0p+0, INEXACT)
T(RN,-0x1.0000000000002p-1022,-0x1.0000000000002p-1022,          0x0p+0, INEXACT)
T(RN,              -0x1p-1021,              -0x1p-1021,          0x0p+0, INEXACT)
T(RN,              -0x1p-1020,              -0x1p-1020,          0x0p+0, INEXACT)
T(RN,                -0x1p-28,                -0x1p-28,  -0x1.555556p-6, INEXACT)
T(RN,                -0x1p-27,                -0x1p-27,  -0x1.555556p-4, INEXACT)
T(RN,              -0x1.8p-27,              -0x1.8p-27,       -0x1.2p-2, INEXACT)
T(RN,                -0x1p-26,  -0x1.fffffffffffffp-27,   0x1.555556p-2, INEXACT)
T(RN,              -0x1.4p-26,  -0x1.3ffffffffffffp-26,   0x1.655556p-2, INEXACT)
T(RN,              -0x1.8p-26,  -0x1.7ffffffffffffp-26,         -0x1p-3, INEXACT)
T(RN,             -0x1.634p+9,                 -0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p+1022,                 -0x1p+0,          0x0p+0, INEXACT)
T(RN,              -0x1p+1023,                 -0x1p+0,          0x0p+0, INEXACT)
T(RN,-0x1.ffffffffffffep+1023,                 -0x1p+0,          0x0p+0, INEXACT)
T(RN,               0x1p-1073,               0x1p-1073,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,               0x1p-1024,               0x1p-1024,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,               0x1p-1023,               0x1p-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RN, 0x1.ffffffffffffcp-1023, 0x1.ffffffffffffcp-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RN, 0x1.ffffffffffffep-1023, 0x1.ffffffffffffep-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,              -0x1p-1073,              -0x1p-1073,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,              -0x1p-1024,              -0x1p-1024,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,              -0x1p-1023,              -0x1p-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,-0x1.ffffffffffffcp-1023,-0x1.ffffffffffffcp-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,-0x1.ffffffffffffep-1023,-0x1.ffffffffffffep-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RN,                     nan,                     nan,          0x0p+0, 0)
T(RU,                  0x0p+0,                  0x0p+0,          0x0p+0, 0)
T(RU,                     inf,                  0x1p+0,          0x0p+0, 0)
T(RU,                 -0x0p+0,                 -0x0p+0,          0x0p+0, 0)
T(RU,                    -inf,                 -0x1p+0,          0x0p+0, 0)
T(RU,                     nan,                     nan,          0x0p+0, 0)
T(RU,                     nan,                     nan,          0x0p+0, 0)
T(RU, 0x1.0000000000001p-1022, 0x1.0000000000001p-1022,          0x0p+0, INEXACT)
T(RU, 0x1.0000000000002p-1022, 0x1.0000000000002p-1022,          0x0p+0, INEXACT)
T(RU,               0x1p-1021,               0x1p-1021,          0x0p+0, INEXACT)
T(RU,               0x1p-1020,               0x1p-1020,          0x0p+0, INEXACT)
T(RU,                 0x1p-28,                 0x1p-28,   0x1.555556p-6, INEXACT)
T(RU,                 0x1p-27,                 0x1p-27,   0x1.555556p-4, INEXACT)
T(RU,               0x1.8p-27,               0x1.8p-27,        0x1.2p-2, INEXACT)
T(RU,                 0x1p-26,                 0x1p-26,   0x1.555556p-2, INEXACT)
T(RU,               0x1.4p-26,               0x1.4p-26,   0x1.4d5556p-1, INEXACT)
T(RU,               0x1.8p-26,   0x1.7ffffffffffffp-26,          0x1p-3, INEXACT)
T(RU,              0x1.634p+9,                  0x1p+0,          0x0p+0, INEXACT)
T(RU,               0x1p+1022,                  0x1p+0,          0x0p+0, INEXACT)
T(RU,               0x1p+1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RU, 0x1.ffffffffffffep+1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RU, 0x1.fffffffffffffp+1023,                  0x1p+0,          0x0p+0, INEXACT)
T(RU,-0x1.0000000000001p-1022,              -0x1p-1022,          0x1p+0, INEXACT)
T(RU,-0x1.0000000000002p-1022,-0x1.0000000000001p-1022,          0x1p+0, INEXACT)
T(RU,              -0x1p-1021,-0x1.fffffffffffffp-1022,          0x1p+0, INEXACT)
T(RU,              -0x1p-1020,-0x1.fffffffffffffp-1021,          0x1p+0, INEXACT)
T(RU,                -0x1p-28,  -0x1.fffffffffffffp-29,   0x1.eaaaaap-1, INEXACT)
T(RU,                -0x1p-27,  -0x1.fffffffffffffp-28,   0x1.aaaaaap-1, INEXACT)
T(RU,              -0x1.8p-27,  -0x1.7ffffffffffffp-27,        0x1.7p-1, INEXACT)
T(RU,                -0x1p-26,  -0x1.fffffffffffffp-27,   0x1.555556p-2, INEXACT)
T(RU,              -0x1.4p-26,  -0x1.3ffffffffffffp-26,   0x1.655556p-2, INEXACT)
T(RU,              -0x1.8p-26,  -0x1.7fffffffffffep-26,        0x1.cp-1, INEXACT)
T(RU,             -0x1.634p+9,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RU,              -0x1p+1022,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RU,              -0x1p+1023,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RU,-0x1.ffffffffffffep+1023,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RU,-0x1.fffffffffffffp+1023,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RU,               0x1p-1074,               0x1p-1074,          0x0p+0, INEXACT|UNDERFLOW)
T(RU,               0x1p-1073,               0x1p-1073,          0x0p+0, INEXACT|UNDERFLOW)
T(RU,               0x1p-1024,               0x1p-1024,          0x0p+0, INEXACT|UNDERFLOW)
T(RU,               0x1p-1023,               0x1p-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RU, 0x1.ffffffffffffcp-1023, 0x1.ffffffffffffcp-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RU, 0x1.ffffffffffffep-1023, 0x1.ffffffffffffep-1023,          0x0p+0, INEXACT|UNDERFLOW)
T(RU,               0x1p-1022,               0x1p-1022,          0x0p+0, INEXACT)
T(RU,              -0x1p-1074,                 -0x0p+0,          0x1p+0, INEXACT|UNDERFLOW)
T(RU,              -0x1p-1073,              -0x1p-1074,          0x1p+0, INEXACT|UNDERFLOW)
T(RU,              -0x1p-1024,-0x1.ffffffffffff8p-1025,          0x1p+0, INEXACT|UNDERFLOW)
T(RU,              -0x1p-1023,-0x1.ffffffffffffcp-1024,          0x1p+0, INEXACT|UNDERFLOW)
T(RU,-0x1.ffffffffffffcp-1023,-0x1.ffffffffffffap-1023,          0x1p+0, INEXACT|UNDERFLOW)
T(RU,-0x1.ffffffffffffep-1023,-0x1.ffffffffffffcp-1023,          0x1p+0, INEXACT|UNDERFLOW)
T(RU,              -0x1p-1022,-0x1.ffffffffffffep-1023,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,                  0x0p+0,                  0x0p+0,          0x0p+0, 0)
T(RZ,                     inf,                  0x1p+0,          0x0p+0, 0)
T(RZ,                 -0x0p+0,                 -0x0p+0,          0x0p+0, 0)
T(RZ,                    -inf,                 -0x1p+0,          0x0p+0, 0)
T(RZ,                     nan,                     nan,          0x0p+0, 0)
T(RZ,                     nan,                     nan,          0x0p+0, 0)
T(RZ, 0x1.0000000000001p-1022,               0x1p-1022,         -0x1p+0, INEXACT)
T(RZ, 0x1.0000000000002p-1022, 0x1.0000000000001p-1022,         -0x1p+0, INEXACT)
T(RZ,               0x1p-1021, 0x1.fffffffffffffp-1022,         -0x1p+0, INEXACT)
T(RZ,               0x1p-1020, 0x1.fffffffffffffp-1021,         -0x1p+0, INEXACT)
T(RZ,                 0x1p-28,   0x1.fffffffffffffp-29,  -0x1.eaaaaap-1, INEXACT)
T(RZ,                 0x1p-27,   0x1.fffffffffffffp-28,  -0x1.aaaaaap-1, INEXACT)
T(RZ,               0x1.8p-27,   0x1.7ffffffffffffp-27,       -0x1.7p-1, INEXACT)
T(RZ,                 0x1p-26,   0x1.fffffffffffffp-27,  -0x1.555556p-2, INEXACT)
T(RZ,               0x1.4p-26,   0x1.3ffffffffffffp-26,  -0x1.655556p-2, INEXACT)
T(RZ,               0x1.8p-26,   0x1.7fffffffffffep-26,       -0x1.cp-1, INEXACT)
T(RZ,              0x1.634p+9,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RZ,               0x1p+1022,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RZ,               0x1p+1023,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RZ, 0x1.ffffffffffffep+1023,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RZ, 0x1.fffffffffffffp+1023,    0x1.fffffffffffffp-1,         -0x1p+0, INEXACT)
T(RZ,-0x1.0000000000001p-1022,              -0x1p-1022,          0x1p+0, INEXACT)
T(RZ,-0x1.0000000000002p-1022,-0x1.0000000000001p-1022,          0x1p+0, INEXACT)
T(RZ,              -0x1p-1021,-0x1.fffffffffffffp-1022,          0x1p+0, INEXACT)
T(RZ,              -0x1p-1020,-0x1.fffffffffffffp-1021,          0x1p+0, INEXACT)
T(RZ,                -0x1p-28,  -0x1.fffffffffffffp-29,   0x1.eaaaaap-1, INEXACT)
T(RZ,                -0x1p-27,  -0x1.fffffffffffffp-28,   0x1.aaaaaap-1, INEXACT)
T(RZ,              -0x1.8p-27,  -0x1.7ffffffffffffp-27,        0x1.7p-1, INEXACT)
T(RZ,                -0x1p-26,  -0x1.fffffffffffffp-27,   0x1.555556p-2, INEXACT)
T(RZ,              -0x1.4p-26,  -0x1.3ffffffffffffp-26,   0x1.655556p-2, INEXACT)
T(RZ,              -0x1.8p-26,  -0x1.7fffffffffffep-26,        0x1.cp-1, INEXACT)
T(RZ,             -0x1.634p+9,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RZ,              -0x1p+1022,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RZ,              -0x1p+1023,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RZ,-0x1.ffffffffffffep+1023,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RZ,-0x1.fffffffffffffp+1023,   -0x1.fffffffffffffp-1,          0x1p+0, INEXACT)
T(RZ,               0x1p-1074,                  0x0p+0,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ,               0x1p-1073,               0x1p-1074,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ,               0x1p-1024, 0x1.ffffffffffff8p-1025,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ,               0x1p-1023, 0x1.ffffffffffffcp-1024,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ, 0x1.ffffffffffffcp-1023, 0x1.ffffffffffffap-1023,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ, 0x1.ffffffffffffep-1023, 0x1.ffffffffffffcp-1023,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ,               0x1p-1022, 0x1.ffffffffffffep-1023,         -0x1p+0, INEXACT|UNDERFLOW)
T(RZ,              -0x1p-1074,                 -0x0p+0,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,              -0x1p-1073,              -0x1p-1074,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,              -0x1p-1024,-0x1.ffffffffffff8p-1025,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,              -0x1p-1023,-0x1.ffffffffffffcp-1024,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,-0x1.ffffffffffffcp-1023,-0x1.ffffffffffffap-1023,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,-0x1.ffffffffffffep-1023,-0x1.ffffffffffffcp-1023,          0x1p+0, INEXACT|UNDERFLOW)
T(RZ,              -0x1p-1022,-0x1.ffffffffffffep-1023,          0x1p+0, INEXACT|UNDERFLOW)
