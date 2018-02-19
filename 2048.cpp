//
// A 2048 puzzle clone for Windows console.
//

//
// see https://gabrielecirulli.github.io/2048/ for original 2048 that
// is created by Gabriele Cirulli.
//

// TODO: {{{
//  need to check these API (not in order and may change)
//      set_terminate
//      _set_purecall_handler
//      _set_new_handler
//      _set_new_mode
//      _RTC_SetErrorFuncW
//
//      set_unexpected
//      _set_abort_behavior
// }}}

// --- notes on compilation {{{
// reasonable compilation options
// ------------------------------
// VC: [-Zs] -EHa -W4
// GCC: [-fsyntax-only] -Wall -Wextra -Weffc++ -pedantic
// Clang: [-fsyntax-only] -Wall -Wextra -pedantic
//      -Wno-dollar-in-identifier-extension
//      -Wno-gnu-zero-variadic-macro-arguments
//
//
// more stricter compilation options
// ---------------------------------
// VC: [-Zs] -EHa -Wall -wd4820
// GCC: [-fsyntax-only] -Wall -Wextra -Weffc++ -pedantic
//      -Wshadow        -Wsign-conversion
//      -Wunused        -Wmisleading-indentation
//      -Wcast-align    -Wnon-virtual-dtor
//      -Wconversion    -Woverloaded-virtual
//      -Wno-old-style-cast
//
// Clang: [-fsyntax-only -Weverything
//      -Wno-dollar-in-identifier-extension
//      -Wno-gnu-zero-variadic-macro-arguments
//      -Wno-c++98-compat
//      -Wno-c++98-compat-pedantic
//      -Wno-old-style-cast
// end of notes on compilation }}}

// --- notes on indentation {{{
//      mostly resembles "One True Brace Style"
//
//      astyle -A10 -xG -o < input.cpp > indented-file.cpp
//
//      -A10    "One True Brace Style"
//      -xG     --indent-modifiers
//      -o      --keep-one-line-statements
// }}}

// --- preprocessor definitions/workarounds {{{
// NOTE: USE_PRINT_MACRO_ chooses macros for variadic functions (for calling
// printf-family) otherwise uses C++11 variadic templates. With the variadic
// templates the 'format' argument cannot be assessed by compilers, whereas
// macros are good in this case to find errors at compile time.
#define USE_PRINT_MACRO_

// Variable-length arrays not supported by VC++ and by pedantic GCC and Clang
// #define USE_VLA_
#undef USE_VLA_

#define UNICODE
#define _UNICODE

#define APP_DESCRIPTION "Puzzle 2048 for Windows console"
#define APP_VERSION "0.01.6"

#define TEST_

// strsafe.h needs MINGW_HAS_SECURE_API, it can be missing in _mingw.h
#define MINGW_HAS_SECURE_API 1

#define STR(x) #x
#define STR1(x) STR(x)
#define STR2(x) STR(x.)

#define EPRINT($fmt, ...) fprintf(stderr, "%d: " $fmt, __LINE__, ##__VA_ARGS__)
#if defined(USE_PRINT_MACRO_)
#define DPRINT($fmt, ...) DPRINT_STUB("%d: " $fmt, __LINE__, ##__VA_ARGS__)
#define dprint($fmt, ...) DPRINT_STUB($fmt, __VA_ARGS__)
#else
#define DPRINT($fmt, ...) dprint("%d: " $fmt, __LINE__, ##__VA_ARGS__)
#endif

#if defined(__clang__)
#if defined(__GNUC__)
#define CLANG_GCC_
#elif defined(_MSC_VER)
#define CLANG_MSC_
#else
static_assert(UNKNOWN_COMPILER, "this compiler may not be fully supported");
#endif
#elif defined(__GNUC__)
#define GCC_ONLY_
#elif defined(_MSC_VER)
#if defined(_DEBUG) || defined(DEBUG)
#define MSC_DEBUG_
#elif defined(__MSVC_RUNTIME_CHECKS)
#pragma message(__FILE__"(" STR1(__LINE__) ") : " \
                "note: use debugger to get runtime checks messages")
#endif
#define MSC_ONLY_
// #pragma intrinsic
#else
static_assert(UNKNOWN_COMPILER, "this compiler may not be fully supported");
#endif  // __clang__

#if defined(GCC_ONLY_)
#define NOTE_PRAGMA(x) _Pragma(#x)
#define NOTE($M) NOTE_PRAGMA(message($M))
#elif defined(MSC_ONLY_)
// NOTE() macro can jump to location, while INFO() cannot
#define NOTE($M) __pragma(message(__FILE__ "(" STR1(__LINE__) "): note: " $M))
#define INFO($M) __pragma(message(__FILE__ "(?): note: " $M))
#elif defined(__clang__)
#define NOTE(X) _Pragma(STR(GCC warning(X)))
#endif  // __GNUC__

// VC++ dislikes do { } while(0), FALSE_CONDITION tries to mute it
#if defined(MSC_ONLY_)
#define FALSE_CONDITION (void)0,0
#else
#define FALSE_CONDITION 0
#endif

#if defined(_MSC_VER)
#pragma comment(lib, "user32")
#endif

#if defined(__GNUC__) || defined(__clang__)
#define __try (void)0;
#define __except(x) (void)0;
#define __finally (void)0;
#endif  // __GNUC__ || __clang__

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif

#if defined(NDEBUG)
#define VERIFYV /* */
#define VERIFYF($func) /* */
#else
#define VERIFYV VerifierX(__LINE__,__FILE__,NULL) =
#define VERIFYF($func) VerifierX(__LINE__,__FILE__,#$func) =
#endif

// Macro taken from Goocle C++ style
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    void operator=(const TypeName&)

// end of preprocessor definitions }}}

#if defined(MSC_ONLY_)
#if defined(NO_WALL_FILTER)
// -Wall can do its job
#else
// only for system headers, trying to reduce the noise caused by -Wall
#pragma warning(push, 4)
// (4005 4100 4189 4242 4514 4548 4668 4710 4820 4668)
// 4668: 'symbol' is not defined as a preprocessor macro, replacing with '0'
//          for 'directives'
// 4820: 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable: 4668)
#pragma warning(disable: 4820)
#if !defined(CC_STUB_CONFIG) || defined(HAS_WALL)
INFO("some warnings are filtered; mostly from system headers")
INFO("with -Wall, add compiler option" \
     " -DNO_WALL_FILTER to disable filtering")
#endif
#endif  // NO_WALL_FILTER
#endif  // MSC_ONLY_

#include <stdint.h>
#ifdef __MSVC_RUNTIME_CHECKS
#include <rtcapi.h>
#endif

#include <windows.h>

#if defined(MSC_DEBUG_)
#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC_NEW
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
// #undef THIS_FILE
// static char THIS_FILE[] = __FILE__;
#endif  // MSC_DEBUG_

#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <strsafe.h>

#if defined(MSC_ONLY_) && !defined(NO_WALL_FILTER)
// #pragma warning(default: 4820 4668)
#pragma warning(pop)
#endif

//
// N is the board size, default is 4; 3 or 5 also possible.
// if other than 3, 4, or 5 then modify Grid::PatchGrid()
//
enum { N = 4, S8 = sizeof(uint64_t), N64 = ((N * N + S8 - 1) / S8) };
enum { X800 = 11 };  // game target value for 2048
enum { UUS, ROW_L2R_STRIPE, COL_U2D_STRIPE, ROW_R2L_STRIPE, COL_D2U_STRIPE };

BOOL CtrlHandler(DWORD ctrl);
void ErrorInfo(LPCTSTR lpszFunction);

// log routines {{{
#if 0
// #define DPRINT($fmt, ...) debug_out().log("%u: " $fmt, __LINE__, ##__VA_ARGS__)
// struct debug_out {
//     void log(const char* formatstring, ...)
//     {
//         int size;
//         char buf[128];
//         va_list args;
//
//         va_start(args, formatstring);
//         size = vsnprintf_s(buf, sizeof(buf) - 1, _TRUNCATE, formatstring, args);
//
//         if (size < 0) {
//             const char msg[] = "...(msg truncated)";
//             buf[sizeof(buf) - sizeof(msg) - 1] = '\0';
//             strcat_s(buf, sizeof(buf), msg);
//         } else { }
//
//         va_end(args);
//         OutputDebugStringA(buf);
//     }
// };

// template<typename ...Args>
// int cprint(const char* fmt, Args... args)
// {
//     FILE* con_ = NULL;  // fopen("CON", "w");
//     if (fopen_s(&con_, "CON", "w") == 0) {
//         if (con_) {
//             int ret = fprintf(con_, fmt, args...);
//             fclose(con_);
//             con_ = 0;
//             return ret;
//         } else { }
//     } else { }
//
//     // FIXME: if fopen(CON) failed, fallback?
//     return printf(fmt, args...);
// }
#endif

#if defined(USE_PRINT_MACRO_)
#define DPRINT_STUB($fmt, ...) \
    do { \
        int $count_; \
        char buf_[128]; \
        $count_ = snprintf(buf_, sizeof(buf_) - 1, $fmt, ##__VA_ARGS__); \
        if ($count_ < 0) { \
            buf_[sizeof(buf_) - 4] = '?'; \
            buf_[sizeof(buf_) - 3] = '?'; \
            buf_[sizeof(buf_) - 2] = '?'; \
        } else { } \
        buf_[sizeof(buf_) - 1] = '\0'; \
        OutputDebugStringA(buf_); \
    } while (FALSE_CONDITION)
#else
void dprint(const char* str)
{
    if (str && *str) {
        OutputDebugStringA(str);
    } else {
        OutputDebugStringA("???");
    }
}

template<typename ...Args>
void dprint(const char* fmt, Args... args)
{
    int count;
    char buf[128];

    count = snprintf(buf, sizeof(buf) - 1, fmt, args...);

    if (count < 0) {
        buf[sizeof(buf) - 4] = '?';
        buf[sizeof(buf) - 3] = '?';
        buf[sizeof(buf) - 2] = '?';
    } else { }

    buf[sizeof(buf) - 1] = '\0';

    OutputDebugStringA(buf);
}
#endif

// Format a readable error message and display it in a message box
void ErrorInfo(LPCTSTR lpszFunction)
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0, NULL);

    if (lpMsgBuf) {
        size_t buf_size = (size_t)(lstrlen((LPCTSTR)lpMsgBuf) +
                                   lstrlen((LPCTSTR)lpszFunction) + 64);
        buf_size *= sizeof(TCHAR);
        lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, buf_size);

        if (lpDisplayBuf) {
            StringCchPrintf((LPTSTR)lpDisplayBuf,
                            LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                            TEXT("%s failed with error %u: %s"),
                            lpszFunction, dw, (LPTSTR)lpMsgBuf);
            MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

            LocalFree(lpDisplayBuf);
        } else {
            // TODO
        }

        LocalFree(lpMsgBuf);
    } else {
        // TODO
    }
}
// end of log routines }}}

// code defect detectors {{{
#if defined(MSC_DEBUG_)
namespace memleak_reporter
{
#pragma comment(lib, "msvcrtd")
class MemLeakReporter
{
  public:
    MemLeakReporter()
    {
        reset();
        _CrtMemCheckpoint(&ms_start_);
    }

    ~MemLeakReporter()
    {
        reset();

        _CrtMemState ms_cur, ms_diff;
        _CrtMemCheckpoint(&ms_cur);

        if (_CrtMemDifference(&ms_diff, &ms_start_, &ms_cur)) {
            _CrtMemDumpStatistics(&ms_diff);
        } else { }
    }

