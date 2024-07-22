#include <windows.h>
#include <iostream>
#include <sstream>

#define PIPE_NAME L"\\\\.\\pipe\\AsyncPipe"
#define BUFFER_SIZE 1024

void SendDataToServer(double val1, double val2, char op) {
    HANDLE hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFile failed, GLE=" << GetLastError() << std::endl;
        return;
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (overlapped.hEvent == NULL) {
        std::cerr << "CreateEvent failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(hPipe);
        return;
    }

    std::ostringstream oss;
    oss << val1 << " " << val2 << " " << op;
    std::string data = oss.str();

    DWORD bytesWritten = 0;
    BOOL writeResult = WriteFile(hPipe, data.c_str(), data.size(), &bytesWritten, &overlapped);
    if (!writeResult && GetLastError() != ERROR_IO_PENDING) {
        std::cerr << "WriteFile failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(overlapped.hEvent);
        CloseHandle(hPipe);
        return;
    }

    if (WaitForSingleObject(overlapped.hEvent, INFINITE) != WAIT_OBJECT_0) {
        std::cerr << "WaitForSingleObject failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(overlapped.hEvent);
        CloseHandle(hPipe);
        return;
    }

    if (!GetOverlappedResult(hPipe, &overlapped, &bytesWritten, FALSE)) {
        std::cerr << "GetOverlappedResult failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(overlapped.hEvent);
        CloseHandle(hPipe);
        return;
    }

    std::cout << "Sent: " << data << std::endl;

    char buffer[BUFFER_SIZE] = { 0 };
    DWORD bytesRead = 0;
    BOOL readResult = ReadFile(hPipe, buffer, BUFFER_SIZE, &bytesRead, &overlapped);
    if (!readResult && GetLastError() != ERROR_IO_PENDING) {
        std::cerr << "ReadFile failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(overlapped.hEvent);
        CloseHandle(hPipe);
        return;
    }

    if (WaitForSingleObject(overlapped.hEvent, INFINITE) != WAIT_OBJECT_0) { //Signalling ba�ar�s�z olduysa
        std::cerr << "WaitForSingleObject failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(overlapped.hEvent);
        CloseHandle(hPipe);
        return;
    }

    if (!GetOverlappedResult(hPipe, &overlapped, &bytesRead, FALSE)) {
        std::cerr << "GetOverlappedResult failed, GLE=" << GetLastError() << std::endl;
        CloseHandle(overlapped.hEvent);
        CloseHandle(hPipe);
        return;
    }

    std::string result(buffer, bytesRead);
    std::cout << "Received result: " << result << std::endl;

    CloseHandle(overlapped.hEvent);
    CloseHandle(hPipe);
}

int main() {
    double val1,val2;
    char op;
    printf("Enter val1, operator, val2: ");
    scanf_s("%lf %c %lf", &val1,&op, sizeof(char), &val2);

    SendDataToServer(val1, val2, op);
    return 0;
}