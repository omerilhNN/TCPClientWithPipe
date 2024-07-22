#include <windows.h>
#include <iostream>
#include <sstream>

#define PIPE_NAME L"\\\\.\\pipe\\AsyncPipe"
#define BUFFER_SIZE 1024
using namespace std;

void CALLBACK WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
void CALLBACK ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

HANDLE hPipe;
char buffer[BUFFER_SIZE];
OVERLAPPED writeOverlapped;
OVERLAPPED readOverlapped;

void SendDataToServer(double val1, double val2, char op) {
    hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "CreateFile failed, GLE=" << GetLastError() << endl;
        return;
    }

    ZeroMemory(&writeOverlapped, sizeof(OVERLAPPED));
    writeOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (writeOverlapped.hEvent == NULL) {
        cerr << "CreateEvent failed, GLE=" << GetLastError() << endl;
        CloseHandle(hPipe);
        return;
    }

    ostringstream oss;
    oss << val1 << " " << val2 << " " << op;
    string data = oss.str();
    memcpy(buffer, data.c_str(), data.size());

    BOOL writeResult = WriteFileEx(hPipe, buffer, data.size(), &writeOverlapped, WriteCompletionRoutine);
    if (!writeResult && GetLastError() != ERROR_IO_PENDING) {
        cerr << "WriteFileEx failed, GLE=" << GetLastError() << endl;
        CloseHandle(hPipe);
        CloseHandle(writeOverlapped.hEvent);
        return;
    }

    // Keep the main thread running to allow the completion routine to execute.
    while (true) {
        SleepEx(INFINITE, TRUE);
    }
}

void CALLBACK WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
    if (dwErrorCode != ERROR_SUCCESS) {
        cerr << "WriteCompletionRoutine failed, GLE=" << dwErrorCode << endl;
        CloseHandle(hPipe);
        CloseHandle(writeOverlapped.hEvent);
        return;
    }

    ZeroMemory(&readOverlapped, sizeof(OVERLAPPED));
    readOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (readOverlapped.hEvent == NULL) {
        cerr << "CreateEvent failed, GLE=" << GetLastError() << endl;
        CloseHandle(hPipe);
        CloseHandle(writeOverlapped.hEvent);
        return;
    }

    BOOL readResult = ReadFileEx(hPipe, buffer, BUFFER_SIZE, &readOverlapped, ReadCompletionRoutine);
    if (!readResult && GetLastError() != ERROR_IO_PENDING) {
        cerr << "ReadFileEx failed, GLE=" << GetLastError() << endl;
        CloseHandle(hPipe);
        CloseHandle(readOverlapped.hEvent);
        CloseHandle(writeOverlapped.hEvent);
        return;
    }
}

void CALLBACK ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
    if (dwErrorCode != ERROR_SUCCESS) {
        cerr << "ReadCompletionRoutine failed, GLE=" << dwErrorCode << endl;
        CloseHandle(hPipe);
        CloseHandle(readOverlapped.hEvent);
        CloseHandle(writeOverlapped.hEvent);
        return;
    }

    string result(buffer, dwNumberOfBytesTransfered);
    cout << "Received result: " << result << endl;

    CloseHandle(hPipe);
    CloseHandle(readOverlapped.hEvent);
    CloseHandle(writeOverlapped.hEvent);
}

int main() {
    double val1, val2;
    char op;
    printf("Enter val1,val2 and op\n");
    scanf_s("%lf %c %lf", &val1, &op, sizeof(char), &val2);


    SendDataToServer(val1, val2, op);
    return 0;
}