  private:
    void reset()
    {
        _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(MemLeakReporter);

  private:
    _CrtMemState ms_start_;
};

// The global object mlr automates memleak-reporting.
// Instantiating it here makes main() look clean, but could lead to
// issues like CRT startup routing not yet called.
MemLeakReporter mld; NOTE("MemLeakReporter in use")
}  // namespace memleak_reporter

#if defined(__MSVC_RUNTIME_CHECKS)
namespace runtime_error_reporter
{
// class RunTimeChecker works only when CRT libraries are linked.
// Otherwise the function _CRT_RTC_INITW needs to be defined to set
// the Run-Time-Error-Reporting-Function (which is RTC_ErrorFunc()
// in this code) and should call _RTC_Initialize.
//
// NOTE: In this version, the class RunTimeChecker in compiled when
//       both RTC and DEBUG options are defined.

#pragma runtime_checks("", off)
int RTC_ErrorFunc(int errType, const wchar_t* file, int line,
                  const wchar_t* module, const wchar_t* format, ...)
{
    (void)errType;
    // Prevent re-entrance, just in case if multi-threaded
    static long running = 0;

    while (InterlockedExchange(&running, 1)) {
        Sleep(0);
    }

    // First, get the rtc error number from the var-arg list.
    va_list vl;
    va_start(vl, format);
    _RTC_ErrorNumber rtc_errnum = va_arg(vl, _RTC_ErrorNumber);
    va_end(vl);

    char buf[512] = { };
    const char* err = _RTC_GetErrDesc(rtc_errnum);

    // remove path from 'file'
    if (file && *file) {
        const wchar_t* filename = wcsrchr(file, L'\\');

        if (filename && *++filename) {
            file = filename;
        } else { }
    } else { }

    if (file && *file) {
        // use format is similar to message generated by _CrtMemDumpStatistics
        sprintf_s(buf, 512, "%S(%d): Run-Time Check Failure (type: %d)%s%S: %s",
                  file, line, rtc_errnum,
                  module ? " in " : "",
                  module ? module : L"",
                  err);
    } else {
        // perhaps the PDB is missing, so omitting file and line info.
        sprintf_s(buf, 512, "Run-Time Check Failure (type: %d)%s%S: %s",
                  rtc_errnum,
                  module ? " in " : "",
                  module ? module : L"",
                  err);
    }

    // MessageBoxA(NULL, (LPCSTR)buf, (LPCSTR)"Run Time Failure", MB_OK);
    fprintf(stderr, "%s\n", buf);

    InterlockedExchange(&running, 0);

    return 0;
}

#pragma runtime_checks("", restore)

class RunTimeChecker
{
  public:
    RunTimeChecker()
    {
        _RTC_SetErrorFuncW(RTC_ErrorFunc);
        // _RTC_SetErrorFuncW(_CrtDbgReportW);
    }
    ~RunTimeChecker()
    {
        _RTC_Terminate();
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(RunTimeChecker);
};

// The global object rtc automates runtime error checking/reporting.
// Instantiating it here makes main() look clean, but could lead to
// issues like CRT startup routing not yet called.
RunTimeChecker rtc; NOTE("RunTimeChecker in use")
}  // namespace runtime_error_reporter
#endif  // __MSVC_RUNTIME_CHECKS
#endif  // MSC_DEBUG_
// end of code defect detectors }}}

// error handling routines {{{
void invalid_parameter_handler(const wchar_t* expression,
                               const wchar_t* function,
                               const wchar_t* file,
                               unsigned int line,
                               uintptr_t pReserved)
{
    (void)pReserved;

    wprintf(L"Invalid parameter detected in function %s."
            L" File: %s Line: %u\n", function, file, line);
    wprintf(L"Expression: %s\n", expression);
}

// NOTE: in case of VC++ compiler, the option -EHa is required.
LONG WINAPI UnhandledExceptionFilterFunc(struct _EXCEPTION_POINTERS* pinfo)
{
    static int call_count = 0;

    DWORD code = pinfo->ExceptionRecord->ExceptionCode;
    fprintf(stderr, "UnhandledExceptionFilterFunc:%d %lx\n", call_count, code);

    ++call_count;

    // FIXME: this may not be a good way of exception handling.
    // It works for me with gvim and command prompt, and no debugger!
    if ((code == EXCEPTION_BREAKPOINT) || (code == EXCEPTION_SINGLE_STEP)) {
        fprintf(stderr, "%s\n", "seems debugger present...");

        if (call_count < 2) {
            return EXCEPTION_EXECUTE_HANDLER;
        } else {
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    } else { }

    return EXCEPTION_CONTINUE_SEARCH;
}

class VerifierX
{
  public:
    VerifierX(int line, const char* file, const char* func) :
        line_(line), file_(file), func_(func) { }
    VerifierX() : line_(0), file_(NULL), func_(NULL) { }
    ~VerifierX() { }

    void operator=(HANDLE handle)
    {
        if (handle == INVALID_HANDLE_VALUE) {
            print();
        } else { }
    }

    void operator=(int status)
    {
        if (status != 0) {
            print();
        } else { }
    }

  private:
    void print()
    {
        if (line_) {
            if (func_) {
                fprintf(stderr, "%s:%d: error in '%s': ...\n", file_, line_, func_);
            } else {
                fprintf(stderr, "%s:%d: error: ...\n", file_, line_);
            }
        } else {
            fprintf(stderr, "%s\n", "error");
        }
    }

  private:
    int line_;
    const char* file_;
    const char* func_;
};
// end of error handling routines }}}

// pseudo random number generator {{{
// random number generator influenced by Gray code
class GRNG
{
  public:
    enum { B = 8, M = ((1 << B) - 1) };

  public:
    GRNG() : seed_(0) { }
    explicit GRNG(int seed) : seed_((uint8_t)seed) { }
    ~GRNG() { }
    void Seed(unsigned int seed)
    {
        seed_ = (uint8_t)(seed & 0xff);
    }

    unsigned int operator()(unsigned int range_max)
    {
        unsigned int r = ((*this)());
        r = (uint8_t)(r % range_max);
        // r = (uint8_t)(r * range_max / (1 << B));
        return r;
    }

  private:
    unsigned int operator()(void)
    {
        // NOTE: (seed ^ (seed >> 1)) gives Gray code
        unsigned int r;
        // r = (seed_ ^ ((seed_ << (B - 5)) | (seed_ >> 2))) & M;
        r = (uint8_t)((seed_ ^ ((seed_ << (B - 2)))) & M);
        ++seed_;
        return r;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(GRNG);

  private:
    uint8_t seed_;
};

// random number generator based on c-stdlib
class SRNG
{
  public:
    SRNG() { }
    ~SRNG() { }

    void Seed(unsigned int seed)
    {
        srand(seed & 0xffff);
    }

    unsigned int operator()(unsigned int range_max)
    {
        unsigned int r = (unsigned int)rand();
        // NOTE: below, 1.0 makes the calculation in double
        r = (unsigned int)(r * range_max / (RAND_MAX + 1.0));
        return r;
    }

  private:
    unsigned int operator()()
    {
        int r = rand();
        return (unsigned int)r;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(SRNG);
};
// end of pseudo random number generator }}}

GRNG rng;

// date/time helpers {{{
struct Clock {
  public:
    Clock() { }
    ~Clock() { }

    int64_t Ticks_ms()
    {
        // NOTE: below can be alternatives
        // QueryUnbiasedInterruptTime
        // QueryPerformanceCounter
#ifdef __GNUC__
#define GetTickCount64 GetTickCount
#endif
#ifdef _WIN64
        return (int64_t)GetTickCount64();
#else
        return GetTickCount();
#endif
    }
};

struct Duration {
    int8_t day;
    int8_t hour;
    int8_t min;
    int8_t sec;
    int16_t ms;
};

class Timer
{
  public:
    Timer() : stopped_(true), wait_(0), start_(0), finish_(0) { }
    ~Timer() { }

    void Start()
    {
        stopped_ = false;
        wait_ = 0;
        start_ = Clock().Ticks_ms();
    }

    void Stop()
    {
        if (!stopped_) {
            stopped_ = true;
            finish_ = Clock().Ticks_ms();
        } else { }
    }

    void Continue()
    {
        if (stopped_) {
            stopped_ = false;
            int64_t now = Clock().Ticks_ms();
            wait_ += now - finish_;
        } else { }
    }

    void Pause()
    {
        Stop();
    }

    operator const Duration()
    {
        Duration dur = { 0, 0, 0, 0, 0 };

        if (stopped_) {
        } else {
            finish_ = Clock().Ticks_ms();
        }

        int64_t tmp = finish_ - start_ - wait_;

        if (tmp > 0) {
            // NOTE: casting not needed below, but to suppress warning
            lldiv_t qr = lldiv(tmp, 1000);
            dur.ms = (int16_t)qr.rem;
            qr = lldiv(qr.quot, 60);
            dur.sec = (int8_t)qr.rem;
            qr = lldiv(qr.quot, 60);
            dur.min = (int8_t)qr.rem;
            qr = lldiv(qr.quot, 24);
            dur.hour = (int8_t)qr.rem;
            dur.day = qr.quot & 0x7f;  // TODO: perhaps few days enough
        } else { }

        return dur;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Timer);

  private:
    bool stopped_;
    int64_t wait_;
    int64_t start_;
    int64_t finish_;
};
// end of date/time helpers }}}

// windows console api wrapper {{{
class Console
{
  public:
    enum { LAST_VALUE = 0xffff };
  public:
    Console() : top_(0), height_(-1), interrupted_(0),
        text_attrib_(0), oldcp_(0), oldMode_(0), cursor_size(0),
        input_(INVALID_HANDLE_VALUE), output_(INVALID_HANDLE_VALUE),
        oldout_(INVALID_HANDLE_VALUE), conout_(INVALID_HANDLE_VALUE)
    {
        memset(ct_, 0, sizeof(ct_));
    }
    ~Console()
    {
        FlushConsoleInputBuffer(input_);

        if (conout_ != INVALID_HANDLE_VALUE) {
            CloseHandle(conout_);
        } else { }
    }

    BOOL CtrlHandler(DWORD ctrl)
    {
        BOOL retval = TRUE;
        interrupted_ = 1;

        switch (ctrl) {
        case CTRL_C_EVENT:
            // printf("Ctrl-C event\n\n");
            Beep(750, 300);
            retval = TRUE;
            break;
        case CTRL_BREAK_EVENT:
            Beep(900, 200);
            // printf("Ctrl-Break event\n\n");
            retval = FALSE;
            break;
        case CTRL_CLOSE_EVENT:
            // CTRL-CLOSE: confirm that the user wants to exit.
            Beep(600, 200);
            // printf("Ctrl-Close event\n\n");
            retval = TRUE;
            ++interrupted_;
            break;
        case CTRL_LOGOFF_EVENT:
            Beep(1000, 200);
            // printf("Ctrl-Logoff event\n\n");
            retval = FALSE;
            ++interrupted_;
            break;
        case CTRL_SHUTDOWN_EVENT:
            Beep(750, 500);
            // printf("Ctrl-Shutdown event\n\n");
            retval = FALSE;
            ++interrupted_;
            break;
        default:
            retval = FALSE;
            break;
        }

        HANDLE in = GetStdHandle(STD_INPUT_HANDLE);

        if ((in == INVALID_HANDLE_VALUE) || (in == NULL)) {
            dprint("CtrlHandler(%lu): error(%lu)", ctrl, GetLastError());
            FreeConsole();  // TODO: is FreeConsole() a bad idea?
        } else {
            CloseHandle(in);  // NOTE: forces time out in ReadConsoleInput()
        }

        // TODO: inter-locked-functions may be good interrupted_, since
        // ctrl-handler runs in another thread
        while (interrupted_) {
            Sleep(50);
        }

        return retval;
    }

    void AllowCtrlHandler()
    {
        interrupted_ = 0;
    }

    int CanReset()
    {
        return interrupted_ < 2;
    }

    void Acquire()
    {
        if (_isatty(_fileno(stdout))) {
        } else {
            conout_ = CreateFile(L"CONOUT$",
                                 GENERIC_READ | GENERIC_WRITE,
                                 0, 0, OPEN_EXISTING, 0, 0);
            SetStdHandle(STD_OUTPUT_HANDLE, conout_);
        }

        VERIFYF(GetStdHandle) input_ = GetStdHandle(STD_INPUT_HANDLE);
        VERIFYF(GetStdHandle) output_ = GetStdHandle(STD_OUTPUT_HANDLE);
        oldcp_ = GetConsoleOutputCP();
        SetConsoleOutputCP(65001);
        SaveState();
        GetConsoleMode(input_, &oldMode_);
        DWORD dwNewMode = oldMode_ | ENABLE_MOUSE_INPUT;
        dwNewMode &= ~(DWORD)ENABLE_QUICK_EDIT_MODE;
        SetConsoleMode(input_, dwNewMode | ENABLE_EXTENDED_FLAGS);
        SaveCursor();
        HideCursor();
    }

    void Release()
    {
        SetConsoleMode(input_, oldMode_ | ENABLE_EXTENDED_FLAGS);
        ResetColor();
        SetConsoleOutputCP(oldcp_);
        ShowCursor();
    }

    void SaveBuffer()
    {
        oldout_ = GetStdHandle(STD_OUTPUT_HANDLE);
        output_ = CreateConsoleScreenBuffer(
            GENERIC_READ |            // read/write access
            GENERIC_WRITE,
            FILE_SHARE_READ |
            FILE_SHARE_WRITE,         // shared
            NULL,                     // default security attributes
            CONSOLE_TEXTMODE_BUFFER,  // must be TEXTMODE
            NULL);                    // reserved; must be NULL

        if (output_ == INVALID_HANDLE_VALUE) {
            printf("CreateConsoleScreenBuffer failed - (%lu)\n", GetLastError());
            return;
        }

        if (!SetStdHandle(STD_OUTPUT_HANDLE, output_)) {
            printf("SetStdHandle failed - (%lu)\n", GetLastError());
            return;
        }

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(output_, &csbi);
        csbi.dwSize.Y++;
        // dprint("SaveBuffer: csbi.dwSize.Y=%d", csbi.dwSize.Y);
        SetConsoleScreenBufferSize(output_, csbi.dwSize);

        if (!SetConsoleActiveScreenBuffer(output_)) {
            printf("SetConsoleActiveScreenBuffer failed - (%lu)\n", GetLastError());
            return;
        }

        HideCursor();
    }

    void RestoreBuffer()
    {
        if (!SetConsoleActiveScreenBuffer(oldout_)) {
            printf("SetConsoleActiveScreenBuffer failed - (%lu)\n", GetLastError());
            return;
        }

        CloseHandle(output_);
        output_ = oldout_;
        SetStdHandle(STD_OUTPUT_HANDLE, output_);
        oldout_ = NULL;
    }

    void ShowCursor()
    {
        CONSOLE_CURSOR_INFO info;
        info.dwSize = cursor_size;
        info.bVisible = TRUE;
        SetConsoleCursorInfo(output_, &info);
    }

    void HideCursor()
    {
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 1;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(output_, &info);
    }

    void SaveCursor()
    {
        CONSOLE_CURSOR_INFO cci;
        GetConsoleCursorInfo(output_, &cci);
        cursor_size = cci.dwSize;
    }

    int ReadInput(INPUT_RECORD& inrec)
    {
        switch (WaitForSingleObject(input_, 100)) {
        case WAIT_OBJECT_0: {
            DWORD nEvents;

            if (ReadConsoleInput(input_, &inrec, 1, &nEvents)) {
                if (nEvents > 0) {
                    return 1;
                } else { }
            }
        } break;
        case WAIT_TIMEOUT:
            inrec.EventType = 0x0;
            return 1;
        case WAIT_ABANDONED:
        case WAIT_FAILED:
        default:  // TODO: to check recoverable or not and to act on it
            // dprint("wait for input failed with error code %lu",
            //        (DWORD)GetLastError());
            break;
        }

        inrec.EventType = 0x0;

        if (interrupted_) {
            return 0;
        } else { }

        return -1;
    }

    void MoveTo(unsigned int x, unsigned int y)
    {
        COORD coord = {(SHORT)x, (SHORT)y};
        coord.Y += top_;

        if (coord.Y >= height_) {
            coord.Y = height_;
        }

        SetConsoleCursorPosition(output_, coord);
    }

    void SetColor(unsigned int color)
    {
        SetConsoleTextAttribute(output_, (WORD)color);
    }

    void SaveState()
    {
        // saves color, cursor position and buffer height
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(output_, &csbi);
        text_attrib_ = csbi.wAttributes;
        top_ = csbi.dwCursorPosition.Y;
        height_ = csbi.dwSize.Y;
    }

    short ResetCursorPosition(short top)
    {
        // only y value is handled
        if (top >= 0) {
            return (top_ = (SHORT)top);
        } else {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(output_, &csbi);
            return (top_ = csbi.srWindow.Top);
        }
    }

    void Resize(int rows)
    {
        // only y value is handled
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(output_, &csbi);

        short height = csbi.dwSize.Y;
        short top = csbi.srWindow.Top;
        short bottom = csbi.srWindow.Bottom;

        if ((rows >= csbi.dwMaximumWindowSize.Y) || (rows <= 1)) {
            // TODO: assert, what if dwMaximumWindowSize is less?
            // DPRINT("((rows >= csbi.dwMaximumWindowSize.Y) || (rows <= 1))");
            return;
        }

        if ((bottom - top) >= rows) {
            // console can diplay full grid
            // DPRINT("((bottom - top) >= rows)");
        } else {
            if (height >= (top + rows)) {
                // good
            } else {
                top = height - (SHORT)rows;
            }

            SMALL_RECT srctWindow;
            srctWindow.Top = top;  // move top saved y-top-position
            srctWindow.Bottom = top + (SHORT)rows;
            srctWindow.Left = 0;
            srctWindow.Right = csbi.srWindow.Right - csbi.srWindow.Left;

            SetConsoleWindowInfo(output_, TRUE, &srctWindow);
            // DPRINT("%d %d / %d", top_, srctWindow.Top, srctWindow.Bottom);
        }
    }

    void Clear()
    {
        // only y value is handled
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(output_, &csbi);

        short top = csbi.srWindow.Top;
        short bottom = csbi.srWindow.Bottom;

        for (int i = 0; i < (bottom - top); i++) {
            printf("\n");
        }
    }

    void ResetColor()
    {
        SetConsoleTextAttribute(output_, text_attrib_);
    }

    void FlushInput()
    {
        FlushConsoleInputBuffer(input_);
    }

    void SavePalette()
    {
        GetPalette(ct_);
    }

    void GetOldPalette(COLORREF ct[16])
    {
        memcpy(ct, ct_, sizeof(COLORREF) * 16);
    }

    void GetPalette(COLORREF ct[16])
    {
        CONSOLE_SCREEN_BUFFER_INFOEX sbi;
        ZeroMemory(&sbi, sizeof(sbi));
        sbi.cbSize = sizeof(sbi);

        if (GetConsoleScreenBufferInfoEx(output_, &sbi)) {
            memcpy(ct, sbi.ColorTable, sizeof(COLORREF) * 16);
        } else { }
    }

    void SetPalette(COLORREF ct[16])
    {
        CONSOLE_SCREEN_BUFFER_INFOEX sbi;
        ZeroMemory(&sbi, sizeof(sbi));
        sbi.cbSize = sizeof(sbi);

        RECT r;
        HWND hwnd = GetConsoleWindow();
        GetWindowRect(hwnd, &r);

        if (GetConsoleScreenBufferInfoEx(output_, &sbi)) {
            memcpy(sbi.ColorTable, ct, sizeof(COLORREF) * 16);

            sbi.srWindow.Right++;
            sbi.srWindow.Bottom++;

            SetConsoleScreenBufferInfoEx(output_, &sbi);

            // TODO: remove MoveWindow()/UpdateWindow() and test
            MoveWindow(hwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
            UpdateWindow(hwnd);
        } else { }
    }

    void ResetPalette()
    {
        SetPalette(ct_);
    }

    void SetTitle(const TCHAR* title)
    {
        if (title) {
            SetConsoleTitle(title);
        } else { }
    }

    int Write(unsigned int color, unsigned int x, unsigned int y, const char* str)
    {
        if (x == LAST_VALUE && y == LAST_VALUE) {
            // continue from current position
        } else {
            MoveTo(x, y);
        }

        if (color == LAST_VALUE) {
            // no change in color
        } else {
            SetColor(color);
        }

        return Write(str);
    }

    int Write(unsigned int x, unsigned int y, const char* str)
    {
        if (x == LAST_VALUE && y == LAST_VALUE) {
            // continue from current position
        } else {
            MoveTo(x, y);
        }

        return Write(str);
    }

    int Write(unsigned int color, const char* str)
    {
        if (color == LAST_VALUE) {
            // no change in color
        } else {
            SetColor(color);
        }

        return Write(str);
    }

    int Write(const char* str)
    {
        if (str && *str) {
        } else {
            return 0;  // TODO: assert
        }

        DWORD n;
        int count = (int)strlen(str);

        WriteConsoleA(output_, str, (unsigned int)count, &n, NULL);

        return count;
    }

#if 0
    template<typename ...Args>
    int Write(const char* fmt, Args... args)
    {
        int count;
        char buf[128];

        count = snprintf(buf, sizeof(buf) - 1, fmt, args...);
        buf[sizeof(buf) - 1] = '\0';

        if (count < 0) {
            buf[sizeof(buf) - 2] = '?';
            OutputDebugStringA(buf);
            count = sizeof(buf) - 2;
        } else { }

        DWORD n;

        WriteConsoleA(output_, buf, (unsigned int)count, &n, NULL);

        return count;
    }
#endif

    void CopyRegion(SMALL_RECT& dst, SMALL_RECT& src)
    {
        COORD buf_size;
        COORD buf_coord;

        src.Top += top_;
        src.Bottom += top_;
        dst.Top += top_;
        dst.Bottom += top_;

        buf_size.X = (short)(src.Right - src.Left + 1);
        buf_size.Y = (short)(src.Bottom - src.Top + 1);

#ifndef USE_VLA_
        CHAR_INFO* buf = NULL;

        try {
            // assert ((size_t)(buf_size.X * buf_size.Y)) > 0
            buf = new CHAR_INFO[(size_t)(buf_size.X * buf_size.Y)];
        } catch (...) {
            dprint("%s", "new CHAR_INFO[] failed");
            return;
        }
#else
        CHAR_INFO buf[buf_size.X * buf_size.Y];
#endif

        // The top left destination cell of the CHAR_INFO buf
        buf_coord.X = 0;
        buf_coord.Y = 0;

        // Copy the block from the screen buffer to the buf
        ReadConsoleOutput(output_,    // screen buffer to read from
                          buf,        // buffer to copy into
                          buf_size,   // col-row size of buf
                          buf_coord,  // top left dest. cell in buf
                          &src);      // screen buffer source rectangle

        // Copy from the buf to the screen buffer.
        WriteConsoleOutput(output_,    // screen buffer to write to
                           buf,        // buffer to copy from
                           buf_size,   // col-row size of buf
                           buf_coord,  // top left src cell in buf
                           &dst);      // dest. screen buffer rectangle

#ifndef USE_VLA_
        delete[] buf;
#endif
#undef USE_VLA_
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Console);

  private:
    SHORT top_;
    SHORT height_;
    short interrupted_;
    WORD text_attrib_;
    UINT oldcp_;
    DWORD oldMode_;
    DWORD cursor_size;
    HANDLE input_;
    HANDLE output_;
    HANDLE oldout_;
    HANDLE conout_;
    COLORREF ct_[16];
};
// end of windows console api wrapper }}}

Console con;

BOOL CtrlHandler(DWORD ctrl)
{
    return con.CtrlHandler(ctrl);
}

// math or numerical routines {{{
template<typename T> T Max(T a, T b)
{
    return a > b ? a : b;
}

template<typename T> T Min(T a, T b)
{
    return a < b ? a : b;
}

class Stripe
{
  public:
    Stripe(int type, int pos, uint8_t (&aa)[N][N])
        : begin_(0), step_(0), end_(0),
          type_(type), stripe_(pos), x_(1), a_(aa)
    {
        if ((stripe_ >= 0) && (stripe_ < N)) {
            switch (type_) {
            case ROW_L2R_STRIPE:
                begin_ = 0;
                step_ = 1;
                end_ = N;
                break;
            case COL_U2D_STRIPE:
                begin_ = 0;
                step_ = 1;
                end_ = N;
                break;
            case ROW_R2L_STRIPE:
                begin_ = N - 1;
                step_ = -1;
                end_ = -1;
                break;
            case COL_D2U_STRIPE:
                begin_ = N - 1;
                step_ = -1;
                end_ = -1;
                break;
            default:
                begin_ = 0;
                step_ = 0;
                end_ = 0;
                stripe_ = -1;
                break;
            }
        } else {
            begin_ = 0;
            step_ = 0;
            end_ = 0;
            stripe_ = -1;
        }
    }
    ~Stripe() { }

#if 0
    // code below can be used for debugging the class
    void DbgPrintM()
    {
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (a_[r][c]) {
                    printf("%u", a_[r][c]);
                } else {
                    printf("%s", ".");
                }
            }

            printf("%s", "\n");
        }
    }

    void DbgPrintI()
    {
        switch (type_) {
        case ROW_L2R_STRIPE:
            printf("L(%d): ", stripe_);
            break;
        case COL_U2D_STRIPE:
            printf("U(%d): ", stripe_);
            break;
        case ROW_R2L_STRIPE:
            printf("R(%d): ", stripe_);
            break;
        case COL_D2U_STRIPE:
            printf("D(%d): ", stripe_);
            break;
        default:
            printf("!!!(%d): ", stripe_);
            break;
        }

        DPRINT("b(%d) s(%d) e(%d)\n", begin_, step_, end_);
    }

    void DbgPrintS()
    {
        Stripe& A = *this;
        /*
        switch (type_) {
        case ROW_L2R_STRIPE:
            printf("L(%d) ", stripe_);
            break;
        case COL_U2D_STRIPE:
            printf("U(%d) ", stripe_);
            break;
        case ROW_R2L_STRIPE:
            printf("R(%d) ", stripe_);
            break;
        case COL_D2U_STRIPE:
            printf("D(%d) ", stripe_);
            break;
        default:
            printf("!!!(%d) ", stripe_);
            printf("%s", "invalid\n");
            return;
        }

        for (int i = begin_; i != end_; i += step_) {
            if (A[i]) {
                printf("%u", A[i]);
            } else {
                printf("%s", ".");
            }
        }  // */

        char s[N + 2] = { 0 };

        for (int i = 0; i < N; ++i) {
            s[i] = (char)(A[i] + '0');
        }

        DPRINT("%s", s);
    }
#endif

    bool within(int index, int limit)
    {
        switch (type_) {
        case ROW_L2R_STRIPE: return (index < limit);
        case COL_U2D_STRIPE: return (index < limit);
        case ROW_R2L_STRIPE: return (limit < index);
        case COL_D2U_STRIPE: return (limit < index);
        default: break;
        }

        printf("within(%d %d)\n", index, limit);
        return false;
    }

    template<typename T>
    unsigned int Nudge(T& scorer)
    {
#define A (*this) /* Stripe& A = *this; */
        int b = -1;
        unsigned int m = 0;
        int ec = end_ - step_;

        for (int c = begin_; within(c, ec); c += step_) {
            if (A[c] == 0) {
                b = c;
            } else if (within(c, ec) && A[c + step_] == 0) {
                b = c + step_;
            }

            if (b > -1) {
                for (int i = b, j = i + step_; within(j, end_); j += step_) {
                    if (A[j] == 0) {
                        ec -= step_;
                        continue;
                    } else {
                        A[i] = A[j];
                        A[j] = 0;
                        ++m;  // m += A[i] ? 1 : 0;
                        i += step_;
                    }
                }

                ec -= step_;
                b = -1;
            } else { }

            if (A[c] == A[c + step_]) {
                if (A[c]) {
                    A[c]++;
                    scorer(1 << A[c]);
                    A[c] |= 0x80u;
                    A[c + step_] = 0;  //
                    m += (A[c] == (X800 | 0x80u)) ? 2048u : 1u;
                } else { /* A[c] = 0; */ }  // A[c + step_] = 0;

                int i;

                for (i = c + step_; within(i, ec); i += step_) {
                    A[i] = A[i + step_];
                }

                A[i] = 0;
                ec -= step_;
            } else { }
        }
#undef A
        return m;
    }

    uint8_t& operator[](int i)
    {
        if ((i >= 0) && (i < N)) {
            switch (type_) {
            case ROW_L2R_STRIPE: return a_[stripe_][i];
            case COL_U2D_STRIPE: return a_[i][stripe_];
            case ROW_R2L_STRIPE: return a_[stripe_][i];
            case COL_D2U_STRIPE: return a_[i][stripe_];
            default: DPRINT("Stripe: incorrect type '%d'\n", type_);
                     break;  // TODO: to assert
            }
        } else { }

        // DPRINT("A[%d] ?\n", i);

        return x_;  // TODO: to assert
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Stripe);

  private:
    int begin_;
    int step_;
    int end_;
    int type_;
    int stripe_;
    uint8_t x_;
    uint8_t (&a_)[N][N];
};

class Matrix
{
    //     column 0 1 2 3
    //          +----------> X
    //    row 0 | 0 1 2 3
    //    row 1 | 4 5 6 7
    //    row 2 | 8 9 A B
    //    row 3 | C D E F
    //          v
    //          Y
    //
  public:
    explicit Matrix(uint8_t (&aa)[N][N])
        : aa_(aa)
    {
    }

    ~Matrix() { }

    void Reset(int n)
    {
        switch (n) {
        // case 0:
        //    for (int r = 0; r < N; ++r) {
        //        for (int c = 0; c < N; ++c) {
        //            aa_[r][c] = 0;
        //        }
        //    } break;
        case 1:
            for (int i = 0; i < N * N; ++i) {
                aa_[i >> 2][i & 0x3] = (uint8_t)(i);
            }

            aa_[0][0] = 1;
            break;
        default:
            for (int r = 0; r < N; ++r) {
                for (int c = 0; c < N; ++c) {
                    aa_[r][c] = 0;
                }
            } break;
        }
    }

    uint8_t operator()(unsigned int r, unsigned int c)
    {
        if ((r < N) && (c < N)) {
            return aa_[r][c];
        } else {
            dprint("A[%u][%u] ?\n", r, c);
        }

        return 0;
    }

    void SetAt(unsigned int r, unsigned int c, uint8_t value)
    {
        if ((r < N) && (c < N)) {
            aa_[r][c] = value;
        } else {
            printf("A[%u][%u] ?\n", r, c);
        }
    }

    void DbgPrint()
    {
        fprintf(stderr, "%s", " ");

        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                fprintf(stderr, "%2X", aa_[r][c]);
            }

            fprintf(stderr, "%s", " ");
        }

        fprintf(stderr, "%s", "\n");
        fflush(stderr);
    }

    void Transpose()
    {
        for (int r = 0; r <= N - 2; ++r) {
            for (int c = r + 1; c <= N - 1; ++c) {
                Swap(aa_[r][c], aa_[c][r]);
            }
        }
    }

    void SwapV()
    {
        for (int r = 0; r < N / 2; ++r) {
            for (int c = 0; c < N; ++c) {
                Swap(aa_[r][c], aa_[N - 1 - r][c]);
            }
        }
    }

    void SwapH()
    {
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N / 2; ++c) {
                Swap(aa_[r][c], aa_[r][N - 1 - c]);
            }
        }
    }

