// NOTE: OLD FROM BIM
#include <windows.h>
// I would've made a struct for it, but I literally need this for no other reason right now.
// basically, this is system(const char*), but synchronous.
struct os_process_information {
    // OS Specific
    HANDLE process_handle;
    HANDLE thread_handle;
};

// NOTE(jerry): Does not do promise based stuff with callbacks.
struct os_process_information os_process_shell_start(char* shell_command) {
    STARTUPINFO         startup_information = {};
    PROCESS_INFORMATION process_information = {};

    {
        startup_information.cb = sizeof(STARTUPINFO);
        startup_information.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
        startup_information.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    bool create_process_result = CreateProcess(NULL, shell_command, NULL, NULL, false, 0, NULL, NULL, &startup_information, &process_information);
    return (struct os_process_information) {
        .process_handle = process_information.hProcess,
        .thread_handle  = process_information.hThread,
    };
}

void os_process_await_multiple(struct os_process_information* processes, int* return_codes, size_t count) {
    HANDLE handles[4096] = {};

    for (size_t process_index = 0; process_index < count; ++process_index) {
        handles[process_index] = processes[process_index].process_handle;
    }

    WaitForMultipleObjects(count, handles, true, INFINITE);

    if (return_codes) {
        for (size_t process_index = 0; process_index < count; ++process_index) {
            if (!GetExitCodeProcess(processes[process_index].process_handle, (LPDWORD)&return_codes[process_index])) {
                // error
            }

            CloseHandle(processes[process_index].process_handle);
            CloseHandle(processes[process_index].thread_handle);
        }
    }
}

int os_process_shell_start_and_run_synchronously(char* shell_command) {
    int return_code = 0;
    struct os_process_information process = os_process_shell_start(shell_command);
    os_process_await_multiple(&process, &return_code, 1);
    return return_code;
}
