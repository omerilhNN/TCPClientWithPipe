Asynchronous TCPClient using Named pipes in order to maintain IPC(Inter-process communication). -> For Windows

Functions that I have used:

ReadFileEx : Read from pipe.
WriteFileEx: Write to pipe.
WaitForSingleObjectEx: While waiting for that particular process, handle other operations.
SleepEx: Return according to callback functions behaviour.