    void RotateCW()
    {
        Transpose();
        SwapH();
    }

    void RotateCCW()
    {
        Transpose();
        SwapV();
    }

  private:
    void Swap(uint8_t& a, uint8_t& b)
    {
        uint8_t t = a;
        a = b;
        b = t;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Matrix);

  private:
    uint8_t (&aa_)[N][N];
};
// end of math or numerical routines }}}

class Grid
{
  public:
    // grid/message offsets of x and y
    enum {
        GRID_X = 5,
        GRID_Y = 2,
        MESG_X = GRID_X + 9 * N + 5,
        MESG_Y = 14
    };
  public:
    Grid()
#if defined(_MSC_VER) && (_MSC_VER < 1800)
        // not supported ?
#else
        : text_{ }
#endif
    {
#if defined(_MSC_VER) && (_MSC_VER < 1800)
        memset(&text_, 0, sizeof(text_));
#endif
        SetGridMode(1);

        for (int i = 0; i < (int)(sizeof(ct) / sizeof(ct[0])); ++i) {
            ct[i] = 0x0u;
        }
    }

    ~Grid() { }

    void DrawGrid()
    {
        //
        //  Board layout
        //  ------------
        //
        //  (GRID_X,GRID_Y) = (5,2)
        //    \           1         2         3
        //     \0123456789012345678901234567890123456
        //    0 +--------+--------+--------+--------+
        //    1 |.      .|.      .|.      .|.      .|
        //    2 |.   2  .|.  64  .|. 128  .|. 1024 .|
        //    3 |.      .|.      .|.      .|.      .|
        //    4 +--------+--------+--------+--------+
        //    5 |        |        |        |        |
        //    6 |        |        |        |        |
        //    7 |        |        |        |        |             1         2
        //    8 +--------+--------+--------+--------+   0123456789012345678901
        //    9 |        |        |        |        |
        //   10 |        |        |        |        |  (MESG_X,MESG_Y) = (46,14)
        //    1 |        |        |        |        |    /
        //    2 +--------+--------+--------+--------+   +--------------------+
        //    3 |        |        |        |        |   |      You WON!      |
        //    4 |        |        |        |        |   +--------------------+
        //    5 |        |        |        |        |
        //    6 +--------+--------+--------+--------+   > Keep Going: Yes / No
        //
        //  Board drawing characters
        //  ------------------------
        //     ┏━━━┯━━━┓   ╔═══╗  ┏━━━┓  ┌───┐
        //     ┃   │   ┃   ║   ║  ┃   ┃  │   │   ___▌___▐___
        //     ┠───┼───┨   ╚═══╝  ┗━━━┛  └───┘
        //     ┃   │   ┃
        //     ┗━━━┷━━━┛
        //

        con.MoveTo(0, 0);

        con.Write(0x08u, GRID_X, GRID_Y, text_.grid_top_line);

        unsigned int y;

        for (y = 1; y < 4 * N; ++y) {
            con.MoveTo(GRID_X, GRID_Y + y);

            switch (y) {
            case 4: case 8: case 12: case 16:
                con.Write(text_.grid_sep_line);
                break;
            default:
                con.Write(text_.grid_mid_line);
            }
        }

        con.MoveTo(GRID_X, GRID_Y + y);
        con.Write(text_.grid_bot_line);

        PatchGrid(N);

        con.Write(MESG_X, 4, "Score      Time");
    }

