#ifndef MACROS_H
#define MACROS_H

#if defined(__GNUC__) || defined(__ghs__)
	#define __REAL_FUNCTION__ __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
  #define __REAL_FUNCTION__ __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define __REAL_FUNCTION__ __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define __REAL_FUNCTION__ __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define __REAL_FUNCTION__ __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define __REAL_FUNCTION__ __func__
#else
	#define #define __REAL_FUNCTION__  \
  #pragma warning Unknown Function Seg Macro REAL_PROFILE_FUNCTION() will not work you can use REAL_PROFILE_SCOPE(name) with the Function Seg as the name 
#endif //defined(__GNUC__) || defined(__ghs__)

#if defined(__linux)
  #if defined(__ANDROID__) 
    #define REAL_PLATFORM_ANDROID
    #error Android is not supported at the moment
  #else 
    #define REAL_PLATFORM_LINUX
  #endif//defined(__ANDROID__)
#elif defined(_WIN32)
  #define REAL_PLATFORM_WINDOWS
  #if defined(_WIN64) 
    #define REAL_PLATFORM_WINDOWS_64
  #endif //defined(_WIN64)
#endif //__LINUX


#define STRINGIFY(x) #x
#define BITSHL(x) (1 << x)
#define BIND_EVENT_FUNCTION(x) std::bind(&x, this, std::placeholders::_1)

#endif //MACROS_H
