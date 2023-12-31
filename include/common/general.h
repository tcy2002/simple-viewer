#pragma once

//// includes for different OS
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined LINUX
#include <sys/time.h>
#include <unistd.h>
#endif

//// inline: depends on OS
#ifdef _WIN32
#define COMMON_FORCE_INLINE __forceinline
#elif defined LINUX
#define COMMON_FORCE_INLINE __attribute__((always_inline))
#else
#define COMMON_FORCE_INLINE inline
#endif

//// timer: depends on OS
COMMON_FORCE_INLINE unsigned long long COMMON_GetTickCount() {
#ifdef _WIN32
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return t.QuadPart * 1000 / f.QuadPart;
#elif defined LINUX
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    return 0;
#endif
}
#ifdef _WIN32
#define COMMON_Sleep(t) Sleep(t)
#elif defined LINUX
#define COMMON_Sleep(t) usleep(t * 1000)
#else
#define COMMON_Sleep(t)
#endif

//// member getter and setter
#define COMMON_BOOL_SET_GET(name, Name) \
protected: \
bool _##name; \
public: \
void set##Name(bool t){ _##name = t; } \
bool is##Name() const { return _##name; } \
private:
#define COMMON_BOOL_GET(name, Name) \
protected: \
bool _##name; \
public: \
bool is##Name() const { return _##name; } \
private:
#define COMMON_MEMBER_SET_GET(T, name, Name) \
protected: \
T _##name; \
public: \
void set##Name(const T& t){ _##name = t; } \
const T& get##Name() const { return _##name; } \
private:
#define COMMON_MEMBER_GET(T, name, Name) \
protected: \
T _##name; \
public: \
const T& get##Name() const { return _##name; } \
private:

// member pointer getter and setter
#define COMMON_MEMBER_PTR_SET_GET(T, name, Name) \
protected: \
T *_##name; \
public: \
void set##Name(T *t) { delete _##name; _##name = t; } \
T* get##Name() const { return _##name; } \
private:
#define COMMON_MEMBER_PTR_GET(T, name, Name) \
protected: \
T *_##name; \
public: \
T* get##Name() const { return _##name; } \
private:

// member getter and setter (atomic)
#include <atomic>
#define COMMON_BOOL_SET_GET_ATOMIC(name, Name) \
protected: \
std::atomic<bool> _##name; \
public: \
void set##Name(bool t){ _##name = t; } \
bool is##Name() const { return _##name; } \
private:
#define COMMON_BOOL_GET_ATOMIC(name, Name) \
protected: \
std::atomic<bool> _##name; \
public: \
bool is##Name() const { return _##name; } \
private:
#define COMMON_MEMBER_SET_GET_ATOMIC(T, name, Name) \
protected: \
T _##name; \
public: \
void set##Name(const T& t){ _##name.store(t); } \
T get##Name() const { return _##name.load(); } \
private:
#define COMMON_MEMBER_GET_ATOMIC(T, name, Name) \
protected: \
std::atomic<T> _##name; \
public: \
T get##Name() const { return _##name.load(); } \
private:
