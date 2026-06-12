#ifndef __FUCKING_HERE_SHIT_H__
#define __FUCKING_HERE_SHIT_H__ // I think nobody would define a macro like that one ... Like honestly

#if defined(__clang__) || defined(__GNUC__)
#   define FUNC_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#   define FUNC_NAME __FUNCSIG__
#else
#   define FUNC_NAME __func__
#endif

#define STR2(x) #x
#define STR(x) STR2(x)

#define SOURCE_POS __FILE__ ":" STR(__LINE__)

#define here SOURCE_POS

#endif // __FUCKING_HERE_SHIT_H__