    void PatchGrid(int grid_size)
    {
        // PatchGrid is used just to experiment copying regions of console
        switch (grid_size) {
        case 3: {
            SMALL_RECT src = { 41, 2, 50, 14 };
            SMALL_RECT dst = { 32, 2, 41, 14 };
            con.CopyRegion(dst, src);
        } break;
        case 5: {
            SMALL_RECT src = { 32, 2, 41, 22 };
            SMALL_RECT dst = { 41, 2, 50, 22 };
            con.CopyRegion(dst, src);
        } break;
        default:
            break;
        }
    }

    void SetGridMode(int mode)
    {
        switch (mode) {
        case 0:
            text_.grid_top_line = "+--------+--------+--------+--------+";
            text_.grid_sep_line = "+--------+--------+--------+--------+";
            text_.grid_mid_line = "|        |        |        |        |";
            text_.grid_bot_line = "+--------+--------+--------+--------+";
            text_.cell_left_pad = " ";
            text_.cell_right_pad = " ";
            text_.won_top_line = " +====================+ ";
            text_.won_mid_line = " |                    | ";
            text_.won_bot_line = " +====================+ ";
            text_.lost_top_line = " +--------------------+ ";
            text_.lost_mid_line = " |                    | ";
            text_.lost_bot_line = " +--------------------+ ";
            text_.prompt_char = ">";
            text_.underline = " ---- ";
            text_.filler_line = "      ";
            break;
        default:
            text_.grid_top_line = "┏━━━━━━━━┯━━━━━━━━┯━━━━━━━━┯━━━━━━━━┓";
            text_.grid_sep_line = "┠────────┼────────┼────────┼────────┨";
            text_.grid_mid_line = "┃        │        │        │        ┃";
            text_.grid_bot_line = "┗━━━━━━━━┷━━━━━━━━┷━━━━━━━━┷━━━━━━━━┛";
            text_.cell_left_pad = "▌";
            text_.cell_right_pad = "▐";
            text_.won_top_line = " ╔════════════════════╗ ";
            text_.won_mid_line = " ║                    ║ ";
            text_.won_bot_line = " ╚════════════════════╝ ";
            text_.lost_top_line = " ┏━━━━━━━━━━━━━━━━━━━━┓ ";
            text_.lost_mid_line = " ┃                    ┃ ";
            text_.lost_bot_line = " ┗━━━━━━━━━━━━━━━━━━━━┛ ";
            text_.prompt_char = "►";
            text_.underline = " ──── ";
            text_.filler_line = "      ";
            break;
        }
    }

    void ShowMessage(bool won)
    {
        con.SetColor((won ? 0xcfu : 0x70u));
        con.MoveTo(MESG_X, MESG_Y + 0);
        con.Write(won ? text_.won_top_line : text_.lost_top_line);
        con.MoveTo(MESG_X, MESG_Y + 1);
        con.Write(won ? text_.won_mid_line : text_.lost_mid_line);
        con.MoveTo(MESG_X, MESG_Y + 2);
        con.Write(won ? text_.won_bot_line : text_.lost_bot_line);

        con.MoveTo(MESG_X + 7, MESG_Y + 1);
        con.Write(won ? " You WON! " : "Game Over!");

        con.MoveTo(MESG_X, MESG_Y + 4);
        con.SetColor(0xfu);
        con.Write(text_.prompt_char);
        con.SetColor(0x7u);
        con.Write(won ? " Keep Going? " : " Play Again? ");
        con.SetColor(0x8fu);
        con.Write(" Y");
        con.SetColor(0x80u);
        con.Write("es ");
        con.SetColor(0x8u);
        con.Write(" / ");
        con.SetColor(0x7u);
        con.Write("N");
        con.SetColor(0x8u);
        con.Write("o");
    }

    void ClearMessage()
    {
        con.SetColor(07);
        con.MoveTo(MESG_X, MESG_Y + 0);
        //        " +--------------------+ ");
        con.Write("                        ");
        con.MoveTo(MESG_X, MESG_Y + 1);
        con.Write("                        ");
        con.MoveTo(MESG_X, MESG_Y + 2);
        con.Write("                        ");

        con.MoveTo(MESG_X, MESG_Y + 4);
        //        ". Keep Going? .Yes. / No");
        con.Write("                        ");
    }

    void ShowCell(unsigned int n, unsigned int r, unsigned int c, bool highlight = true)
    {
        if ((r < N) && (c < N)) {
        } else {
            return;
        }

        unsigned int i;

        if (highlight) {
            i = 0x70;
        } else {
            i = GetColor(n & 0x7fu);
        }

        unsigned int x = 6 + 9 * c;
        unsigned int y = 3 + 4 * r;
        con.MoveTo(x, y);
        PrintLine(i, text_.filler_line);
        con.MoveTo(x, y + 1);
        PrintNumber(n, i);
        con.MoveTo(x, y + 2);
        PrintLine(i, text_.filler_line);
    }

    void ShowMatrix(Matrix& matrix)
    {
        for (unsigned int r = 0; r < N; ++r) {
            unsigned int y = 3 + 4 * r;

            for (unsigned int c = 0; c < N; ++c) {
                unsigned int x = 6 + 9 * c;

                unsigned int n = matrix(r, c);
                unsigned int i = GetColor(n & 0x7fu);

                con.MoveTo(x, y);
                PrintLine(i, text_.filler_line);
                con.MoveTo(x, (y + 1));
                PrintNumber(n, i);
                con.MoveTo(x, (y + 2));
                PrintLine(i, (n & 0x80u) ? text_.underline : text_.filler_line);
            }
        }
    }

    void PrintNumber(unsigned int n, unsigned int i)
    {
        switch (n & 0x7fu) {
        case 0:  PrintLine(i, "   0  "); break;
        case 1:  PrintLine(i, "   2  "); break;
        case 2:  PrintLine(i, "   4  "); break;
        case 3:  PrintLine(i, "   8  "); break;
        case 4:  PrintLine(i, "  16  "); break;
        case 5:  PrintLine(i, "  32  "); break;
        case 6:  PrintLine(i, "  64  "); break;
        case 7:  PrintLine(i, "  128 "); break;
        case 8:  PrintLine(i, "  256 "); break;
        case 9:  PrintLine(i, "  512 "); break;
        case 10: PrintLine(i, " 1024 "); break;
        case 11: PrintLine(i, " 2048 "); break;
        case 12: PrintLine(i, " 4096 "); break;
        case 13: PrintLine(i, " 8192 "); break;
        case 14: PrintLine(i, "  16K "); break;
        case 15: PrintLine(i, "  32K "); break;
        case 16: PrintLine(i, "  64K "); break;
        case 17: PrintLine(i, " 128K "); break;
        case 18: PrintLine(i, " 256K "); break;
        case 19: PrintLine(i, " 512K "); break;
        case 20: PrintLine(i, "  1M  "); break;
        case 21: PrintLine(i, "  2M  "); break;
        case 22: PrintLine(i, "  4M  "); break;
        case 23: PrintLine(i, "  8M  "); break;
        case 24: PrintLine(i, "  16M "); break;
        case 25: PrintLine(i, "  32M "); break;
        case 26: PrintLine(i, "  64M "); break;
        case 27: PrintLine(i, " 128M "); break;
        case 28: PrintLine(i, " 256M "); break;
        case 29: PrintLine(i, " 512M "); break;
        default: PrintLine(i, "  ??  "); break;
        }
    }

#if 0
    // old code with macros
    void PrintNumber(unsigned int n, unsigned int i)
    {
        unsigned int v = (n & 0x7fu);
        v = (v ? (1u << v) : v);
#define CASE1(n) case n
#define CASE3(n) case n: case n + 1: case n + 2
#define CASE4(n) case n: case n + 1: case n + 2: case n + 3
#define k ((v/1024))
#define m ((v/1024/1024))

        switch (n & 0x7fu) {
        CASE1( 0): PRINTLINE(i,"   %u  ", 0u); break;  // 0       > |   0  |
        CASE3( 1): PRINTLINE(i,"   %u  ", v); break;  // 2-8      > |   #  |
        CASE3( 4): PRINTLINE(i,"  %u  ", v); break;  // 16-64     > |  ##  |
        CASE3( 7): PRINTLINE(i,"  %u ", v); break;  // 128-512    > |  ### |
        CASE4(10): PRINTLINE(i," %u ", v); break;  // 1024-8192   > | #### |
        CASE3(14): PRINTLINE(i,"  %uK ", k); break;  // 16K-64K   > |  ### |
        CASE3(17): PRINTLINE(i," %uK ", k); break;  // 128K-512K  > | #### |
        CASE4(20): PRINTLINE(i,"  %uM  ", m); break;  // 1M-8M    > |  ##  |
        CASE3(24): PRINTLINE(i,"  %uM ", m); break;  // 16M-64M   > |  ### |
        CASE3(27): PRINTLINE(i," %uM ", m); break;  // 128M-512M  > | #### |
        default: PRINTLINE(i,"%s", "  ??  "); break;  // overflow > |  ??  |
        }
#undef CASE1
#undef CASE3
#undef CASE4
#undef k
#undef m
    }
#endif

    void PrintLine(unsigned int color, const char* value)
    {
        con.SetColor(color & 0xf0u);
        con.Write(text_.cell_left_pad);
        con.SetColor(color);
        con.Write(value);
        con.SetColor(color & 0xf0u);
        con.Write(text_.cell_right_pad);
    }

    unsigned int GetColor(unsigned int value)
    {
        unsigned int n = value;

        uint8_t const cid[] = {
            // 0     2     4     8    16    32    64   128   256   512
            0x08, 0x9f, 0x6f, 0x5f, 0x4f, 0x4f, 0x3f, 0x3f, 0x2f, 0x2f,
            0x17, 0xcf, 0xdf, 0xb0, 0xe6, 0xe2, 0xf0, 0xfc, 0xfc, 0xfc,
            0xac  // 2k    4k    8k
        };

        if (n < (sizeof(cid) / sizeof(cid[0]))) {
            return cid[n];
        } else {
            return cid[(sizeof(cid) / sizeof(cid[0])) - 1];
        }
    }

