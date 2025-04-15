#include <Windows.h>
#include <string>
#include <array>
#include <vector>
#include <filesystem>

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64) && !defined(__CYGWIN__)
#define GEODE_IS_WINDOWS64 1
#define GEODE_CALL 
#else
#define GEODE_IS_WINDOWS32 1
#define GEODE_CALL __stdcall
#endif

#define GEODE_LSTR2(x) L#x
#define GEODE_LSTR(x) GEODE_LSTR2(x)

struct XINPUT_STATE;
struct XINPUT_CAPABILITIES;
struct XINPUT_VIBRATION;

constexpr static auto MAX_PATH_CHARS = 32768u;

static HMODULE getXInput() {
    static auto xinput = []() -> HMODULE {
        std::wstring path(MAX_PATH_CHARS, L'\0');
        auto size = GetSystemDirectoryW(path.data(), path.size());
        if (size) {
            path.resize(size);
            return LoadLibraryW((path + L"\\" + GEODE_LSTR(GEODE_LAUNCHER_OUTPUT_NAME)).c_str());
        }
        return NULL;
    }();

    return xinput;
}

static FARPROC getFP(const std::string& sym) {
    if (auto xinput = getXInput())
        return GetProcAddress(xinput, sym.c_str());

    return NULL;
}

template <typename T>
struct AddCallToType {};

template <typename Ret, typename... Args>
struct AddCallToType<Ret(*)(Args...)> {
    using type = Ret(GEODE_CALL *)(Args...);
};

#ifdef GEODE_IS_WINDOWS64
#pragma comment(linker, "/export:XInputGetState,@2")
#else
#pragma comment(linker, "/export:XInputGetState=_XInputGetState")
#endif
extern "C" DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState) {
    static auto fp = getFP("XInputGetState");
    if (fp) {
        using FPType = decltype(&XInputGetState);
        return reinterpret_cast<AddCallToType<FPType>::type>(fp)(dwUserIndex, pState);
    }

    return ERROR_DEVICE_NOT_CONNECTED;
}

#ifdef GEODE_IS_WINDOWS64
#pragma comment(linker, "/export:XInputSetState,@3")
#else
#pragma comment(linker, "/export:XInputSetState=_XInputSetState")
#endif
extern "C" DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
    static auto fp = getFP("XInputSetState");
    if (fp) {
        using FPType = decltype(&XInputSetState);
        return reinterpret_cast<AddCallToType<FPType>::type>(fp)(dwUserIndex, pVibration);
    }

    return ERROR_DEVICE_NOT_CONNECTED;
}

#ifdef GEODE_IS_WINDOWS64
#pragma comment(linker, "/export:XInputGetCapabilities,@4")
#else
#pragma comment(linker, "/export:XInputGetCapabilities=_XInputGetCapabilities")
#endif
extern "C" DWORD XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES *pCapabilities) {
    static auto fp = getFP("XInputGetCapabilities");
    if (fp) {
        using FPType = decltype(&XInputGetCapabilities);
        return reinterpret_cast<AddCallToType<FPType>::type>(fp)(dwUserIndex, dwFlags, pCapabilities);
    }

    return ERROR_DEVICE_NOT_CONNECTED;
}

static std::wstring getErrorString(DWORD error) {
    return L"Could not load Geode! Error code: " + std::to_wstring(error);
}

static GEODE_CALL DWORD errorThread(LPVOID param) {
    constexpr wchar_t REDIST_ERROR[] = L"Could not load Geode!\n"
        "This is likely due to an outdated redist package.\n"
        "Do you want to update Microsoft Visual C++ Redistributable 2022 to try to fix this issue?";
    constexpr wchar_t ALT_REDIST_ERROR[] = L"Could not load Geode!\n\n"
        "Please **delete** the following files from your Geometry Dash directory and try again: ";
    const DWORD error = reinterpret_cast<size_t>(param);

    if (error == ERROR_DLL_INIT_FAILED) {

        std::array<std::wstring, 4> msvcpDlls = {
            L"msvcp140.dll",
            L"msvcp140d.dll",
            L"vcruntime140.dll",
            L"vcruntime140d.dll"
        };

        std::vector<std::wstring> foundDlls;
        for(auto dll : msvcpDlls) {
            if(std::filesystem::exists(dll)) {
                foundDlls.push_back(dll);
            }
        }

        if(foundDlls.empty()) {
            const auto choice = MessageBoxW(NULL, REDIST_ERROR, L"Load failed", MB_YESNO | MB_ICONWARNING);
            if (choice == IDYES)
                ShellExecuteW(NULL, L"open", L"https://aka.ms/vs/17/release/vc_redist.x64.exe", NULL, NULL, SW_SHOWNORMAL);
        } else {
            std::wstring files = ALT_REDIST_ERROR;
            bool first = true;
            for(auto dll : foundDlls) {
                if(!first) files += L", ";
                files += dll;
                first = false;
            }
            const auto choice = MessageBoxW(NULL, files.c_str(), L"Load failed", MB_OK | MB_ICONWARNING);
        }

    } else {
        MessageBoxW(NULL, getErrorString(error).c_str(), L"Load failed" , MB_OK | MB_ICONWARNING);
    }

    return 0u;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID _) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);

        // This is UB.
        if (LoadLibraryW(GEODE_LSTR(GEODE_OUTPUT_NAME) L".dll") == NULL) {
            const auto param = reinterpret_cast<LPVOID>(static_cast<size_t>(GetLastError()));
            CreateThread(NULL, 0, &errorThread, param, 0, NULL);
        }
    }

    return TRUE;
}