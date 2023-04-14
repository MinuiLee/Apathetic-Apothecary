/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Version: v2019.2.4  Build: 7329
  Copyright (c) 2006-2020 Audiokinetic Inc.
*******************************************************************************/

// AkPlatformFuncs.h

/// \file 
/// Platform-dependent functions definition.

#ifndef _AK_TOOLS_COMMON_AKPLATFORMFUNCS_H
#define _AK_TOOLS_COMMON_AKPLATFORMFUNCS_H

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkAtomic.h>

// Uncomment the following to enable built-in platform profiler markers in the sound engine
//#define AK_ENABLE_INSTRUMENT

#if defined(AK_WIN)
#include <AK/Tools/Win32/AkPlatformFuncs.h>

#elif defined (AK_XBOX)
#include <AK/Tools/XboxOne/AkPlatformFuncs.h>

#elif defined (AK_APPLE)
#include <AK/Tools/Mac/AkPlatformFuncs.h>
#include <AK/Tools/POSIX/AkPlatformFuncs.h>

#elif defined (AK_ANDROID)
#include <AK/Tools/Android/AkPlatformFuncs.h>

#elif defined (AK_PS4)
#include <AK/Tools/PS4/AkPlatformFuncs.h>

#elif defined (AK_PELLEGRINO)
#include <AK/Tools/Pellegrino/AkPlatformFuncs.h>

#elif defined (AK_EMSCRIPTEN)
#include <AK/Tools/Emscripten/AkPlatformFuncs.h>

#elif defined (AK_LINUX)

#ifdef AK_GGP
#include <AK/Tools/GGP/AkPlatformFuncs.h>
#endif
#include <AK/Tools/Linux/AkPlatformFuncs.h>
#include <AK/Tools/POSIX/AkPlatformFuncs.h>

#elif defined (AK_QNX)
#include <AK/Tools/QNX/AkPlatformFuncs.h>
#include <AK/Tools/POSIX/AkPlatformFuncs.h>

#elif defined (AK_NX)
#include <AK/Tools/NX/AkPlatformFuncs.h>

#else
#error AkPlatformFuncs.h: Undefined platform
#endif

#ifndef AkPrefetchZero
#define AkPrefetchZero(___Dest, ___Size)
#endif

#ifndef AkPrefetchZeroAligned
#define AkPrefetchZeroAligned(___Dest, ___Size)
#endif

#ifndef AkZeroMemAligned
#define AkZeroMemAligned(___Dest, ___Size) AKPLATFORM::AkMemSet(___Dest, 0, ___Size);
#endif
#ifndef AkZeroMemLarge
#define AkZeroMemLarge(___Dest, ___Size) AKPLATFORM::AkMemSet(___Dest, 0, ___Size);
#endif
#ifndef AkZeroMemSmall
#define AkZeroMemSmall(___Dest, ___Size) AKPLATFORM::AkMemSet(___Dest, 0, ___Size);
#endif

#ifndef AkAllocaSIMD
#ifdef __clang__
#if __has_builtin( __builtin_alloca_with_align )
#define AkAllocaSIMD( _size_ ) __builtin_alloca_with_align( _size_, 128 )
#else
// work around alloca alignment issues in versions of clang before 4.0
#define AkAllocaSIMD( _size_ ) (void*)( ( ( uintptr_t )AkAlloca( _size_ + 16 ) + 0xF ) & ~0xF )
#endif
#else
#define AkAllocaSIMD( _size_ ) AkAlloca( _size_ )
#endif
#endif

#ifndef AK_THREAD_INIT_CODE
#define AK_THREAD_INIT_CODE(_threadProperties)
#endif

/// Utility functions
namespace AK
{
	/// Count non-zero bits.
	/// \return Number of channels.
	AkForceInline AkUInt32 GetNumNonZeroBits( AkUInt32 in_uWord )
	{
		AkUInt32 num = 0;
		while( in_uWord ){ ++num; in_uWord &= in_uWord-1; }
		return num;
	}
}

namespace AKPLATFORM
{
	inline void AkGetDefaultHighPriorityThreadProperties(AkThreadProperties & out_threadProperties)
	{
		AkGetDefaultThreadProperties(out_threadProperties);
		out_threadProperties.nPriority = AK_THREAD_PRIORITY_ABOVE_NORMAL;
	}
}

#ifndef AK_PERF_RECORDING_RESET
#define AK_PERF_RECORDING_RESET()
#endif
#ifndef AK_PERF_RECORDING_START
#define AK_PERF_RECORDING_START( __StorageName__, __uExecutionCountStart__, __uExecutionCountStop__ )
#endif
#ifndef AK_PERF_RECORDING_STOP
#define AK_PERF_RECORDING_STOP( __StorageName__, __uExecutionCountStart__, __uExecutionCountStop__ )	
#endif

#ifndef AK_INSTRUMENT_BEGIN
	#define AK_INSTRUMENT_BEGIN( _zone_name_ )
	#define AK_INSTRUMENT_END( _zone_name_ )
	#define AK_INSTRUMENT_SCOPE( _zone_name_ )

	#define AK_INSTRUMENT_BEGIN_C(_colour_, _zone_name_ )

	#define AK_INSTRUMENT_IDLE_BEGIN( _zone_name_ )
	#define AK_INSTRUMENT_IDLE_END( _zone_name_ )
	#define AK_INSTRUMENT_IDLE_SCOPE( _zone_name_ )

	#define AK_INSTRUMENT_STALL_BEGIN( _zone_name_ )
	#define AK_INSTRUMENT_STALL_END( _zone_name_ )
	#define AK_INSTRUMENT_STALL_SCOPE( _zone_name_ )

	#define AK_INSTRUMENT_THREAD_START( _thread_name_ )
#endif

#endif // _AK_TOOLS_COMMON_AKPLATFORMFUNCS_H