    void SetColorScheme(int s)
    {
        COLORREF cto[16];
        con.GetOldPalette(cto);

        switch (s) {
        case 1:
            ct[0]  = 0x00182028;
            ct[1]  = 0x00133af1;  // #f13a13
            ct[2]  = 0x000068ff;  // #ff6800
            ct[3]  = 0x005c7aff;  // #ff7a5c
            ct[4]  = 0x00008eff;  // #ff8e00
            ct[5]  = 0x0000b3ff;  // #ffb300
            ct[6]  = 0x0000c8f4;  // #f4c800
            ct[7]  = cto[7];
            ct[8]  = cto[8];
            ct[9]  = 0x0062a2ce;  // #cea262
            ct[10] = 0x0000aa93;  // #93aa00
            ct[11] = 0x00347d00;  // #007d34
            ct[12] = 0x001000ff;  // #ff0010
            ct[13] = 0x008a5300;  // #00538a
            ct[14] = 0x007a3753;  // #53377a
            ct[15]  = cto[15];
            break;
        case 2:
            ct[0]  = 0x00251810;
            ct[1]  = 0x00f990c4 - 0x444444;  // #c490f9
            ct[2]  = 0x00f99090 - 0x444444;  // #9090f9
            ct[3]  = 0x00f9c490 - 0x444444;  // #90c4f9
            ct[4]  = 0x00f9f990 - 0x444444;  // #90f9f9
            ct[5]  = 0x00c4f990 - 0x444444;  // #90f9c4
            ct[6]  = 0x0090f990 - 0x444444;  // #90f990
            ct[7]  = cto[7];
            ct[8]  = cto[8];
            ct[9]  = 0x0090f9c4 - 0x444444;  // #c4f990
            ct[10] = 0x0090f9f9 - 0x444444;  // #f9f990
            ct[11] = 0x0090c4f9 - 0x444444;  // #f9c490
            ct[12] = 0x009090f9 - 0x444444;  // #f99090
            ct[13] = 0x00f990f9 - 0x444444;  // #f990f9
            ct[14] = 0x00c490f9 - 0x444444;  // #f990c4
            ct[15]  = cto[15];
            break;
#if 0
        case 3:
            ct[0]  = 0x000000u + 0x201010u;  //      0
            ct[1]  = 0x800000u + 0x002828u;  // 800000
            ct[2]  = 0x008000u + 0x280028u;  //   8000
            ct[3]  = 0x808000u + 0x000028u;  // 808000
            ct[4]  = 0x000080u + 0x282800u;  //     80
            ct[5]  = 0x800080u + 0x002800u;  // 800080
            ct[6]  = 0x008080u + 0x280000u;  //   8080
            ct[7]  = 0xc0c0c0u - 0x000000u;  // c0c0c0
            ct[8]  = 0x808080u - 0x282828u;  // 808080
            ct[9]  = 0xff0000u - 0x330000u + 0x002222u;  // ff0000
            ct[10] = 0x00ff00u - 0x003300u + 0x220022u;  //   ff00
            ct[11] = 0xffff00u - 0x333300u + 0x000022u;  // ffff00
            ct[12] = 0x0000ffu - 0x000022u + 0x222200u;  //     ff
            ct[13] = 0xff00ffu - 0x330022u + 0x002200u;  // ff00ff
            ct[14] = 0x00ffffu - 0x003322u + 0x220000u;  //   ffff
            ct[15] = 0xffffffu - 0x111111u;
            break;
#endif
        case 3:
            ct[0]  = 0x222827u;
            ct[1]  = 0x9e5401u;
            ct[2]  = 0x04aa74u;
            ct[3]  = 0xa6831au;
            ct[4]  = 0x3403a7u;
            ct[5]  = 0x9c5689u;
            ct[6]  = 0x49b6b6u;
            ct[7]  = 0xcacacau;
            ct[8]  = 0x7c7c7cu;
            ct[9]  = 0xf58303u;
            ct[10] = 0x06d08du;
            ct[11] = 0xe5c258u;
            ct[12] = 0x4b04f3u;
            ct[13] = 0xb87da8u;
            ct[14] = 0x81ccccu;
            ct[15] = 0xffffffu;
            break;
        default:
            return;
        }

        con.SetPalette(ct);
    }

    void ShowScore(const int score)
    {
        char buf[32] = { };
        int unused = snprintf(buf, sizeof(buf) - 1, "%-8d ", score);
        (void)unused;  // TODO: assert?

        con.Write(0xfu, MESG_X, 5, buf);
    }

    void ShowTime(Duration dur, bool show_ms = false)
    {
        int unused;
        char buf[32] = { };
        con.SetColor((show_ms ? 0x7u : 0x8u));
        con.MoveTo(MESG_X + 10, 5);

        if (dur.day > 0) {
            unused = snprintf(buf, sizeof(buf) - 1, " %d day%s %02d",
                              dur.day, (dur.day > 1 ? "s" : ""), dur.hour);
            (void)unused;  // TODO: assert?
            con.Write(buf);
        } else {
            if (dur.hour > 0) {
                unused = snprintf(buf, sizeof(buf) - 1, " %02d", dur.hour);
                (void)unused;  // TODO: assert?
                con.Write(buf);
            } else {
                con.Write(" ..");
            }
        }

        unused = snprintf(buf, sizeof(buf) - 1, ":%02d:%02d", dur.min, dur.sec);
        (void)unused;  // TODO: assert?
        con.Write(buf);

        if (show_ms) {
            unused = snprintf(buf, sizeof(buf) - 1, ".%03d ", dur.ms);
            (void)unused;  // TODO: assert?
            con.Write(buf);
        } else {
            con.Write("     ");
        }
    }

    int GetMinHeight()
    {
        enum {
            BOARD_HEIGHT = GRID_Y + N * 4 + 1
        };
        return BOARD_HEIGHT;
    }

  private:
    struct TextData {
        const char* grid_top_line;
        const char* grid_sep_line;
        const char* grid_mid_line;
        const char* grid_bot_line;
        const char* cell_left_pad;
        const char* cell_right_pad;
        const char* won_top_line;
        const char* won_mid_line;
        const char* won_bot_line;
        const char* lost_top_line;
        const char* lost_mid_line;
        const char* lost_bot_line;
        // const char* won_message;
        // const char* lost_message;
        // const char* won_question;
        // const char* lost_question;
        const char* prompt_char;
        const char* filler_line;
        const char* underline;
    };

  private:
    DISALLOW_COPY_AND_ASSIGN(Grid);

  private:
    TextData text_;
    COLORREF ct[16];
};

union Board4x4 {
    uint8_t ac[N][N];
    uint64_t al[N64];  // since N = 4
};

static_assert((sizeof(Board4x4) >= N * N), "Board4x4: error in size");

enum {
    GAME_ERROR = -1,
    GAME_NOOP = 0,
    GAME_ABORT,
    GAME_STOP,
    GAME_LOST,
    GAME_WON,
    GAME_PERSIST,
    GAME_RESTART,
    GAME_TIMER,
    GAME_UNDO,

    MOVE_LEFT = 0x10,
    MOVE_UP,
    MOVE_RIGHT,
    MOVE_DOWN,
    MOVE_MOUSE_WHEEL_FW = 0x20,
    MOVE_MOUSE_WHEEL_BW,
    MOVE_MOUSE_WHEEL_FW_SHIFT,
    MOVE_MOUSE_WHEEL_BW_SHIFT,

    BOARD_REFRESH = 0x40,
    BOARD_TRANSPOSE,
    BOARD_ROTATE_CW,
    BOARD_ROTATE_CCW,
    BOARD_SWAP_VERTICAL,
    BOARD_SWAP_HORIZONTAL,
};

class Mapper
{
  public:
    Mapper(int y_top = 0) : r(-1), c(-1), top(y_top) { }
    ~Mapper() { }

    int FindCell(int x, int y)
    {
        enum {
            W = 9,
            H = 4,
            NO_X1 = Grid::MESG_X + 22,
            NO_X2 = Grid::MESG_X + 23,
            YES_X1 = Grid::MESG_X + 14,
            YES_X2 = Grid::MESG_X + 18,
        };

        if (y == Grid::MESG_Y + 4 + top) {
            if ((x >= YES_X1) && (x <= YES_X2)) {
                // DPRINT("mouse yes");
                return GAME_PERSIST;
            } else if ((x >= NO_X1) && (x <= NO_X2)) {
                // DPRINT("mouse no");
                return GAME_STOP;
            } else {
                return 0;
            }
        } else { }

        x -= Grid::GRID_X;
        y -= Grid::GRID_Y + top;

        if ((x <= 0) || (y <= 0) || (x >= N * W) || (y >= N * H)) {
            r = c = -1;
            return 0;
        } else { }

        switch (x % W) {
        case 0:
        case 1:
        case W - 1:
            return 0;
        default:
            c = x / W;
        }

        switch (y % H) {
        case 0:
            return 0;
        default:
            r = y / H;
        }

        // DPRINT("mouse cell: %3d %3d", c, r);

        return (0x10000 + r * 0x100 + c);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Mapper);

  private:
    int r;
    int c;
    int top;
    // previous position (xc,xr) is needed for tracing mouse path
    // int xr;
    // int xc;
};

class TimeKeeper
{
  public:
    explicit TimeKeeper(Grid& g) : show_ms_(false), hash_(-1), timer_(), grid_(g) { }
    ~TimeKeeper() { }

    void operator()()
    {
        Update();
    }

    void Update()
    {
        const Duration dur = timer_;

        if (show_ms_) {
            if (dur.ms + 1000 == hash_) {
                return;
            } else {
                hash_ = dur.ms + 1000;
            }
        } else  {
            if (dur.sec == hash_) {
                return;
            } else {
                hash_ = dur.sec;
            }
        }

        grid_.ShowTime(timer_, show_ms_);
    }

    void Update(bool show_ms)
    {
        show_ms_ = show_ms;
        Update();
    }

    void Start()
    {
        show_ms_ = false;
        hash_ = -1;
        timer_.Start();
    }

    void Stop()
    {
        show_ms_ = true;
        timer_.Stop();
    }

    void Continue()
    {
        show_ms_ = false;
        hash_ = -1;
        timer_.Continue();
        Update();
    }

    void Pause()
    {
        show_ms_ = true;
        timer_.Pause();
        Update(true);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(TimeKeeper);

  private:
    bool show_ms_;
    int hash_;
    Timer timer_;
    Grid& grid_;
};

class Puzzle2048
{
  public:
    Puzzle2048()
        : old_score_(0), score_(), grid_(), time_keeper_(grid_),
#if defined(_MSC_VER) && (_MSC_VER < 1800)
          // not supported ?
#else
          undo_{ }, board_{ },
#endif
          matrix(board_.ac)
    {
#if defined(_MSC_VER) && (_MSC_VER < 1800)
        memset(&undo_, 0, sizeof(undo_));
        memset(&board_, 0, sizeof(undo_));
#endif
        matrix.Reset(0);
    }
    ~Puzzle2048() { }

    int Play(int s, int d)
    {
        unsigned int m = 1;
        int k = GAME_NOOP;
        int state = 0;
        unsigned int rand_row = 0;
        unsigned int rand_col = 0;
        bool highlight = false;

        short y_top = InitConsole(s, d);
        InputReader ir(y_top);
        Start2048();

        for (; k == GAME_NOOP; k = ir.GetInput()) {
            // no-op
        }

        time_keeper_.Start();

        for (; k != GAME_ABORT; k = ir.GetInput(time_keeper_)) {
            switch (k) {
            case GAME_ERROR:  // TODO: report error and exit or try recover?
            case GAME_NOOP:
                continue;
            case GAME_ABORT:
                time_keeper_.Update(true);
                break;
            case GAME_UNDO:
                highlight = false;
                Undo();

                if (state) {
                    grid_.ClearMessage();
                    state = 0;
                } else { }

                time_keeper_.Continue();
                grid_.ShowScore(score_);
                grid_.ShowMatrix(matrix);
                continue;
            default:
                if (k >= 0x10000) {  // cheating ...
                    int r = (k / 0x100) & 0xff;
                    int c = k & 0xff;
                    uint8_t cur = 0;

                    if ((r - 1) >= 0) {
                        cur = Max<uint8_t>(cur, matrix(r - 1, c));
                    } else { }

                    if ((r + 1) < N) {
                        cur = Max<uint8_t>(cur, matrix(r + 1, c));
                    } else { }

                    if ((c - 1) >= 0) {
                        cur = Max<uint8_t>(cur, matrix(r, c - 1));
                    } else { }

                    if ((c + 1) < N) {
                        cur = Max<uint8_t>(cur, matrix(r, c + 1));
                    } else { }

                    if (cur && cur > matrix(r, c)) {
                        matrix.SetAt(r, c, cur);
                    } else { }

                    grid_.ShowMatrix(matrix);
                } else { }
                break;
            }

            if (state) {
                if (k == GAME_STOP) {
                    time_keeper_.Pause();
                    break;
                } else {
                    if (((state & 0x20) && ((state & 0x0f) > 0)) ||
                        ((state & 0x10) && ((state & 0x0f) > 1))) {
                        k = GAME_PERSIST;
                    } else { }
                }

                switch (k) {
                case GAME_PERSIST:
                    grid_.ClearMessage();

                    if ((state & 0xf0) == 0x10) {
                        Start2048();
                    } else {
                        time_keeper_.Continue();
                        AddNew(rand_row, rand_col);
                    }

                    state = 0;
                    break;
                case GAME_STOP:
                    break;  // see above
                case MOVE_LEFT:
                case MOVE_RIGHT:
                case MOVE_UP:
                case MOVE_DOWN:
                    ++state;
                    continue;
                default:
                    continue;
                }
            } else {
                if (m) {
                    Save();
                } else { }

                m = 0;
                time_keeper_.Update(false);

                switch (k) {
                case GAME_RESTART:
                    Start2048();
                    grid_.ShowMatrix(matrix);
                    continue;
                case BOARD_REFRESH:
                    grid_.DrawGrid();
                    grid_.ShowMatrix(matrix);
                    grid_.ShowScore(score_);
                    time_keeper_.Update();
                    continue;
                case BOARD_TRANSPOSE:
                    matrix.Transpose();
                    grid_.ShowMatrix(matrix);
                    continue;
                case BOARD_ROTATE_CW:
                    matrix.RotateCW();
                    grid_.ShowMatrix(matrix);
                    continue;
                case BOARD_ROTATE_CCW:
                    matrix.RotateCCW();
                    grid_.ShowMatrix(matrix);
                    continue;
                case BOARD_SWAP_VERTICAL:
                    matrix.SwapV();
                    grid_.ShowMatrix(matrix);
                    continue;
                case BOARD_SWAP_HORIZONTAL:
                    matrix.SwapH();
                    grid_.ShowMatrix(matrix);
                    continue;
#ifdef TEST_
                case GAME_WON:
                    state = 0x20;
                    time_keeper_.Pause();
                    grid_.ShowMessage(true);  // won
                    continue;
                case GAME_LOST:
                    state = 0x10;
                    time_keeper_.Pause();
                    grid_.ShowMessage(false);  // lost
                    continue;
#endif
                default:
                    break;
                }

                switch (k) {
                case MOVE_LEFT:
                case MOVE_MOUSE_WHEEL_FW:
                    m = Nudge(ROW_L2R_STRIPE);
                    break;
                case MOVE_RIGHT:
                case MOVE_MOUSE_WHEEL_FW_SHIFT:
                    m = Nudge(ROW_R2L_STRIPE);
                    break;
                case MOVE_UP:
                case MOVE_MOUSE_WHEEL_BW_SHIFT:
                    m = Nudge(COL_U2D_STRIPE);
                    break;
                case MOVE_DOWN:
                case MOVE_MOUSE_WHEEL_BW:
                    m = Nudge(COL_D2U_STRIPE);
                    break;
                default:
                    break;
                }

                if (m >= 2048) {
                    state = 0x20;
                    time_keeper_.Pause();
                    grid_.ShowScore(score_);
                    grid_.ShowMessage(true);  // won
                    grid_.ShowMatrix(matrix);
                    ResetHighlight();
                    continue;
                } else { }

                if (m > 0) {
                    grid_.ShowScore(score_);
                    grid_.ShowMatrix(matrix);
                    m = AddNew(rand_row, rand_col);
                    highlight = (m > 0);
                } else {
                    if (highlight) {
                        grid_.ShowCell(matrix(rand_row, rand_col), rand_row, rand_col, false);  // reset
                        highlight = false;
                    } else { }
                }

                if (IsSolvable()) {
                } else {
                    m = 0;
                    state = 0x10;
                    time_keeper_.Pause();
                    grid_.ShowMessage(false);  // lost
                }
            }
        }

        ResetConsole(s);

        return 1;
    }

