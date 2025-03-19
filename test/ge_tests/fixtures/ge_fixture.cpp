#include "ge_fixture.h"

#include <Windows.h>

void PMAFixture::RunTestApp()
{
    PROCESS_INFORMATION processInfo = {0};
    STARTUPINFO startupInfo = {0};
    startupInfo.cb = sizeof(startupInfo);

    // Tests expect the test_app to be installed in the same directory as the test executable
    constexpr auto testAppPath = "test_app.exe";

    if (CreateProcess(testAppPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
    {
        s_testAppPid = processInfo.dwProcessId;
        s_testAppHandle = processInfo.hProcess;
    }
    else
    {
        s_testAppPid = std::nullopt;
        s_testAppHandle = std::nullopt;
    }
    CloseHandle(processInfo.hThread);

    // Give the test app some time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void PMAFixture::KillTestApp()
{
    if (s_testAppPid.has_value())
    {
        HANDLE hProcess = s_testAppHandle.has_value() ? *s_testAppHandle :
                                                        OpenProcess(PROCESS_TERMINATE, FALSE, s_testAppPid.value());
        if (hProcess)
        {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
            s_testAppPid.reset();
            s_testAppHandle.reset();
        }
    }
}

bool PMAFixture::TestAppRunning()
{
    DWORD exCode = 0;
    if (!s_testAppHandle.has_value() || !GetExitCodeProcess(*s_testAppHandle, &exCode))
    {
        return false;
    }
    return exCode == STILL_ACTIVE;
}

unsigned long PMAFixture::GetTestAppPid()
{
    return s_testAppPid.value_or(0);
}

PMA::TargetProcess::Config PMAFixture::CreateTestAppConfig()
{
    return {GetTestAppPid(), {{"TestApp", "TestAppWindowClass"}}, {"test_shared.dll"}, "."};
}