    int SchemeTest(int s, int d)
    {
        int unused;
        COLORREF ct[16];
        InitConsole(s, d);
        con.GetPalette(ct);

        con.Write("\n Press ESC key to exit\n\n");

        char buf[16] = { };

        for (unsigned int i = 0; i < 16; ++i) {
            con.Write(" ");

            for (unsigned int j = 0; j < 16; ++j) {
                unsigned int color = ((i * 16) + j) & 0xffu;
                unused = snprintf(buf, sizeof(buf) - 1, " %2x ", color);
                (void)unused;  // TODO: assert?
                con.Write(color, buf);
            }

            unused = snprintf(buf, sizeof(buf) - 1, " - %06lx\n", ct[i]);
            (void)unused;  // TODO: assert?
            con.Write(0x07u, buf);
        }

        int k;
        InputReader ir;

        while ((k = ir.GetInput()) != GAME_ABORT) {
            Sleep(0);
        }

        con.Write("\n");
        ResetConsole(s);
        return 0;
    }

    int GridTest(int g, int s, int d)
    {
        InitConsole(s, d);

        switch (g) {
        case 1:
            Preset(-1);
            board_.ac[0][0] = 0;
            board_.ac[0][1] = 1;
            break;
        case 2:
            Preset(N * N - N - 1);
            break;
        case 3:
            Preset(N * N + 1);
            break;
        default:
            matrix.Reset(0);
            break;
        }

        grid_.DrawGrid();
        grid_.ShowMatrix(matrix);

        if (g == 1) {
            grid_.ShowCell(matrix(0, 1), 0, 1);
        } else { }

        con.MoveTo(48, 10);
        con.SetColor(0x70u);
        con.Write(" Press ESC to exit ");

        int k;
        InputReader ir;

        while ((k = ir.GetInput()) != GAME_ABORT) {
            if (k == 'W') {
                grid_.ShowMessage(true);  // won
                continue;
            } else if (k == 'E') {
                grid_.ShowMessage(false);  // lost
                continue;
            } else {
                grid_.ClearMessage();
            }

            Sleep(0);
        }

        ResetConsole(s);
        return 0;
    }

  private:
    class Scorer
    {
      public:
        Scorer() : value_(0) { }
        ~Scorer() { }

        void operator()(int a)
        {
            value_ += a;
        }

        // int operator=(int n)
        // {
        //     value_ = n;
        //     return value_;
        // }

        operator int() const
        {
            return value_;
        }

        void Reset(int n)
        {
            value_ = n;
        }

      private:
        DISALLOW_COPY_AND_ASSIGN(Scorer);

      private:
        int value_;
    };

    class InputReader
    {
      public:
        InputReader(short top = 0)
            : k_(0), xk_(0), t_(0), xt_(0), mapper_(top)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
        // not supported ?
#else
              , inrec_{0, {{0, 0, 0, 0, {0}, 0}}}
#endif
        {
#if defined(_MSC_VER) && (_MSC_VER < 1900)
            memset(&inrec_, 0, sizeof(inrec_));
#endif
        }
        ~InputReader() { }

        int GetInput()
        {
            class Dummy
            {
              public:
                void operator()()
                {
                    // no-op
                }
            };
            Dummy dummy;
            return GetInput(dummy);
        }

        template<typename T>
        int GetInput(T& time_keeper)
        {
            k_ = GAME_NOOP;
            int n = 0;
            int value = 0;
            bool done = false;

            do {
                if (con.ReadInput(inrec_)) {
                    switch (inrec_.EventType) {
                    case 0:  // NOTE: 0 is set by Console.ReadInput(). can be error
                        time_keeper();
                        continue;
                    case KEY_EVENT:
                        value = GetKeyInput(inrec_.Event.KeyEvent, n);

                        if (value) {
                            return value;
                        } else { }
                        continue;
                    case MOUSE_EVENT:
                        value = GetMouseInput(inrec_.Event.MouseEvent);

                        if (value) {
                            return value;
                        } else { }
                        continue;
                    case WINDOW_BUFFER_SIZE_EVENT:
                    case MENU_EVENT:
                    case FOCUS_EVENT:
                        continue;
                    default:
                        DPRINT("unknown input.EventType: %u", inrec_.EventType);
                        continue;
                    }
                } else {
                    return GAME_ABORT;
                }
                continue;
            } while (!done);

            return GAME_NOOP;
        }

      private:
        int GetKeyInput(KEY_EVENT_RECORD& ker, int& n)
        {
            k_ = ker.wVirtualKeyCode;

            if (ker.bKeyDown == FALSE) {
                switch (k_) {
                case VK_F5: return BOARD_REFRESH;
                case 'Y': case VK_RETURN: return GAME_PERSIST;
                case 'Q': case VK_ESCAPE: return GAME_ABORT;
                case 'N': return GAME_STOP;
                case 'Z': return GAME_UNDO;
                case 'I': return GAME_RESTART;
                case 'T': return BOARD_TRANSPOSE;
                case 'R': return BOARD_ROTATE_CW;
                case 'V': return BOARD_SWAP_VERTICAL;
                case 'H': return BOARD_SWAP_HORIZONTAL;
#ifdef TEST_
                case 'W': return (++n == 3) ? GAME_WON : GAME_NOOP;
                case 'E': return (++n == 3) ? GAME_LOST : GAME_NOOP;
#endif
                default: break;
                }
            } else { }

            n = (xk_ != k_) ? 0 : n;

            t_ = Clock().Ticks_ms();

            if (k_ == xk_) {
                if ((t_ - xt_) < 300u) {
                    return GAME_NOOP;
                } else { }
            } else { }

            xt_ = t_;
            xk_ = k_;

            switch (k_) {
            case VK_LEFT: return MOVE_LEFT;
            case VK_RIGHT: return MOVE_RIGHT;
            case VK_UP: return MOVE_UP;
            case VK_DOWN: return MOVE_DOWN;
            case 'R':
                if (ker.dwControlKeyState & SHIFT_PRESSED) {
                    return BOARD_ROTATE_CCW;
                } else { }
                break;
            default: break;
            }

            return GAME_NOOP;
        }

        int GetMouseInput(MOUSE_EVENT_RECORD& mer)
        {
            if (mer.dwEventFlags & MOUSE_WHEELED) {
                if (mer.dwControlKeyState & SHIFT_PRESSED) {
                    if ((int)mer.dwButtonState > 0) {
                        return MOVE_MOUSE_WHEEL_FW_SHIFT;
                    } else {
                        return MOVE_MOUSE_WHEEL_BW_SHIFT;
                    }
                } else {
                    if ((int)mer.dwButtonState > 0) {
                        return MOVE_MOUSE_WHEEL_FW;
                    } else {
                        return MOVE_MOUSE_WHEEL_BW;
                    }
                }
            } else {
                if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                    COORD& pos = mer.dwMousePosition;
                    int ret = mapper_.FindCell(pos.X, pos.Y);

                    if (ret < 0x10000) {
                        return ret;
                    } else {
                        DWORD& key = mer.dwControlKeyState;

                        if (key & LEFT_ALT_PRESSED) {
                            return ret;  // cheating :)
                        } else { }
                    }
#if 0
                } else if (mer.dwControlKeyState & SHIFT_PRESSED) {
#endif
                } else { }
            }

            return GAME_NOOP;
        }

      private:
        DISALLOW_COPY_AND_ASSIGN(InputReader);

      private:
        int k_;
        int xk_;
        int64_t t_;
        int64_t xt_;
        Mapper mapper_;
        INPUT_RECORD inrec_;
    };

    short InitConsole(int a, int d)
    {
        con.Acquire();
        grid_.SetGridMode(d);
        int board_height = grid_.GetMinHeight();
        con.Resize(board_height + 1);

        short y_top;

        if (a & 0x4) {
            con.SaveBuffer();
            y_top = con.ResetCursorPosition(0);
        } else {
            con.Clear();
            y_top = con.ResetCursorPosition(-1);
        }

        con.SavePalette();

        if (a != '?') {
            grid_.SetColorScheme(a & 0x3);
        } else { }

        con.FlushInput();
        con.MoveTo(0, 0);

        return y_top;
    }

    void ResetConsole(int a)
    {
        if (con.CanReset()) {
            if (a != '?') {
                con.ResetPalette();

                if (a & 0x4) {
                    con.RestoreBuffer();
                } else {
                    con.MoveTo(0, (4 * N + 3));
                }
            } else { }

            con.Release();
        } else { }
    }

    void Start2048(int n = 0)
    {
        grid_.ClearMessage();
        con.MoveTo(0, 0);
        rng.Seed(rng(256) + (((unsigned int)Clock().Ticks_ms()) & 0xffff));

        if (n) {
            Preset(n);
            board_.ac[0][0] = (uint8_t)(n + 1);
            score_.Reset(0);
        } else {
            matrix.Reset(0);
            unsigned int r, c;
            AddNew(r, c);
            AddNew(r, c);
            score_.Reset(0);
        }

        grid_.DrawGrid();
        grid_.ShowMatrix(matrix);
        grid_.ShowScore(score_);
        time_keeper_.Start();
        time_keeper_.Update();
        Save();
    }

    unsigned int Nudge(int type)
    {
        unsigned int m = 0;

        for (int i = 0; i < N; ++i) {
            Stripe s(type, i, board_.ac);
            m += s.Nudge(score_);
        }

        return m;
    }

    void Save()
    {
        old_score_ = score_;

        for (int i = 0; i < N64; ++i) {
            undo_.al[i] = board_.al[i];
        }
    }

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#define HIGHLIGHT_FILTER 0x7f7f7f7f7f7f7f7fu
#else
#define HIGHLIGHT_FILTER 0x7f7f7f7f7f7f7f7fllu
#endif
    void Undo()
    {
        score_.Reset(old_score_);

        for (int i = 0; i < N64; ++i) {
            board_.al[i] = undo_.al[i] & HIGHLIGHT_FILTER;
        }
    }

    void ResetHighlight()
    {
        for (int i = 0; i < N64; ++i) {
            board_.al[i] &= HIGHLIGHT_FILTER;
        }
    }
#undef HIGHLIGHT_FILTER

    unsigned int CountZeros(int& min, int& max)
    {
        max = 0;
        min = 16;
        unsigned int n = 0;

        ResetHighlight();

        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (board_.ac[r][c] == 0) {
                    ++n;
                } else if (board_.ac[r][c] < min) {
                    min = board_.ac[r][c];
                } else if (board_.ac[r][c] > max) {
                    max = board_.ac[r][c];
                } else { }
            }
        }

        return n;
    }

    unsigned int AddNew(unsigned int& row, unsigned int& col)
    {
        int min, max;
        unsigned int nz = CountZeros(min, max);

        if (nz > N + N) {
            nz -= N;
        } else { }

        if (nz > 0) {
            unsigned int pos = rng(nz);
            uint8_t v = GetNewValue(min, max);
            unsigned int n = 0;

            for (unsigned int r = N; r--; (void)0) {
                for (unsigned int c = N; c--; (void)0) {
                    if (board_.ac[r][c] == 0) {
                        if (n == pos) {
                            board_.ac[r][c] = v;  // TODO
                            row = r;
                            col = c;
                        } else { }

                        ++n;
                    } else { }
                }
            }

            grid_.ShowCell(matrix(row, col), row, col, true);
        } else { }

        return nz;
    }

    uint8_t GetNewValue(int min, int max)
    {
        // max  1 2 3  4      5  6   7    8   9   10    11
        //             |             |        |    |     |
        //      2 4 8 16     32 64 128  256 512 1024  2048

        int r2, r4, r8;  // ranges for 2, 4 and 8 respectively

#define SET($r2, $r4, $r8, $x) \
        r2 = $r2; \
        r4 = $r4; \
        r8 = $r8; \
        static_assert(($r2+$r4+$r8+$x) == 256, \
                      "GetNewValue: total of ranges should be 256") \

#define SET2($r2, $r4, $r8, $r22, $r24, $r28) \
        if (min != 1) { \
            r2 = $r2; \
            r4 = $r4; \
            r8 = $r8; \
        } else { \
            r2 = $r22; \
            r4 = $r24; \
            r8 = $r28; \
        } \
        static_assert(($r2+$r4+$r8) == 256, \
                      "GetNewValue: total of ranges should be 256"); \
        static_assert(($r22+$r24+$r28) == 256, \
                      "GetNewValue: total of ranges should be 256") \

        switch (max) {
        case 0: case 1: case 2: case 3: case 4:
            SET(160, 96, 0, 0);
            break;
        case 5: case 6: case 7:
            SET(64, 160, 32, 0);
            break;
        case 8: case 9:
            SET2(0, 208, 48, 120, 120, 16);
            break;
        case 10:
            SET(0, 160, 96, 0);
            break;
        default:
            SET(0, 96, 152, 8);
            break;
        }
#undef SET
#undef SET2

        int rnd = (unsigned short)rng(256);
        int r = 2;

        if (r2 > rnd) {
            r = 1;  // 2
        } else if ((r2 + r4) > rnd) {
            r = 2;  // 4
        } else if ((r2 + r4 + r8) > rnd) {
            r = 3;  // 8
        } else {
            r = 4;  // 16
        }

        return (uint8_t)r;
    }

    int IsSolvable()
    {
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N - 1; ++c) {
                if (board_.ac[r][c] == board_.ac[r][c + 1]) {
                    return 1;
                } else { }
            }
        }

        for (int c = 0; c < N; ++c) {
            for (int r = 0; r < N - 1; ++r) {
                if (board_.ac[r][c] == board_.ac[r + 1][c]) {
                    return 1;
                } else { }
            }
        }

        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (board_.ac[r][c] == 0) {
                    return 1;
                } else { }
            }
        }

        return 0;
    }

    void Preset(int i)
    {
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c, ++i) {
                if (r & 1) {
                    board_.ac[r][N - 1 - c] = (uint8_t)(i > 0 ? i : 0);
                } else {
                    board_.ac[r][c] = (uint8_t)(i > 0 ? i : 0);
                }
            }
        }
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Puzzle2048);

  private:
    int old_score_;
    Scorer score_;
    Grid grid_;
    TimeKeeper time_keeper_;
    Board4x4 undo_;
    Board4x4 board_;
    Matrix matrix;
};

// application option/help/version helpers {{{
class AppInfo
{
  public:
    AppInfo() { }
    ~AppInfo() { }

    void ShowVersion()
    {
        const char* ret;

        ShowTitle(true);
        char str[128];
        fprintf(stdout, " Compilation:\n  compiler: %s\n", GetCompilerVersion(str));

        if ((ret = GetCompilerArgs()) != 0) {
            fprintf(stdout, "  command: %s\n", ret);
        } else { }
    }

    void ShowTitle(bool show_date = false)
    {
        fprintf(stdout, "%s version %s", GetDescription(), GetVersion());

        if (show_date) {
            char str[128];
            const char* ret;

            if ((ret = GetBuildDate(str)) != 0) {
                fprintf(stdout, " (%s)\n", ret);
            } else { }
        } else {
            fprintf(stdout, " \n");
        }
    }

    const char* GetDescription()
    {
#if defined(APP_DESCRIPTION)
        return APP_DESCRIPTION;
#else
        return 0;
#endif
    }

    const char* GetVersion()
    {
#if defined(APP_VERSION)
        return APP_VERSION;
#else
        return 0;
#endif
    }

    template <size_t N>
    const char* GetBuildDate(char (&str)[N])
    {
        memset(str, 0, N);

        // Mmm dd yyyy - format of __DATE__
        // 012 45 7890

        str[0] = __DATE__[7];
        str[1] = __DATE__[8];
        str[2] = __DATE__[9];
        str[3] = __DATE__[10];
        str[4] = '-';
        str[5] = '0';
        str[6] = '?';
        str[7] = '-';

        switch (__DATE__[0]) {
        case 'O': case 'N': case 'D':
            str[5] = '1'; break;
        default: break;
        }

        switch (__DATE__[2]) {
        case 'b': str[6] = '2'; break;  // Feb
        case 'c': str[6] = '2'; break;  // Dec
        case 'g': str[6] = '8'; break;  // Aug
        case 'l': str[6] = '7'; break;  // Jul
        case 'n': str[6] = __DATE__[1] == 'a' ? '1' : '6'; break;  // Jan | Jun
        case 'p': str[6] = '9'; break;  // Sep
        case 'r': str[6] = __DATE__[1] == 'a' ? '3' : '4'; break;  // Mar | Apr
        case 't': str[6] = '0'; break;  // Oct
        case 'v': str[6] = '1'; break;  // Nov
        case 'y': str[6] = '5'; break;  // May
        default:  str[6] = '?'; break;  // NOTE: error
        }

        str[7] = '-';
        str[8] = (__DATE__[4] == ' ' ? '0' : __DATE__[4]);
        str[9] = __DATE__[5];
        str[10] = ' ';
        str[11] = __TIME__[0];
        str[12] = __TIME__[1];
        str[13] = ':';
        str[14] = __TIME__[3];
        str[15] = __TIME__[4];
        str[16] = ':';
        str[17] = __TIME__[6];
        str[18] = __TIME__[7];
        str[19] = '\0';
#if defined(GCC_ONLY_)
        static_assert((__DATE__[0] != '?'), "error in __DATE__ macro");
#endif
        static_assert((N > 19), "specify a larger string, at least 19 chars needed");

        return str;
    }

    template <size_t N>
    const char* GetCompilerVersion(char (&str)[N])
    {
        // TODO: __GNUC__ et al. seems incorrect with LLVM/Clang
        memset(str, 0, N);

        errno_t error;

#define BREAK_IF_ERROR($cmd, $err) \
        do { \
            $err = $cmd; \
            if ($err != 0) { \
                break; \
            } else { \
            } \
        } while (FALSE_CONDITION)

        do {
#if defined(__clang__)
            BREAK_IF_ERROR(strcat_s(str, "Clang/LLVM "), error);
            BREAK_IF_ERROR(strcat_s(str, STR2(__clang_major__)), error);
            BREAK_IF_ERROR(strcat_s(str, STR2(__clang_minor__)), error);
            BREAK_IF_ERROR(strcat_s(str, STR1(__clang_patchlevel__)), error);

#if defined(__GNUC__) || defined(_MSC_FULL_VER)
            BREAK_IF_ERROR(strcat_s(str, " ["), error);
#endif
#endif  // __clang__
#if defined(__GNUC__)
            BREAK_IF_ERROR(strcat_s(str, "GCC/C++ "), error);
#if defined(CC_STUB_GC_VER)
            BREAK_IF_ERROR(strcat_s(str, STR1(CC_STUB_GC_VER)), error);
#else
            BREAK_IF_ERROR(strcat_s(str, STR2(__GNUC__)), error);
            BREAK_IF_ERROR(strcat_s(str, STR2(__GNUC_MINOR__)), error);
            BREAK_IF_ERROR(strcat_s(str, STR1(__GNUC_PATCHLEVEL__)), error);
#endif  // CC_STUB_GC_VER
#if defined(__MINGW32__)
#if defined(__MINGW64_VERSION_MAJOR)
            BREAK_IF_ERROR(strcat_s(str, " [MinGW-w64 "), error);
            BREAK_IF_ERROR(strcat_s(str, STR2(__MINGW64_VERSION_MAJOR)), error);
            BREAK_IF_ERROR(strcat_s(str, STR1(__MINGW64_VERSION_MINOR)), error);
#else
            // NOTE: __MINGW32__ may not be needed
            BREAK_IF_ERROR(strcat_s(str, " [MinGW "), error);
            BREAK_IF_ERROR(strcat_s(str, STR2(__MINGW32_MAJOR_VERSION)), error);
            BREAK_IF_ERROR(strcat_s(str, STR1(__MINGW32_MINOR_VERSION)), error);
#endif  // __MINGW64__
            BREAK_IF_ERROR(strcat_s(str, "]"), error);
#endif  // __MINGW32__
#endif  // __GNUC__
#if defined(_MSC_FULL_VER)
            BREAK_IF_ERROR(strcat_s(str, "MSVC++ "), error);
#if defined(CC_STUB_VC_VER)
            BREAK_IF_ERROR(strcat_s(str, STR1(CC_STUB_VC_VER)), error);
#else
            // VVRRPPPPP since VS 2005
            BREAK_IF_ERROR(strncat_s(str, STR1(_MSC_FULL_VER) + 0, 2), error);
            BREAK_IF_ERROR(strcat_s(str, "."), error);
            BREAK_IF_ERROR(strncat_s(str, STR1(_MSC_FULL_VER) + 2, 2), error);
            BREAK_IF_ERROR(strcat_s(str, "."), error);
            BREAK_IF_ERROR(strcat_s(str, STR1(_MSC_FULL_VER) + 4), error);
#endif  // CC_STUB_VC_VER
#endif  // _MSC_FULL_VER

#if defined(__clang__)
#if defined(__GNUC__) || defined(_MSC_FULL_VER)
            BREAK_IF_ERROR(strcat_s(str, "]"), error);
#endif
#endif  // __clang__
        } while (FALSE_CONDITION);

#undef BREAK_IF_ERROR

        if (error) {
            return 0;
        } else { }

        return str;
    }

    const char* GetCompilerArgs()
    {
#if defined(CC_STUB_CONFIG)
        return STR1(CC_STUB_CONFIG);
#else
        return 0;
#endif
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(AppInfo);
};

struct arg_definition {
    int need_value;
    char short_option;
    const char* long_option;
    const char* default_value;
    const char* help;
    const char* value;
    int count;
};

template <size_t N>
class ArgParser
{
  private:
    enum { LENGTH = N };
  public:
    explicit ArgParser(arg_definition(&arg_def)[N])
        : help_(0), version_(0), arg_def_(arg_def) { }
    ~ArgParser() { }

    int parse(int argc, char* argv[])
    {
        int error = 0;
        int token = 0;
        bool end_of_opt = false;

        for (int i = 1; i < argc; i++) {
            int ret = 0;

            if (argv[i][0] == '-') {
                if (argv[i][1] == '-') {
                    if (argv[i][2] == '\0') {
                        end_of_opt = !end_of_opt;
                        continue;
                    } else {
                        if (end_of_opt) {
                            ret = get_argument(argv[i]);
                        } else {
                            ret = get_long_arg(argv[i]);
                        }
                    }
                } else if (argv[i][1] != '\0') {
                    if (end_of_opt) {
                        ret = get_argument(argv[i]);
                    } else {
                        ret = get_short_arg(argv, i, argc);
                    }
                } else {
                    ret = -1;
                }
            } else {
                ret = get_argument(argv[i]);
            }

            if (ret < 0) {
                if (check_known_opt(argv[i])) {
                    return 1;
                } else { }

                ++error;
            } else {
                token += ret + 1;
                i += ret;
            }
        }

        if (token == 0 && argc > 1) {
            error++;
        } else { }

        // EPRINT("tok %d, argc %d, err %d, ret %d\n", token, argc, error,
        //        (error == 0 ? 1 : 0));

        return error == 0 ? 1 : 0;
    }

    void show_help(const char* name, const char* header, const char* footer)
    {
        if (header && *header) {
            fprintf(stdout, "%s", header);
        } else { }

        fprintf(stdout, "Usage: %s [options]\n", name);
        fprintf(stdout, " Options:\n");

        for (int i = 0; i < LENGTH; ++i) {
            show_option_help(i);
        }

        if (footer && *footer) {
            fprintf(stdout, "%s", footer);
        } else { }
    }

    int show_option_help(int id)
    {
        if ((id < 0) || (id >= LENGTH)) {  // TODO: assert
            return 0;
        } else { }

#define PRINT_OPT($opt, $fmt, $sep, $hasopt) \
        if (arg_def_[id].need_value) { \
            if (arg_def_[id].$opt) { \
                fprintf(stdout, $fmt $sep "VALUE", arg_def_[id].$opt); \
                $hasopt = 1; \
            } else { } \
        } else { \
            if (arg_def_[id].$opt) { \
                fprintf(stdout, $fmt, arg_def_[id].$opt); \
                $hasopt = 1; \
            } else { } \
        } (void)0

        int has_opt = 0;

        PRINT_OPT(short_option, "  -%c", " ", has_opt);
        PRINT_OPT(long_option, "  --%s", "=", has_opt);

        if (has_opt) {
            fprintf(stdout, "  %s %s%s%c\n",
                    (arg_def_[id].help ? arg_def_[id].help : ""),
                    (arg_def_[id].default_value ? "(default: " : ""),
                    (arg_def_[id].default_value ? arg_def_[id].default_value : ""),
                    (arg_def_[id].default_value ? ')' : ' '));
        } else { }

#undef PRINT_OPT
        return 1;
    }

    int has_help_opt()
    {
        return help_;
    }

    int has_version_opt()
    {
        return version_;
    }

  private:
    int get_short_arg(char* argv[], int pos, int argc)
    {
        if (!(argv && (pos > 0) && (pos < argc))) {  // TODO: assert
            return -1;
        } else { }

        int error = 0;
        int consumed = 0;
        const char* arg = argv[pos];

        for (char opt = *++arg; opt; opt = *++arg) {
            // EPRINT("get_short_arg: %c\n", opt);
            int found = 0;

            for (int i = 0; i < LENGTH; i++) {
                if (arg_def_[i].short_option == opt) {
                    arg_def_[i].count += 1;

                    if (arg_def_[i].need_value) {
                        if ((++pos) < argc) {
                            arg_def_[i].value = argv[pos];
                            ++consumed;
                        } else {
                            // EPRINT("get_short_arg ret -1: %c\n", opt);
                            return -1;
                        }
                    } else { }

                    ++found;
                } else { }
            }

            // EPRINT("get_short_arg: %c %s found , con %d err %d ret %d\n",
            //        opt, (found?"":"not"), consumed, error,
            //        (error == 0 ? consumed : -1));
            if (found == 0) {
                ++error;
            } else { }
        }

        return error == 0 ? consumed : -1;
    }

    int get_long_arg(const char* arg)
    {
        if (!arg) {  // TODO: assert
            return -1;
        } else { }

        bool negate = false;

        if ((arg[2] == 'n') && (arg[3] == 'o')) {
            negate = true;

            if (arg[4] == '-') {
                arg += 1;
            } else { }

            arg += 2;
        } else { }

        if (*arg == '\0') {
            return -1;
        } else { }

        for (int i = 0; i < LENGTH; i++) {
            size_t pos = opt_equal(arg_def_[i].long_option, arg);

            if (pos) {
                arg += pos;

                if (arg_def_[i].need_value) {
                    if (*arg) {
                        arg_def_[i].value = arg;
                    } else {
                        return -1;
                    }
                } else {
                    if (*arg) {
                        arg_def_[i].value = arg;  // TODO: ignoring error, why?
                    } else { }
                }

                if (negate) {
                    if (arg_def_[i].count > 0) {
                        arg_def_[i].count = -arg_def_[i].count;
                    } else { }

                    arg_def_[i].count -= 1;
                } else {
                    if (arg_def_[i].count < 0) {
                        arg_def_[i].count = -arg_def_[i].count;
                    } else { }

                    arg_def_[i].count += 1;
                }

                return 0;
            } else { }
        }

        return -1;
    }

    int get_argument(const char* arg)
    {
        if (!arg) {  // TODO: assert
            return -1;
        } else { }

        int found = 0;

        for (int i = 0; i < LENGTH; i++) {
            if ((!arg_def_[i].value) &&
                (!arg_def_[i].short_option) &&
                (!arg_def_[i].long_option)) {
                ++found;
                arg_def_[i].value = arg;
                break;
            } else { }
        }

        return found ? 0 : -1;
    }

    int check_known_opt(const char* arg)
    {
        if (!arg) {  // TODO: assert
            return -1;
        } else { }

        char c0 = arg[0];

        if (c0 == '?') {
            help_ = 1;
            return 1;
        } else if (c0 == '-') {
            char c1 = arg[1];

            if (c1 == '?') {
                help_ = 1;
                return 1;
            } else if (c1 == '-') {
                char c2 = arg[2];

                if (c2 == '?') {
                    help_ = 1;
                    return 1;
                } else if ((c2 == 'h') &&
                           (arg[3] == 'e') &&
                           (arg[4] == 'l') &&
                           (arg[5] == 'p') &&
                           (arg[6] == '\0')) {
                    help_ = 1;
                    return 1;
                } else if ((c2 == 'v') &&
                           (arg[3] == 'e') &&
                           (arg[4] == 'r') &&
                           (arg[5] == 's') &&
                           (arg[6] == 'i') &&
                           (arg[7] == 'o') &&
                           (arg[8] == 'n') &&
                           (arg[9] == '\0')) {
                    version_ = 1;
                    return 1;
                } else { }
            } else { }
        } else { }

        return 0;
    }

    size_t opt_equal(const char* option, const char* arg)
    {
        if (!(option && arg && arg[2])) {  // TODO: assert
            return 0;
        } else { }

        size_t len = strlen(option);  // NOTE: length can be cached

        if (len > 0) {
            if (strncmp(option, (arg + 2), len) == 0) {
                if (arg[len + 2] == '=') {
                    return len + 3;
                } else if (arg[len + 2] == '\0') {
                    return len + 2;
                } else { }
            } else { }
        } else { }

        return 0;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ArgParser);

  private:
    int help_;
    int version_;
    struct arg_definition(&arg_def_)[N];
};

/* get-options */
#define UNWRAP(x) x
// macro args:    id, need, short, long, default, help, value, count
// arg types:     --, int,  char,  char*, char*,  char*, char*, int
#define OPT_CLRS (color_id, 1, 'c', "color", "0", "color scheme: 0, 1, 2 or 3")
#define OPT_GRID (grid_type, 1, 'g', "grid", "unicode", "draw with ascii|unicode characters")
#define OPT_WIPE (wipe_con, 0, 'w', "wipe", NULL, "wipes the display when exiting(default: do not wipe)")
#define OPT_TEST (test_mode, 0, '\0', "test", NULL, "with '--color' shows color scheme and exit")
#define OPT_TILE (tile_set, 1, '\0', "tile-set", "1", "previews grid/tiles, choices 1, 2 or 3")
#define OPT_HELP (more_arg, 0, '\0', NULL, NULL, NULL)

#define OPTS \
    OPT_CLRS,OPT_GRID,OPT_WIPE,OPT_TEST,OPT_TILE,OPT_HELP

// macros GET_FUNC and CODE_GEN based on FOR_EACH macros from link below:
// http://stackoverflow.com/questions/1872220/

#define F1(F,A) F(A)
#define F2(F,A,...) F(A)UNWRAP(F1(F,__VA_ARGS__))
#define F3(F,A,...) F(A)UNWRAP(F2(F,__VA_ARGS__))
#define F4(F,A,...) F(A)UNWRAP(F3(F,__VA_ARGS__))
#define F5(F,A,...) F(A)UNWRAP(F4(F,__VA_ARGS__))
#define F6(F,A,...) F(A)UNWRAP(F5(F,__VA_ARGS__))

#define GET_FUNC(A1,A2,A3,A4,A5,A6,FUNC,...) FUNC
#define CODE_GEN(GEN_FUNC,...) \
    UNWRAP(GET_FUNC(__VA_ARGS__,F6,F5,F4,F3,F2,F1)(GEN_FUNC,__VA_ARGS__))

#define GET_ID(a,...) int a;
#define CALL_GET_ID(x) GET_ID x
struct option {
    CODE_GEN(CALL_GET_ID, OPTS)
};
#undef GET_ID
#undef CALL_GET_ID

#define GET_ENUM_ID(a,...) k_##a,
#define CALL_GET_ENUM_ID(x) GET_ENUM_ID x
enum {
    CODE_GEN(CALL_GET_ENUM_ID, OPTS)
};
#undef GET_ENUM_ID
#undef CALL_GET_ENUM_ID

template <size_t N>
class ArgResolver
{
  private:
    enum { LENGTH = N };
  public:
    ArgResolver(option& opt, arg_definition(&arg_def)[N])
        : help_(0), error_(0), version_(0), opt_(opt), arg_def_(arg_def) { }
    ~ArgResolver() { }

    int Resolve()
    {
#define RESOLVE_OPT(a,...) \
        Resolve_##a();

#define CALL_RESOLVE_OPT(x) RESOLVE_OPT x

        CODE_GEN(CALL_RESOLVE_OPT, OPTS)

#undef RESOLVE_OPT
#undef CALL_RESOLVE_OPT

        if (error_ > 0) {
            return -error_;
        } else if (help_ || version_) {
            return 0;
        } else { }

        return 1;
    }

    int Resolve_color_id()
    {
        // EPRINT("%s\n", "color");
        int id = k_color_id;

        if (arg_def_[id].count && arg_def_[id].value) {
            if (arg_def_[id].value[1] == '\0') {
                switch (arg_def_[id].value[0]) {
                case '0': opt_.color_id = 0; break;
                case '1': opt_.color_id = 1; break;
                case '2': opt_.color_id = 2; break;
                case '3': opt_.color_id = 3; break;
                default: ++error_; break;
                }
            } else {
                ++error_;
            }
        } else { }

        return error_ ? 0 : 1;
    }

    int Resolve_grid_type()
    {
        // EPRINT("%s\n", "grid");
        int id = k_grid_type;

        if (arg_def_[id].count && arg_def_[id].value) {
            if (strcmp(arg_def_[id].value, "ascii") == 0) {
                opt_.grid_type = 0;
            } else if (strcmp(arg_def_[id].value, "unicode") == 0) {
                opt_.grid_type = 1;
            } else {
                ++error_;
            }
        } else { }

        return error_ ? 0 : 1;
    }

    int Resolve_wipe_con()
    {
        // EPRINT("%s\n", "wipe");
        int id = k_wipe_con;

        if (arg_def_[id].count > 0) {
            opt_.wipe_con = 1;
        } else {
            opt_.wipe_con = 0;
        }

        return 1;
    }

    int Resolve_test_mode()
    {
        // EPRINT("%s\n", "test");
        int id = k_test_mode;

        if (arg_def_[id].count > 0) {
            opt_.test_mode = 1;
        } else {
            opt_.test_mode = 0;
        }

        return 1;
    }

    int Resolve_tile_set()
    {
        int id = k_tile_set;

        // EPRINT("%s %s\n", "tile", arg_def_[id].value);
        if (arg_def_[id].count && arg_def_[id].value) {
            if (arg_def_[id].value[1] == '\0') {
                switch (arg_def_[id].value[0]) {
                case '1': opt_.tile_set = 1; break;
                case '2': opt_.tile_set = 2; break;
                case '3': opt_.tile_set = 3; break;
                default: ++error_; break;
                }
            } else {
                ++error_;
            }
        } else { }

        return error_ ? 0 : 1;
    }

    int Resolve_more_arg()
    {
        // EPRINT("%s\n", "unknown");
        int id = k_more_arg;

        if (arg_def_[id].value) {
            if ((arg_def_[id].value[0] == '?') &&
                (arg_def_[id].value[1] == '\0')) {
                help_ = 1;
                return 1;  // help option cannot be combined with others
            } else if ((arg_def_[id].value[0] == 'h') &&
                       (arg_def_[id].value[1] == 'e') &&
                       (arg_def_[id].value[2] == 'l') &&
                       (arg_def_[id].value[3] == 'p') &&
                       (arg_def_[id].value[4] == '\0')) {
                help_ = 1;
                return 1;  // help option cannot be combined with others
            } else if ((arg_def_[id].value[0] == 'v') &&
                       (arg_def_[id].value[1] == 'e') &&
                       (arg_def_[id].value[2] == 'r') &&
                       (arg_def_[id].value[3] == 's') &&
                       (arg_def_[id].value[4] == 'i') &&
                       (arg_def_[id].value[5] == 'o') &&
                       (arg_def_[id].value[6] == 'n') &&
                       (arg_def_[id].value[7] == '\0')) {
                version_ = 1;
                return 1;  // version option cannot be combined with others
            } else { }

            ++error_;
        } else { }

        return error_ ? 0 : 1;
    }

    int error()
    {
        return error_;
    }

    int has_help_opt()
    {
        return help_;
    }

    int has_version_opt()
    {
        return version_;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ArgResolver);

  private:
    int help_;
    int error_;
    int version_;
    struct option& opt_;
    struct arg_definition(&arg_def_)[N];
};

void dump_arg_def(struct arg_definition* arg_def)
{
#define PRINT_DEF(a,...) \
    fprintf(stderr, "%-10s need(%d), count(%d), value(%s)\n", \
            (#a), arg_def[k_##a].need_value, arg_def[k_##a].count, \
            arg_def[k_##a].value ? arg_def[k_##a].value : "(null)"); \

#define CALL_PRINT_DEF(x) PRINT_DEF x

    CODE_GEN(CALL_PRINT_DEF, OPTS)

#undef PRINT_DEF
#undef CALL_PRINT_DEF
}

int get_option(int argc, char* argv[], struct option& opt)
{
#define GET_DEF(a,b,c,d,e,f) { b, c, d, e, f, 0, 0 },
#define CALL_GET_DEF(x) GET_DEF x
    int error = 0;
    struct arg_definition arg_def[] = {
        CODE_GEN(CALL_GET_DEF, OPTS)
    };

#undef GET_DEF
#undef CALL_GET_DEF

    ArgParser<sizeof(arg_def)/sizeof(arg_definition)> arg_parser(arg_def);

#define SHOW_HELP_VERSION($obj) \
    do { \
        if ($obj.has_help_opt()) { \
            AppInfo appinfo; \
            appinfo.ShowTitle(); \
            arg_parser.show_help(argv[0], 0, 0); \
            return 0; \
        } else if ($obj.has_version_opt()) { \
            AppInfo appinfo; \
            appinfo.ShowVersion(); \
            return 0; \
        } else { } \
    } while (FALSE_CONDITION)

    if (arg_parser.parse(argc, argv)) {
        // dump_arg_def(arg_def);

        SHOW_HELP_VERSION(arg_parser);

        ArgResolver<sizeof(arg_def)/sizeof(arg_definition)> arg_resolver(opt, arg_def);
        int ret = arg_resolver.Resolve();

        if (ret >= 0) {  // help or version
            SHOW_HELP_VERSION(arg_resolver);
        } else {
            ++error;
        }
#undef SHOW_HELP_VERSION
    } else {
        ++error;
    }

    if (error) {
        fprintf(stderr, "Try `%s --help' for more information\n", argv[0]);
    } else { }

    return error ? 0 : 1;
}

void dump_opt(struct option& opt)
{
#define PRINT_OPT(a,...) \
    fprintf(stderr, "%-10s : %d\n", \
            (#a), opt.a);

#define CALL_PRINT_OPT(x) PRINT_OPT x

    CODE_GEN(CALL_PRINT_OPT, OPTS)

#undef PRINT_OPT
#undef CALL_PRINT_OPT
}

#undef CODE_GEN
#undef F
#undef F1
#undef F2
#undef F3
#undef F4
#undef F5
#undef F6
#undef FUNC
#undef GEN_FUNC
#undef GET_FUNC
#undef OPTS
#undef OPT_CLRS
#undef OPT_GRID
#undef OPT_HELP
#undef OPT_TEST
#undef OPT_TILE
#undef OPT_WIPE
#undef UNWRAP

void dump_arg(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++) {
        fprintf(stderr, "%d: %s\n", i, argv[i]);
    }
}
// end of application option/help/version helpers }}}

int play1(int argc, char* argv[])
{
    int ret;
    rng.Seed((unsigned int)Clock().Ticks_ms() & 0xffff);
    Puzzle2048 p2048;

    option opt = { 0, 1, 0, 0, 0, 0 };

    if (argc > 1) {
        ret = get_option(argc, argv, opt);
        // dump_arg(argc, argv);
        // dump_opt(opt);

        if (ret) {
            if (opt.wipe_con) {
                opt.color_id |= 0x4;
            } else { }

            if (opt.test_mode) {
                con.SetTitle(_TEXT("Puzzle 2048: color scheme test"));
                ret = p2048.SchemeTest(opt.color_id, opt.grid_type);
            } else if (opt.tile_set) {
                con.SetTitle(_TEXT("Puzzle 2048: draw mode test"));
                ret = p2048.GridTest(opt.tile_set, opt.color_id, opt.grid_type);
            } else {
                con.SetTitle(_TEXT("Puzzle 2048"));
                ret = p2048.Play(opt.color_id, opt.grid_type);
            }
        } else { }
    } else {
        con.SetTitle(_TEXT("Puzzle 2048"));
        ret = p2048.Play(0, 1);
    }

    if (opt.wipe_con) {
    } else if (ret) {
        fprintf(stderr, "%s\n", "Good day, bye");
    } else { }

    return ret;
}

int main(int argc, char* argv[])
{
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(UnhandledExceptionFilterFunc);
    _invalid_parameter_handler oldHandler, newHandler;
    newHandler = invalid_parameter_handler;
    oldHandler = _set_invalid_parameter_handler(newHandler);
    (void)oldHandler;

    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
        // printf("\nThe Control Handler is installed.\n");
    } else {
        fprintf(stderr, "\nERROR: Could not set control handler");
    }

    int ret = 0;

    __try {
        __try {
        } __finally {
            ret = play1(argc, argv);
        }
    } __except (UnhandledExceptionFilterFunc(GetExceptionInformation())) {
#if defined(__GNUC__) || defined(__clang__)
#else  // _MSC_VER
        OutputDebugStringA("executed exception filter function\n");
#endif  // __GNUC__ || __clang__
    }

    con.AllowCtrlHandler();

    return ret;
}

#if defined(MSC_ONLY_)
#if defined(NO_WALL_FILTER)
// INFO("warnings were not filtered")
#else
// INFO("warnings were filtered")
#pragma warning(disable: 4514)  // unreferenced inline function has been removed
#pragma warning(disable: 4710)  // function not inlined
#endif
#endif

// vim:foldmethod=marker
