/*
 * This file contains an auto-generated table of NTSTATUS values mapped to DOS and POSIX error codes.
 */

#include <string_view>

namespace
{
static const struct { unsigned long status; unsigned long dos_err; int posix_err; std::string_view msg; } fallback_table[] = {
	{.status = 0, .dos_err = 0, .posix_err = 0, .msg = R"(STATUS_SUCCESS)"},
	{.status = 1, .dos_err = 731, .posix_err = 0, .msg = R"(STATUS_WAIT_1)"},
	{.status = 2, .dos_err = 732, .posix_err = 0, .msg = R"(STATUS_WAIT_2)"},
	{.status = 3, .dos_err = 733, .posix_err = 0, .msg = R"(STATUS_WAIT_3)"},
	{.status = 4, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot open the file.)"},
	{.status = 5, .dos_err = 317, .posix_err = 0, .msg = R"(Access is denied.)"},
	{.status = 6, .dos_err = 317, .posix_err = 0, .msg = R"(The handle is invalid.)"},
	{.status = 7, .dos_err = 317, .posix_err = 0, .msg = R"(The storage control blocks were destroyed.)"},
	{.status = 8, .dos_err = 317, .posix_err = 0, .msg = R"(Not enough memory resources are available to process this command.)"},
	{.status = 9, .dos_err = 317, .posix_err = 0, .msg = R"(The storage control block address is invalid.)"},
	{.status = 10, .dos_err = 317, .posix_err = 0, .msg = R"(The environment is incorrect.)"},
	{.status = 11, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to load a program with an incorrect format.)"},
	{.status = 12, .dos_err = 317, .posix_err = 0, .msg = R"(The access code is invalid.)"},
	{.status = 13, .dos_err = 317, .posix_err = 0, .msg = R"(The data is invalid.)"},
	{.status = 14, .dos_err = 317, .posix_err = 0, .msg = R"(Not enough memory resources are available to complete this operation.)"},
	{.status = 15, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot find the drive specified.)"},
	{.status = 16, .dos_err = 317, .posix_err = 0, .msg = R"(The directory cannot be removed.)"},
	{.status = 17, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot move the file to a different disk drive.)"},
	{.status = 18, .dos_err = 317, .posix_err = 0, .msg = R"(There are no more files.)"},
	{.status = 19, .dos_err = 317, .posix_err = 0, .msg = R"(The media is write protected.)"},
	{.status = 20, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot find the device specified.)"},
	{.status = 21, .dos_err = 317, .posix_err = 0, .msg = R"(The device is not ready.)"},
	{.status = 22, .dos_err = 317, .posix_err = 0, .msg = R"(The device does not recognize the command.)"},
	{.status = 23, .dos_err = 317, .posix_err = 0, .msg = R"(Data error (cyclic redundancy check).)"},
	{.status = 24, .dos_err = 317, .posix_err = 0, .msg = R"(The program issued a command but the command length is incorrect.)"},
	{.status = 25, .dos_err = 317, .posix_err = 0, .msg = R"(The drive cannot locate a specific area or track on the disk.)"},
	{.status = 26, .dos_err = 317, .posix_err = 0, .msg = R"(The specified disk or diskette cannot be accessed.)"},
	{.status = 27, .dos_err = 317, .posix_err = 0, .msg = R"(The drive cannot find the sector requested.)"},
	{.status = 28, .dos_err = 317, .posix_err = 0, .msg = R"(The printer is out of paper.)"},
	{.status = 29, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot write to the specified device.)"},
	{.status = 30, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot read from the specified device.)"},
	{.status = 31, .dos_err = 317, .posix_err = 0, .msg = R"(A device attached to the system is not functioning.)"},
	{.status = 32, .dos_err = 317, .posix_err = 0, .msg = R"(The process cannot access the file because it is being used by another process.)"},
	{.status = 33, .dos_err = 317, .posix_err = 0, .msg = R"(The process cannot access the file because another process has locked a portion of the file.)"},
	{.status = 34, .dos_err = 317, .posix_err = 0, .msg = R"(The wrong diskette is in the drive.

Insert %2 (Volume Serial Number: %3) into drive %1.)"},
	{.status = 36, .dos_err = 317, .posix_err = 0, .msg = R"(Too many files opened for sharing.)"},
	{.status = 38, .dos_err = 317, .posix_err = 0, .msg = R"(Reached the end of the file.)"},
	{.status = 39, .dos_err = 317, .posix_err = 0, .msg = R"(The disk is full.)"},
	{.status = 44, .dos_err = 317, .posix_err = 0, .msg = R"(The path cannot be traversed because it contains an untrusted mount point.)"},
	{.status = 50, .dos_err = 317, .posix_err = 0, .msg = R"(The request is not supported.)"},
	{.status = 51, .dos_err = 317, .posix_err = 0, .msg = R"(Windows cannot find the network path. Verify that the network path is correct and the destination computer is not busy or turned off. If Windows still cannot find the network path, contact your network administrator.)"},
	{.status = 52, .dos_err = 317, .posix_err = 0, .msg = R"(You were not connected because a duplicate name exists on the network. If joining a domain, go to System in Control Panel to change the computer name and try again. If joining a workgroup, choose another workgroup name.)"},
	{.status = 53, .dos_err = 317, .posix_err = 0, .msg = R"(The network path was not found.)"},
	{.status = 54, .dos_err = 317, .posix_err = 0, .msg = R"(The network is busy.)"},
	{.status = 55, .dos_err = 317, .posix_err = 0, .msg = R"(The specified network resource or device is no longer available.)"},
	{.status = 56, .dos_err = 317, .posix_err = 0, .msg = R"(The network BIOS command limit has been reached.)"},
	{.status = 57, .dos_err = 317, .posix_err = 0, .msg = R"(A network adapter hardware error occurred.)"},
	{.status = 58, .dos_err = 317, .posix_err = 0, .msg = R"(The specified server cannot perform the requested operation.)"},
	{.status = 59, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected network error occurred.)"},
	{.status = 60, .dos_err = 317, .posix_err = 0, .msg = R"(The remote adapter is not compatible.)"},
	{.status = 61, .dos_err = 317, .posix_err = 0, .msg = R"(The printer queue is full.)"},
	{.status = 62, .dos_err = 317, .posix_err = 0, .msg = R"(Space to store the file waiting to be printed is not available on the server.)"},
	{.status = 63, .dos_err = 734, .posix_err = 0, .msg = R"(STATUS_WAIT_63)"},
	{.status = 64, .dos_err = 317, .posix_err = 0, .msg = R"(The specified network name is no longer available.)"},
	{.status = 65, .dos_err = 317, .posix_err = 0, .msg = R"(Network access is denied.)"},
	{.status = 66, .dos_err = 317, .posix_err = 0, .msg = R"(The network resource type is not correct.)"},
	{.status = 67, .dos_err = 317, .posix_err = 0, .msg = R"(The network name cannot be found.)"},
	{.status = 68, .dos_err = 317, .posix_err = 0, .msg = R"(The name limit for the local computer network adapter card was exceeded.)"},
	{.status = 69, .dos_err = 317, .posix_err = 0, .msg = R"(The network BIOS session limit was exceeded.)"},
	{.status = 70, .dos_err = 317, .posix_err = 0, .msg = R"(The remote server has been paused or is in the process of being started.)"},
	{.status = 71, .dos_err = 317, .posix_err = 0, .msg = R"(No more connections can be made to this remote computer at this time because there are already as many connections as the computer can accept.)"},
	{.status = 72, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer or disk device has been paused.)"},
	{.status = 80, .dos_err = 317, .posix_err = 0, .msg = R"(The file exists.)"},
	{.status = 82, .dos_err = 317, .posix_err = 0, .msg = R"(The directory or file cannot be created.)"},
	{.status = 83, .dos_err = 317, .posix_err = 0, .msg = R"(Fail on INT 24.)"},
	{.status = 84, .dos_err = 317, .posix_err = 0, .msg = R"(Storage to process this request is not available.)"},
	{.status = 85, .dos_err = 317, .posix_err = 0, .msg = R"(The local device name is already in use.)"},
	{.status = 86, .dos_err = 317, .posix_err = 0, .msg = R"(The specified network password is not correct.)"},
	{.status = 87, .dos_err = 317, .posix_err = 0, .msg = R"(The parameter is incorrect.)"},
	{.status = 88, .dos_err = 317, .posix_err = 0, .msg = R"(A write fault occurred on the network.)"},
	{.status = 89, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot start another process at this time.)"},
	{.status = 100, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot create another system semaphore.)"},
	{.status = 101, .dos_err = 317, .posix_err = 0, .msg = R"(The exclusive semaphore is owned by another process.)"},
	{.status = 102, .dos_err = 317, .posix_err = 0, .msg = R"(The semaphore is set and cannot be closed.)"},
	{.status = 103, .dos_err = 317, .posix_err = 0, .msg = R"(The semaphore cannot be set again.)"},
	{.status = 104, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot request exclusive semaphores at interrupt time.)"},
	{.status = 105, .dos_err = 317, .posix_err = 0, .msg = R"(The previous ownership of this semaphore has ended.)"},
	{.status = 106, .dos_err = 317, .posix_err = 0, .msg = R"(Insert the diskette for drive %1.)"},
	{.status = 107, .dos_err = 317, .posix_err = 0, .msg = R"(The program stopped because an alternate diskette was not inserted.)"},
	{.status = 108, .dos_err = 317, .posix_err = 0, .msg = R"(The disk is in use or locked by another process.)"},
	{.status = 109, .dos_err = 317, .posix_err = 0, .msg = R"(The pipe has been ended.)"},
	{.status = 110, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot open the device or file specified.)"},
	{.status = 111, .dos_err = 317, .posix_err = 0, .msg = R"(The file name is too long.)"},
	{.status = 112, .dos_err = 317, .posix_err = 0, .msg = R"(There is not enough space on the disk.)"},
	{.status = 113, .dos_err = 317, .posix_err = 0, .msg = R"(No more internal file identifiers available.)"},
	{.status = 114, .dos_err = 317, .posix_err = 0, .msg = R"(The target internal file identifier is incorrect.)"},
	{.status = 117, .dos_err = 317, .posix_err = 0, .msg = R"(The IOCTL call made by the application program is not correct.)"},
	{.status = 118, .dos_err = 317, .posix_err = 0, .msg = R"(The verify-on-write switch parameter value is not correct.)"},
	{.status = 119, .dos_err = 317, .posix_err = 0, .msg = R"(The system does not support the command requested.)"},
	{.status = 120, .dos_err = 317, .posix_err = 0, .msg = R"(This function is not supported on this system.)"},
	{.status = 121, .dos_err = 317, .posix_err = 0, .msg = R"(The semaphore timeout period has expired.)"},
	{.status = 122, .dos_err = 317, .posix_err = 0, .msg = R"(The data area passed to a system call is too small.)"},
	{.status = 123, .dos_err = 317, .posix_err = 0, .msg = R"(The filename, directory name, or volume label syntax is incorrect.)"},
	{.status = 124, .dos_err = 317, .posix_err = 0, .msg = R"(The system call level is not correct.)"},
	{.status = 125, .dos_err = 317, .posix_err = 0, .msg = R"(The disk has no volume label.)"},
	{.status = 126, .dos_err = 317, .posix_err = 0, .msg = R"(The specified module could not be found.)"},
	{.status = 127, .dos_err = 317, .posix_err = 0, .msg = R"(The specified procedure could not be found.)"},
	{.status = 128, .dos_err = 128, .posix_err = 0, .msg = R"(STATUS_ABANDONED_WAIT_0)"},
	{.status = 129, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 application cannot be run in Win32 mode.)"},
	{.status = 130, .dos_err = 317, .posix_err = 0, .msg = R"(Attempt to use a file handle to an open disk partition for an operation other than raw disk I/O.)"},
	{.status = 131, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to move the file pointer before the beginning of the file.)"},
	{.status = 132, .dos_err = 317, .posix_err = 0, .msg = R"(The file pointer cannot be set on the specified device or file.)"},
	{.status = 133, .dos_err = 317, .posix_err = 0, .msg = R"(A JOIN or SUBST command cannot be used for a drive that contains previously joined drives.)"},
	{.status = 134, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to use a JOIN or SUBST command on a drive that has already been joined.)"},
	{.status = 135, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to use a JOIN or SUBST command on a drive that has already been substituted.)"},
	{.status = 136, .dos_err = 317, .posix_err = 0, .msg = R"(The system tried to delete the JOIN of a drive that is not joined.)"},
	{.status = 137, .dos_err = 317, .posix_err = 0, .msg = R"(The system tried to delete the substitution of a drive that is not substituted.)"},
	{.status = 138, .dos_err = 317, .posix_err = 0, .msg = R"(The system tried to join a drive to a directory on a joined drive.)"},
	{.status = 139, .dos_err = 317, .posix_err = 0, .msg = R"(The system tried to substitute a drive to a directory on a substituted drive.)"},
	{.status = 140, .dos_err = 317, .posix_err = 0, .msg = R"(The system tried to join a drive to a directory on a substituted drive.)"},
	{.status = 141, .dos_err = 317, .posix_err = 0, .msg = R"(The system tried to SUBST a drive to a directory on a joined drive.)"},
	{.status = 142, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot perform a JOIN or SUBST at this time.)"},
	{.status = 143, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot join or substitute a drive to or for a directory on the same drive.)"},
	{.status = 144, .dos_err = 317, .posix_err = 0, .msg = R"(The directory is not a subdirectory of the root directory.)"},
	{.status = 145, .dos_err = 317, .posix_err = 0, .msg = R"(The directory is not empty.)"},
	{.status = 146, .dos_err = 317, .posix_err = 0, .msg = R"(The path specified is being used in a substitute.)"},
	{.status = 147, .dos_err = 317, .posix_err = 0, .msg = R"(Not enough resources are available to process this command.)"},
	{.status = 148, .dos_err = 317, .posix_err = 0, .msg = R"(The path specified cannot be used at this time.)"},
	{.status = 149, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to join or substitute a drive for which a directory on the drive is the target of a previous substitute.)"},
	{.status = 150, .dos_err = 317, .posix_err = 0, .msg = R"(System trace information was not specified in your CONFIG.SYS file, or tracing is disallowed.)"},
	{.status = 151, .dos_err = 317, .posix_err = 0, .msg = R"(The number of specified semaphore events for DosMuxSemWait is not correct.)"},
	{.status = 152, .dos_err = 317, .posix_err = 0, .msg = R"(DosMuxSemWait did not execute; too many semaphores are already set.)"},
	{.status = 153, .dos_err = 317, .posix_err = 0, .msg = R"(The DosMuxSemWait list is not correct.)"},
	{.status = 154, .dos_err = 317, .posix_err = 0, .msg = R"(The volume label you entered exceeds the label character limit of the target file system.)"},
	{.status = 155, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot create another thread.)"},
	{.status = 156, .dos_err = 317, .posix_err = 0, .msg = R"(The recipient process has refused the signal.)"},
	{.status = 157, .dos_err = 317, .posix_err = 0, .msg = R"(The segment is already discarded and cannot be locked.)"},
	{.status = 158, .dos_err = 317, .posix_err = 0, .msg = R"(The segment is already unlocked.)"},
	{.status = 159, .dos_err = 317, .posix_err = 0, .msg = R"(The address for the thread ID is not correct.)"},
	{.status = 160, .dos_err = 317, .posix_err = 0, .msg = R"(One or more arguments are not correct.)"},
	{.status = 161, .dos_err = 317, .posix_err = 0, .msg = R"(The specified path is invalid.)"},
	{.status = 162, .dos_err = 317, .posix_err = 0, .msg = R"(A signal is already pending.)"},
	{.status = 164, .dos_err = 317, .posix_err = 0, .msg = R"(No more threads can be created in the system.)"},
	{.status = 167, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to lock a region of a file.)"},
	{.status = 170, .dos_err = 317, .posix_err = 0, .msg = R"(The requested resource is in use.)"},
	{.status = 171, .dos_err = 317, .posix_err = 0, .msg = R"(Device's command support detection is in progress.)"},
	{.status = 173, .dos_err = 317, .posix_err = 0, .msg = R"(A lock request was not outstanding for the supplied cancel region.)"},
	{.status = 174, .dos_err = 317, .posix_err = 0, .msg = R"(The file system does not support atomic changes to the lock type.)"},
	{.status = 180, .dos_err = 317, .posix_err = 0, .msg = R"(The system detected a segment number that was not correct.)"},
	{.status = 182, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 183, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot create a file when that file already exists.)"},
	{.status = 186, .dos_err = 317, .posix_err = 0, .msg = R"(The flag passed is not correct.)"},
	{.status = 187, .dos_err = 317, .posix_err = 0, .msg = R"(The specified system semaphore name was not found.)"},
	{.status = 188, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 189, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 190, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 191, .dos_err = 736, .posix_err = 0, .msg = R"(STATUS_ABANDONED_WAIT_63)"},
	{.status = 192, .dos_err = 737, .posix_err = 0, .msg = R"(STATUS_USER_APC)"},
	{.status = 193, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a valid Win32 application.)"},
	{.status = 194, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 195, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 196, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run this application program.)"},
	{.status = 197, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system is not presently configured to run this application.)"},
	{.status = 198, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 199, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run this application program.)"},
	{.status = 200, .dos_err = 317, .posix_err = 0, .msg = R"(The code segment cannot be greater than or equal to 64K.)"},
	{.status = 201, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 202, .dos_err = 317, .posix_err = 0, .msg = R"(The operating system cannot run %1.)"},
	{.status = 203, .dos_err = 317, .posix_err = 0, .msg = R"(The system could not find the environment option that was entered.)"},
	{.status = 205, .dos_err = 317, .posix_err = 0, .msg = R"(No process in the command subtree has a signal handler.)"},
	{.status = 206, .dos_err = 317, .posix_err = 0, .msg = R"(The filename or extension is too long.)"},
	{.status = 207, .dos_err = 317, .posix_err = 0, .msg = R"(The ring 2 stack is in use.)"},
	{.status = 208, .dos_err = 317, .posix_err = 0, .msg = R"(The global filename characters, * or ?, are entered incorrectly or too many global filename characters are specified.)"},
	{.status = 209, .dos_err = 317, .posix_err = 0, .msg = R"(The signal being posted is not correct.)"},
	{.status = 210, .dos_err = 317, .posix_err = 0, .msg = R"(The signal handler cannot be set.)"},
	{.status = 212, .dos_err = 317, .posix_err = 0, .msg = R"(The segment is locked and cannot be reallocated.)"},
	{.status = 214, .dos_err = 317, .posix_err = 0, .msg = R"(Too many dynamic-link modules are attached to this program or dynamic-link module.)"},
	{.status = 215, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot nest calls to LoadModule.)"},
	{.status = 216, .dos_err = 317, .posix_err = 0, .msg = R"(This version of %1 is not compatible with the version of Windows you're running. Check your computer's system information and then contact the software publisher.)"},
	{.status = 217, .dos_err = 317, .posix_err = 0, .msg = R"(The image file %1 is signed, unable to modify.)"},
	{.status = 218, .dos_err = 317, .posix_err = 0, .msg = R"(The image file %1 is strong signed, unable to modify.)"},
	{.status = 220, .dos_err = 317, .posix_err = 0, .msg = R"(This file is checked out or locked for editing by another user.)"},
	{.status = 221, .dos_err = 317, .posix_err = 0, .msg = R"(The file must be checked out before saving changes.)"},
	{.status = 222, .dos_err = 317, .posix_err = 0, .msg = R"(The file type being saved or retrieved has been blocked.)"},
	{.status = 223, .dos_err = 317, .posix_err = 0, .msg = R"(The file size exceeds the limit allowed and cannot be saved.)"},
	{.status = 224, .dos_err = 317, .posix_err = 0, .msg = R"(Access Denied. Before opening files in this location, you must first add the web site to your trusted sites list, browse to the web site, and select the option to login automatically.)"},
	{.status = 225, .dos_err = 317, .posix_err = 0, .msg = R"(Operation did not complete successfully because the file contains a virus or potentially unwanted software.)"},
	{.status = 226, .dos_err = 317, .posix_err = 0, .msg = R"(This file contains a virus or potentially unwanted software and cannot be opened. Due to the nature of this virus or potentially unwanted software, the file has been removed from this location.)"},
	{.status = 229, .dos_err = 317, .posix_err = 0, .msg = R"(The pipe is local.)"},
	{.status = 230, .dos_err = 317, .posix_err = 0, .msg = R"(The pipe state is invalid.)"},
	{.status = 231, .dos_err = 317, .posix_err = 0, .msg = R"(All pipe instances are busy.)"},
	{.status = 232, .dos_err = 317, .posix_err = 0, .msg = R"(The pipe is being closed.)"},
	{.status = 233, .dos_err = 317, .posix_err = 0, .msg = R"(No process is on the other end of the pipe.)"},
	{.status = 234, .dos_err = 317, .posix_err = 0, .msg = R"(More data is available.)"},
	{.status = 235, .dos_err = 317, .posix_err = 0, .msg = R"(The action requested resulted in no work being done. Error-style clean-up has been performed.)"},
	{.status = 240, .dos_err = 317, .posix_err = 0, .msg = R"(The session was canceled.)"},
	{.status = 254, .dos_err = 317, .posix_err = 0, .msg = R"(The specified extended attribute name was invalid.)"},
	{.status = 255, .dos_err = 0, .posix_err = 0, .msg = R"(The requested action was completed by an earlier operation.)"},
	{.status = 256, .dos_err = 738, .posix_err = 0, .msg = R"(STATUS_KERNEL_APC)"},
	{.status = 257, .dos_err = 739, .posix_err = 0, .msg = R"(STATUS_ALERTED)"},
	{.status = 258, .dos_err = 1460, .posix_err = 138, .msg = R"(STATUS_TIMEOUT)"},
	{.status = 259, .dos_err = 997, .posix_err = 0, .msg = R"(The operation that was requested is pending completion.)"},
	{.status = 260, .dos_err = 741, .posix_err = 0, .msg = R"(A reparse should be performed by the Object Manager since the name of the file resulted in a symbolic link.)"},
	{.status = 261, .dos_err = 234, .posix_err = 0, .msg = R"(Returned by enumeration APIs to indicate more information is available to successive calls.)"},
	{.status = 262, .dos_err = 1300, .posix_err = 0, .msg = R"(Indicates not all privileges or groups referenced are assigned to the caller.

This allows, for example, all privileges to be disabled without having to know exactly which privileges are assigned.)"},
	{.status = 263, .dos_err = 1301, .posix_err = 0, .msg = R"(Some of the information to be translated has not been translated.)"},
	{.status = 264, .dos_err = 742, .posix_err = 0, .msg = R"(An open/create operation completed while an oplock break is underway.)"},
	{.status = 265, .dos_err = 743, .posix_err = 0, .msg = R"(A new volume has been mounted by a file system.)"},
	{.status = 266, .dos_err = 744, .posix_err = 0, .msg = R"(This success level status indicates that the transaction state already exists for the registry sub-tree, but that a transaction commit was previously aborted. The commit has now been completed.)"},
	{.status = 267, .dos_err = 745, .posix_err = 0, .msg = R"(This indicates that a notify change request has been completed due to closing the handle which made the notify change request.)"},
	{.status = 268, .dos_err = 1022, .posix_err = 0, .msg = R"(This indicates that a notify change request is being completed and that the information is not being returned in the caller's buffer.

The caller now needs to enumerate the files to find the changes.)"},
	{.status = 269, .dos_err = 1302, .posix_err = 0, .msg = R"({No Quotas}

No system quota limits are specifically set for this account.)"},
	{.status = 270, .dos_err = 746, .posix_err = 0, .msg = R"({Connect Failure on Primary Transport}

An attempt was made to connect to the remote server %hs on the primary transport, but the connection failed.

The computer WAS able to connect on a secondary transport.)"},
	{.status = 272, .dos_err = 747, .posix_err = 0, .msg = R"(Page fault was a transition fault.)"},
	{.status = 273, .dos_err = 748, .posix_err = 0, .msg = R"(Page fault was a demand zero fault.)"},
	{.status = 274, .dos_err = 749, .posix_err = 0, .msg = R"(Page fault was a demand zero fault.)"},
	{.status = 275, .dos_err = 750, .posix_err = 0, .msg = R"(Page fault was a demand zero fault.)"},
	{.status = 276, .dos_err = 751, .posix_err = 0, .msg = R"(Page fault was satisfied by reading from a secondary storage device.)"},
	{.status = 277, .dos_err = 752, .posix_err = 0, .msg = R"(Cached page was locked during operation.)"},
	{.status = 278, .dos_err = 753, .posix_err = 0, .msg = R"(Crash dump exists in paging file.)"},
	{.status = 279, .dos_err = 754, .posix_err = 0, .msg = R"(Specified buffer contains all zeros.)"},
	{.status = 280, .dos_err = 755, .posix_err = 0, .msg = R"(A reparse should be performed by the Object Manager since the name of the file resulted in a symbolic link.)"},
	{.status = 281, .dos_err = 756, .posix_err = 0, .msg = R"(The device has succeeded a query-stop and its resource requirements have changed.)"},
	{.status = 282, .dos_err = 317, .posix_err = 0, .msg = R"(The mounted file system does not support extended attributes.)"},
	{.status = 288, .dos_err = 757, .posix_err = 0, .msg = R"(The translator has translated these resources into the global space and no further translations should be performed.)"},
	{.status = 289, .dos_err = 8201, .posix_err = 0, .msg = R"(The directory service evaluated group memberships locally, as it was unable to contact a global catalog server.)"},
	{.status = 290, .dos_err = 758, .posix_err = 0, .msg = R"(A process being terminated has no threads to terminate.)"},
	{.status = 291, .dos_err = 759, .posix_err = 0, .msg = R"(The specified process is not part of a job.)"},
	{.status = 292, .dos_err = 760, .posix_err = 0, .msg = R"(The specified process is part of a job.)"},
	{.status = 293, .dos_err = 761, .posix_err = 0, .msg = R"({Volume Shadow Copy Service}

The system is now ready for hibernation.)"},
	{.status = 294, .dos_err = 762, .posix_err = 0, .msg = R"(A file system or file system filter driver has successfully completed an FsFilter operation.)"},
	{.status = 295, .dos_err = 763, .posix_err = 0, .msg = R"(The specified interrupt vector was already connected.)"},
	{.status = 296, .dos_err = 764, .posix_err = 0, .msg = R"(The specified interrupt vector is still connected.)"},
	{.status = 297, .dos_err = 0, .posix_err = 0, .msg = R"(The current process is a cloned process.)"},
	{.status = 298, .dos_err = 0, .posix_err = 0, .msg = R"(The file was locked and all users of the file can only read.)"},
	{.status = 299, .dos_err = 0, .posix_err = 0, .msg = R"(The file was locked and at least one user of the file can write.)"},
	{.status = 300, .dos_err = 0, .posix_err = 0, .msg = R"(The file image hash is valid.)"},
	{.status = 301, .dos_err = 0, .posix_err = 0, .msg = R"(The file catalog hash is valid.)"},
	{.status = 302, .dos_err = 0, .posix_err = 0, .msg = R"(The file hash is valid and uses strong code integrity.)"},
	{.status = 303, .dos_err = 0, .posix_err = 0, .msg = R"(At least a portion of IO range intersects with a ghosted file range.)"},
	{.status = 304, .dos_err = 0, .posix_err = 0, .msg = R"(A completed operation may have overwritten previous data.)"},
	{.status = 305, .dos_err = 317, .posix_err = 0, .msg = R"(Short names are not enabled on this volume.)"},
	{.status = 306, .dos_err = 317, .posix_err = 0, .msg = R"(The security stream for the given volume is in an inconsistent state.

Please run CHKDSK on the volume.)"},
	{.status = 307, .dos_err = 317, .posix_err = 0, .msg = R"(A requested file lock operation cannot be processed due to an invalid byte range.)"},
	{.status = 308, .dos_err = 317, .posix_err = 0, .msg = R"(The subsystem needed to support the image type is not present.)"},
	{.status = 309, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file already has a notification GUID associated with it.)"},
	{.status = 310, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid exception handler routine has been detected.)"},
	{.status = 311, .dos_err = 317, .posix_err = 0, .msg = R"(Duplicate privileges were specified for the token.)"},
	{.status = 312, .dos_err = 317, .posix_err = 0, .msg = R"(No ranges for the specified operation were able to be processed.)"},
	{.status = 313, .dos_err = 317, .posix_err = 0, .msg = R"(Operation is not allowed on a file system internal file.)"},
	{.status = 314, .dos_err = 317, .posix_err = 0, .msg = R"(The physical resources of this disk have been exhausted.)"},
	{.status = 315, .dos_err = 317, .posix_err = 0, .msg = R"(The token representing the data is invalid.)"},
	{.status = 316, .dos_err = 317, .posix_err = 0, .msg = R"(The device does not support the command feature.)"},
	{.status = 317, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot find message text for message number 0x%1 in the message file for %2.)"},
	{.status = 318, .dos_err = 317, .posix_err = 0, .msg = R"(The scope specified was not found.)"},
	{.status = 319, .dos_err = 317, .posix_err = 0, .msg = R"(The Central Access Policy specified is not defined on the target machine.)"},
	{.status = 320, .dos_err = 317, .posix_err = 0, .msg = R"(The Central Access Policy obtained from Active Directory is invalid.)"},
	{.status = 321, .dos_err = 317, .posix_err = 0, .msg = R"(The device is unreachable.)"},
	{.status = 322, .dos_err = 317, .posix_err = 0, .msg = R"(The target device has insufficient resources to complete the operation.)"},
	{.status = 323, .dos_err = 317, .posix_err = 0, .msg = R"(A data integrity checksum error occurred. Data in the file stream is corrupt.)"},
	{.status = 324, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to modify both a KERNEL and normal Extended Attribute (EA) in the same operation.)"},
	{.status = 326, .dos_err = 317, .posix_err = 0, .msg = R"(Device does not support file-level TRIM.)"},
	{.status = 327, .dos_err = 317, .posix_err = 0, .msg = R"(The command specified a data offset that does not align to the device's granularity/alignment.)"},
	{.status = 328, .dos_err = 317, .posix_err = 0, .msg = R"(The command specified an invalid field in its parameter list.)"},
	{.status = 329, .dos_err = 317, .posix_err = 0, .msg = R"(An operation is currently in progress with the device.)"},
	{.status = 330, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to send down the command via an invalid path to the target device.)"},
	{.status = 331, .dos_err = 317, .posix_err = 0, .msg = R"(The command specified a number of descriptors that exceeded the maximum supported by the device.)"},
	{.status = 332, .dos_err = 317, .posix_err = 0, .msg = R"(Scrub is disabled on the specified file.)"},
	{.status = 333, .dos_err = 317, .posix_err = 0, .msg = R"(The storage device does not provide redundancy.)"},
	{.status = 334, .dos_err = 317, .posix_err = 0, .msg = R"(An operation is not supported on a resident file.)"},
	{.status = 335, .dos_err = 317, .posix_err = 0, .msg = R"(An operation is not supported on a compressed file.)"},
	{.status = 336, .dos_err = 317, .posix_err = 0, .msg = R"(An operation is not supported on a directory.)"},
	{.status = 337, .dos_err = 317, .posix_err = 0, .msg = R"(The specified copy of the requested data could not be read.)"},
	{.status = 338, .dos_err = 317, .posix_err = 0, .msg = R"(The specified data could not be written to any of the copies.)"},
	{.status = 339, .dos_err = 317, .posix_err = 0, .msg = R"(One or more copies of data on this device may be out of sync. No writes may be performed until a data integrity scan is completed.)"},
	{.status = 340, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied kernel information version is invalid.)"},
	{.status = 341, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied PEP information version is invalid.)"},
	{.status = 342, .dos_err = 317, .posix_err = 0, .msg = R"(This object is not externally backed by any provider.)"},
	{.status = 343, .dos_err = 317, .posix_err = 0, .msg = R"(The external backing provider is not recognized.)"},
	{.status = 344, .dos_err = 317, .posix_err = 0, .msg = R"(Compressing this object would not save space.)"},
	{.status = 345, .dos_err = 317, .posix_err = 0, .msg = R"(The request failed due to a storage topology ID mismatch.)"},
	{.status = 346, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was blocked by parental controls.)"},
	{.status = 347, .dos_err = 317, .posix_err = 0, .msg = R"(A file system block being referenced has already reached the maximum reference count and can't be referenced any further.)"},
	{.status = 348, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation failed because the file stream is marked to disallow writes.)"},
	{.status = 349, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation failed with an architecture-specific failure code.)"},
	{.status = 350, .dos_err = 317, .posix_err = 0, .msg = R"(No action was taken as a system reboot is required.)"},
	{.status = 351, .dos_err = 317, .posix_err = 0, .msg = R"(The shutdown operation failed.)"},
	{.status = 352, .dos_err = 317, .posix_err = 0, .msg = R"(The restart operation failed.)"},
	{.status = 353, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of sessions has been reached.)"},
	{.status = 354, .dos_err = 317, .posix_err = 0, .msg = R"(Windows Information Protection policy does not allow access to this network resource.)"},
	{.status = 355, .dos_err = 317, .posix_err = 0, .msg = R"(The device hint name buffer is too small to receive the remaining name.)"},
	{.status = 356, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation was blocked by Windows Information Protection policy. For more information, contact your system administrator.)"},
	{.status = 357, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation cannot be performed because hardware or software configuration of the device does not comply with Windows Information Protection under Lock policy. Please, verify that user PIN has been created. For more information, contact your system administrator.)"},
	{.status = 358, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud sync root metadata is corrupted.)"},
	{.status = 359, .dos_err = 317, .posix_err = 0, .msg = R"(The device is in maintenance mode.)"},
	{.status = 360, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not supported on a DAX volume.)"},
	{.status = 361, .dos_err = 317, .posix_err = 0, .msg = R"(The volume has active DAX mappings.)"},
	{.status = 362, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file provider is not running.)"},
	{.status = 363, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file metadata is corrupt and unreadable.)"},
	{.status = 364, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file metadata is too large.)"},
	{.status = 365, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file property is too large.)"},
	{.status = 366, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file property is possibly corrupt. The on-disk checksum does not match the computed checksum.)"},
	{.status = 367, .dos_err = 317, .posix_err = 0, .msg = R"(The process creation has been blocked.)"},
	{.status = 368, .dos_err = 317, .posix_err = 0, .msg = R"(The storage device has lost data or persistence.)"},
	{.status = 369, .dos_err = 317, .posix_err = 0, .msg = R"(The provider that supports file system virtualization is temporarily unavailable.)"},
	{.status = 370, .dos_err = 317, .posix_err = 0, .msg = R"(The metadata for file system virtualization is corrupt and unreadable.)"},
	{.status = 371, .dos_err = 317, .posix_err = 0, .msg = R"(The provider that supports file system virtualization is too busy to complete this operation.)"},
	{.status = 372, .dos_err = 317, .posix_err = 0, .msg = R"(The provider that supports file system virtualization is unknown.)"},
	{.status = 373, .dos_err = 317, .posix_err = 0, .msg = R"(GDI handles were potentially leaked by the application.)"},
	{.status = 374, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of cloud file properties has been reached.)"},
	{.status = 375, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the cloud file property store is not supported.)"},
	{.status = 376, .dos_err = 317, .posix_err = 0, .msg = R"(The file is not a cloud file.)"},
	{.status = 377, .dos_err = 317, .posix_err = 0, .msg = R"(The file is not in sync with the cloud.)"},
	{.status = 378, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud sync root is already connected with another cloud sync provider.)"},
	{.status = 379, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is not supported by the cloud sync provider.)"},
	{.status = 380, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation is invalid.)"},
	{.status = 381, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation is not supported on a read-only volume.)"},
	{.status = 382, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is reserved for a connected cloud sync provider.)"},
	{.status = 383, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud sync provider failed to validate the downloaded data.)"},
	{.status = 384, .dos_err = 317, .posix_err = 0, .msg = R"(You can't connect to the file share because it's not secure. This share requires the obsolete SMB1 protocol, which is unsafe and could expose your system to attack.

Your system requires SMB2 or higher. For more info on resolving this issue, see: https://go.microsoft.com/fwlink/?linkid=852747)"},
	{.status = 385, .dos_err = 317, .posix_err = 0, .msg = R"(The virtualization operation is not allowed on the file in its current state.)"},
	{.status = 386, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud sync provider failed user authentication.)"},
	{.status = 387, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud sync provider failed to perform the operation due to low system resources.)"},
	{.status = 388, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud sync provider failed to perform the operation due to network being unavailable.)"},
	{.status = 389, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation was unsuccessful.)"},
	{.status = 390, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is only supported on files under a cloud sync root.)"},
	{.status = 391, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed on cloud files in use.)"},
	{.status = 392, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed on pinned cloud files.)"},
	{.status = 393, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation was aborted.)"},
	{.status = 394, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file's property store is corrupt.)"},
	{.status = 395, .dos_err = 317, .posix_err = 0, .msg = R"(Access to the cloud file is denied.)"},
	{.status = 396, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation cannot be performed on a file with incompatible hardlinks.)"},
	{.status = 397, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed due to a conflicting cloud file property lock.)"},
	{.status = 398, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation was canceled by user.)"},
	{.status = 399, .dos_err = 317, .posix_err = 0, .msg = R"(An externally encrypted syskey has been configured, but the system no longer supports this feature.  Please see https://go.microsoft.com/fwlink/?linkid=851152 for more information.)"},
	{.status = 400, .dos_err = 317, .posix_err = 0, .msg = R"(The thread is already in background processing mode.)"},
	{.status = 401, .dos_err = 317, .posix_err = 0, .msg = R"(The thread is not in background processing mode.)"},
	{.status = 402, .dos_err = 317, .posix_err = 0, .msg = R"(The process is already in background processing mode.)"},
	{.status = 403, .dos_err = 317, .posix_err = 0, .msg = R"(The process is not in background processing mode.)"},
	{.status = 404, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud file provider exited unexpectedly.)"},
	{.status = 405, .dos_err = 317, .posix_err = 0, .msg = R"(The file is not a cloud sync root.)"},
	{.status = 406, .dos_err = 317, .posix_err = 0, .msg = R"(The read or write operation to an encrypted file could not be completed because the file can only be accessed when the device is unlocked.)"},
	{.status = 407, .dos_err = 317, .posix_err = 0, .msg = R"(The volume is not cluster aligned on the disk.)"},
	{.status = 408, .dos_err = 317, .posix_err = 0, .msg = R"(No physically aligned free space was found on the volume.)"},
	{.status = 409, .dos_err = 317, .posix_err = 0, .msg = R"(The APPX file can not be accessed because it is not encrypted as expected.)"},
	{.status = 410, .dos_err = 317, .posix_err = 0, .msg = R"(A read or write of raw encrypted data cannot be performed because the file is not encrypted.)"},
	{.status = 411, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid file offset in the encrypted data info block was passed for read or write operation of file's raw encrypted data.)"},
	{.status = 412, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid offset and length combination in the encrypted data info block was passed for read or write operation of file's raw encrypted data.)"},
	{.status = 413, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid parameter in the encrypted data info block was passed for read or write operation of file's raw encrypted data.)"},
	{.status = 414, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows Subsystem for Linux has not been enabled.)"},
	{.status = 415, .dos_err = 317, .posix_err = 0, .msg = R"(The specified data could not be read from any of the copies.)"},
	{.status = 416, .dos_err = 317, .posix_err = 0, .msg = R"(The specified storage reserve ID is invalid.)"},
	{.status = 417, .dos_err = 317, .posix_err = 0, .msg = R"(The specified storage reserve does not exist.)"},
	{.status = 418, .dos_err = 317, .posix_err = 0, .msg = R"(The specified storage reserve already exists.)"},
	{.status = 419, .dos_err = 317, .posix_err = 0, .msg = R"(The specified storage reserve is not empty.)"},
	{.status = 420, .dos_err = 317, .posix_err = 0, .msg = R"(This operation requires a DAX volume.)"},
	{.status = 421, .dos_err = 317, .posix_err = 0, .msg = R"(This stream is not DAX mappable.)"},
	{.status = 422, .dos_err = 317, .posix_err = 0, .msg = R"(Operation cannot be performed on a time critical thread.)"},
	{.status = 423, .dos_err = 317, .posix_err = 0, .msg = R"(User data protection is not supported for the current or provided user.)"},
	{.status = 424, .dos_err = 317, .posix_err = 0, .msg = R"(This directory contains entries whose names differ only in case.)"},
	{.status = 425, .dos_err = 317, .posix_err = 0, .msg = R"(The file cannot be safely opened because it is not supported by this version of Windows.)"},
	{.status = 426, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud operation was not completed before the time-out period expired.)"},
	{.status = 427, .dos_err = 317, .posix_err = 0, .msg = R"(A task queue is required for this operation but none is available.)"},
	{.status = 428, .dos_err = 317, .posix_err = 0, .msg = R"(Failed loading a valid version of srcsrv.dll.)"},
	{.status = 429, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not supported with BTT enabled.)"},
	{.status = 430, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed because encryption is currently disabled.)"},
	{.status = 431, .dos_err = 317, .posix_err = 0, .msg = R"(This encryption operation cannot be performed on filesystem metadata.)"},
	{.status = 432, .dos_err = 317, .posix_err = 0, .msg = R"(Encryption cannot be cleared on this file/directory because it still has an encrypted attribute.)"},
	{.status = 433, .dos_err = 317, .posix_err = 0, .msg = R"(A device which does not exist was specified.)"},
	{.status = 434, .dos_err = 317, .posix_err = 0, .msg = R"(Dehydration of the cloud file is disallowed by the cloud sync provider.)"},
	{.status = 435, .dos_err = 317, .posix_err = 0, .msg = R"(A file snapshot operation was attempted when one is already in progress.)"},
	{.status = 436, .dos_err = 317, .posix_err = 0, .msg = R"(A snapshot of the file cannot be taken because a user-mapped section is present.)"},
	{.status = 437, .dos_err = 317, .posix_err = 0, .msg = R"(The file snapshot operation was terminated because one of the files was modified in a way incompatible with a snapshot operation.  Please try again.)"},
	{.status = 438, .dos_err = 317, .posix_err = 0, .msg = R"(An I/O request could not be coordinated with a file snapshot operation.)"},
	{.status = 439, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected error occurred while processing a file snapshot operation.)"},
	{.status = 440, .dos_err = 317, .posix_err = 0, .msg = R"(A file snapshot operation received an invalid parameter.)"},
	{.status = 441, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed due to one or more unsatisfied dependencies.)"},
	{.status = 442, .dos_err = 317, .posix_err = 0, .msg = R"(The file cannot be opened because the path has a case-sensitive directory.)"},
	{.status = 443, .dos_err = 317, .posix_err = 0, .msg = R"(The filesystem couldn't handle one of the CacheManager's callback error codes.)"},
	{.status = 444, .dos_err = 317, .posix_err = 0, .msg = R"(WSL 2 requires an update to its kernel component. For information please visit https://aka.ms/wsl2kernel)"},
	{.status = 445, .dos_err = 317, .posix_err = 0, .msg = R"(This action is blocked, but you can choose to allow it. Please refer to the data loss prevention notification for further information.)"},
	{.status = 446, .dos_err = 317, .posix_err = 0, .msg = R"(This action is blocked. Please refer to the data loss prevention notification for further information.)"},
	{.status = 449, .dos_err = 317, .posix_err = 0, .msg = R"(This action is blocked. Please refer to the data loss prevention notification for further information.)"},
	{.status = 450, .dos_err = 317, .posix_err = 0, .msg = R"(Neither developer unlocked mode nor side loading mode is enabled on the device.)"},
	{.status = 451, .dos_err = 317, .posix_err = 0, .msg = R"(Can not change application type during upgrade or re-provision.)"},
	{.status = 452, .dos_err = 317, .posix_err = 0, .msg = R"(The application has not been provisioned.)"},
	{.status = 453, .dos_err = 317, .posix_err = 0, .msg = R"(The requested capability can not be authorized for this application.)"},
	{.status = 454, .dos_err = 317, .posix_err = 0, .msg = R"(There is no capability authorization policy on the device.)"},
	{.status = 455, .dos_err = 317, .posix_err = 0, .msg = R"(The capability authorization database has been corrupted.)"},
	{.status = 456, .dos_err = 317, .posix_err = 0, .msg = R"(The custom capability's SCCD has an invalid catalog.)"},
	{.status = 457, .dos_err = 317, .posix_err = 0, .msg = R"(None of the authorized entity elements in the SCCD matched the app being installed; either the PFNs don't match, or the element's signature hash doesn't validate.)"},
	{.status = 458, .dos_err = 317, .posix_err = 0, .msg = R"(The custom capability's SCCD failed to parse.)"},
	{.status = 459, .dos_err = 317, .posix_err = 0, .msg = R"(The custom capability's SCCD requires developer mode.)"},
	{.status = 460, .dos_err = 317, .posix_err = 0, .msg = R"(There not all declared custom capabilities are found in the SCCD.)"},
	{.status = 470, .dos_err = 317, .posix_err = 0, .msg = R"(The CimFS image is corrupt.)"},
	{.status = 475, .dos_err = 317, .posix_err = 0, .msg = R"(The cloud provider failed to acknowledge a message before the time-out expired.)"},
	{.status = 480, .dos_err = 317, .posix_err = 0, .msg = R"(The operation timed out waiting for this device to complete a PnP query-remove request due to a potential hang in its device stack. The system may need to be rebooted to complete the request.)"},
	{.status = 481, .dos_err = 317, .posix_err = 0, .msg = R"(The operation timed out waiting for this device to complete a PnP query-remove request due to a potential hang in the device stack of a related device. The system may need to be rebooted to complete the operation.)"},
	{.status = 482, .dos_err = 317, .posix_err = 0, .msg = R"(The operation timed out waiting for this device to complete a PnP query-remove request due to a potential hang in the device stack of an unrelated device. The system may need to be rebooted to complete the operation.)"},
	{.status = 483, .dos_err = 317, .posix_err = 0, .msg = R"(The request failed due to a fatal device hardware error.)"},
	{.status = 487, .dos_err = 317, .posix_err = 0, .msg = R"(Attempt to access invalid address.)"},
	{.status = 488, .dos_err = 317, .posix_err = 0, .msg = R"(The volume contains paging, crash dump or other system critical files.)"},
	{.status = 500, .dos_err = 317, .posix_err = 0, .msg = R"(User profile cannot be loaded.)"},
	{.status = 510, .dos_err = 317, .posix_err = 0, .msg = R"(The file system encountered a metadata file with inconsistent data.)"},
	{.status = 514, .dos_err = 6707, .posix_err = 0, .msg = R"(The specified ResourceManager made no changes or updates to the resource under this transaction.)"},
	{.status = 528, .dos_err = 0, .posix_err = 0, .msg = R"(The specified ring buffer was empty before the packet was successfully inserted.)"},
	{.status = 529, .dos_err = 0, .posix_err = 0, .msg = R"(The specified ring buffer was full before the packet was successfully removed.)"},
	{.status = 530, .dos_err = 0, .posix_err = 0, .msg = R"(The specified ring buffer has dropped below its quota of outstanding transactions.)"},
	{.status = 531, .dos_err = 0, .posix_err = 0, .msg = R"(The specified ring buffer has, with the removal of the current packet, now become empty.)"},
	{.status = 532, .dos_err = 0, .posix_err = 0, .msg = R"(The specified ring buffer was either previously empty or previously full which implies that the caller should signal the opposite endpoint.)"},
	{.status = 533, .dos_err = 800, .posix_err = 0, .msg = R"(The oplock that was associated with this handle is now associated with a different handle.)"},
	{.status = 534, .dos_err = 803, .posix_err = 0, .msg = R"(The handle with which this oplock was associated has been closed.  The oplock is now broken.)"},
	{.status = 535, .dos_err = 317, .posix_err = 0, .msg = R"(There is a process on other end of the pipe.)"},
	{.status = 536, .dos_err = 317, .posix_err = 0, .msg = R"(Waiting for a process to open the other end of the pipe.)"},
	{.status = 537, .dos_err = 317, .posix_err = 0, .msg = R"(Application verifier has found an error in the current process.)"},
	{.status = 538, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred in the ABIOS subsystem.)"},
	{.status = 539, .dos_err = 317, .posix_err = 0, .msg = R"(A warning occurred in the WX86 subsystem.)"},
	{.status = 540, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred in the WX86 subsystem.)"},
	{.status = 541, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to cancel or set a timer that has an associated APC and the subject thread is not the thread that originally set the timer with an associated APC routine.)"},
	{.status = 542, .dos_err = 317, .posix_err = 0, .msg = R"(Unwind exception code.)"},
	{.status = 543, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid or unaligned stack was encountered during an unwind operation.)"},
	{.status = 544, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid unwind target was encountered during an unwind operation.)"},
	{.status = 545, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid Object Attributes specified to NtCreatePort or invalid Port Attributes specified to NtConnectPort)"},
	{.status = 546, .dos_err = 317, .posix_err = 0, .msg = R"(Length of message passed to NtRequestPort or NtRequestWaitReplyPort was longer than the maximum message allowed by the port.)"},
	{.status = 547, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to lower a quota limit below the current usage.)"},
	{.status = 548, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to attach to a device that was already attached to another device.)"},
	{.status = 549, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to execute an instruction at an unaligned address and the host system does not support unaligned instruction references.)"},
	{.status = 550, .dos_err = 317, .posix_err = 0, .msg = R"(Profiling not started.)"},
	{.status = 551, .dos_err = 317, .posix_err = 0, .msg = R"(Profiling not stopped.)"},
	{.status = 552, .dos_err = 317, .posix_err = 0, .msg = R"(The passed ACL did not contain the minimum required information.)"},
	{.status = 553, .dos_err = 317, .posix_err = 0, .msg = R"(The number of active profiling objects is at the maximum and no more may be started.)"},
	{.status = 554, .dos_err = 317, .posix_err = 0, .msg = R"(Used to indicate that an operation cannot continue without blocking for I/O.)"},
	{.status = 555, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates that a thread attempted to terminate itself by default (called NtTerminateThread with NULL) and it was the last thread in the current process.)"},
	{.status = 556, .dos_err = 317, .posix_err = 0, .msg = R"(If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter.

In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 557, .dos_err = 317, .posix_err = 0, .msg = R"(If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter.

In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 558, .dos_err = 317, .posix_err = 0, .msg = R"(If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter.

In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 559, .dos_err = 317, .posix_err = 0, .msg = R"(A malformed function table was encountered during an unwind operation.)"},
	{.status = 560, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates that an attempt was made to assign protection to a file system file or directory and one of the SIDs in the security descriptor could not be translated into a GUID that could be stored by the file system.

This causes the protection attempt to fail, which may cause a file creation attempt to fail.)"},
	{.status = 561, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates that an attempt was made to grow an LDT by setting its size, or that the size was not an even number of selectors.)"},
	{.status = 563, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates that the starting value for the LDT information was not an integral multiple of the selector size.)"},
	{.status = 564, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates that the user supplied an invalid descriptor when trying to set up Ldt descriptors.)"},
	{.status = 565, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates a process has too many threads to perform the requested action. For example, assignment of a primary token may only be performed when a process has zero or one threads.)"},
	{.status = 566, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to operate on a thread within a specific process, but the thread specified is not in the process specified.)"},
	{.status = 567, .dos_err = 317, .posix_err = 0, .msg = R"(Page file quota was exceeded.)"},
	{.status = 568, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service cannot start because another Netlogon service running in the domain conflicts with the specified role.)"},
	{.status = 569, .dos_err = 317, .posix_err = 0, .msg = R"(The SAM database on a Windows Server is significantly out of synchronization with the copy on the Domain Controller. A complete synchronization is required.)"},
	{.status = 570, .dos_err = 317, .posix_err = 0, .msg = R"(The NtCreateFile API failed. This error should never be returned to an application, it is a place holder for the Windows Lan Manager Redirector to use in its internal error mapping routines.)"},
	{.status = 571, .dos_err = 317, .posix_err = 0, .msg = R"({Privilege Failed}

The I/O permissions for the process could not be changed.)"},
	{.status = 572, .dos_err = 317, .posix_err = 0, .msg = R"({Application Exit by CTRL+C}

The application terminated as a result of a CTRL+C.)"},
	{.status = 573, .dos_err = 317, .posix_err = 0, .msg = R"({Missing System File}

The required system file %hs is bad or missing.)"},
	{.status = 574, .dos_err = 317, .posix_err = 0, .msg = R"({Application Error}

The exception %s (0x)"},
	{.status = 575, .dos_err = 317, .posix_err = 0, .msg = R"({Application Error}

The application was unable to start correctly (0x%lx). Click OK to close the application.)"},
	{.status = 576, .dos_err = 317, .posix_err = 0, .msg = R"({Unable to Create Paging File}

The creation of the paging file %hs failed (%lx). The requested size was %ld.)"},
	{.status = 577, .dos_err = 317, .posix_err = 0, .msg = R"(Windows cannot verify the digital signature for this file. A recent hardware or software change might have installed a file that is signed incorrectly or damaged, or that might be malicious software from an unknown source.)"},
	{.status = 578, .dos_err = 317, .posix_err = 0, .msg = R"({No Paging File Specified}

No paging file was specified in the system configuration.)"},
	{.status = 579, .dos_err = 317, .posix_err = 0, .msg = R"({EXCEPTION}

A real-mode application issued a floating-point instruction and floating-point hardware is not present.)"},
	{.status = 580, .dos_err = 317, .posix_err = 0, .msg = R"(An event pair synchronization operation was performed using the thread specific client/server event pair object, but no event pair object was associated with the thread.)"},
	{.status = 581, .dos_err = 317, .posix_err = 0, .msg = R"(A Windows Server has an incorrect configuration.)"},
	{.status = 582, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal character was encountered. For a multi-byte character set this includes a lead byte without a succeeding trail byte. For the Unicode character set this includes the characters 0xFFFF and 0xFFFE.)"},
	{.status = 583, .dos_err = 317, .posix_err = 0, .msg = R"(The Unicode character is not defined in the Unicode character set installed on the system.)"},
	{.status = 584, .dos_err = 317, .posix_err = 0, .msg = R"(The paging file cannot be created on a floppy diskette.)"},
	{.status = 585, .dos_err = 317, .posix_err = 0, .msg = R"(The system BIOS failed to connect a system interrupt to the device or bus for which the device is connected.)"},
	{.status = 586, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is only allowed for the Primary Domain Controller of the domain.)"},
	{.status = 587, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to acquire a mutant such that its maximum count would have been exceeded.)"},
	{.status = 588, .dos_err = 317, .posix_err = 0, .msg = R"(A volume has been accessed for which a file system driver is required that has not yet been loaded.)"},
	{.status = 589, .dos_err = 317, .posix_err = 0, .msg = R"({Registry File Failure}

The registry cannot load the hive (file):

%hs

or its log or alternate.

It is corrupt, absent, or not writable.)"},
	{.status = 590, .dos_err = 317, .posix_err = 0, .msg = R"({Unexpected Failure in DebugActiveProcess}

An unexpected failure occurred while processing a DebugActiveProcess API request. You may choose OK to terminate the process, or Cancel to ignore the error.)"},
	{.status = 591, .dos_err = 317, .posix_err = 0, .msg = R"({Fatal System Error}

The %hs system process terminated unexpectedly with a status of 0x)"},
	{.status = 592, .dos_err = 317, .posix_err = 0, .msg = R"({Data Not Accepted}

The TDI client could not handle the data received during an indication.)"},
	{.status = 593, .dos_err = 317, .posix_err = 0, .msg = R"(NTVDM encountered a hard error.)"},
	{.status = 594, .dos_err = 317, .posix_err = 0, .msg = R"({Cancel Timeout}

The driver %hs failed to complete a cancelled I/O request in the allotted time.)"},
	{.status = 595, .dos_err = 317, .posix_err = 0, .msg = R"({Reply Message Mismatch}

An attempt was made to reply to an LPC message, but the thread specified by the client ID in the message was not waiting on that message.)"},
	{.status = 596, .dos_err = 317, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs. The data has been lost.

This error may be caused by a failure of your computer hardware or network connection. Please try to save this file elsewhere.)"},
	{.status = 597, .dos_err = 317, .posix_err = 0, .msg = R"(The parameter(s) passed to the server in the client/server shared memory window were invalid. Too much data may have been put in the shared memory window.)"},
	{.status = 598, .dos_err = 317, .posix_err = 0, .msg = R"(The stream is not a tiny stream.)"},
	{.status = 599, .dos_err = 317, .posix_err = 0, .msg = R"(The request must be handled by the stack overflow code.)"},
	{.status = 600, .dos_err = 317, .posix_err = 0, .msg = R"(Internal OFS status codes indicating how an allocation operation is handled. Either it is retried after the containing onode is moved or the extent stream is converted to a large stream.)"},
	{.status = 601, .dos_err = 317, .posix_err = 0, .msg = R"(The attempt to find the object found an object matching by ID on the volume but it is out of the scope of the handle used for the operation.)"},
	{.status = 602, .dos_err = 317, .posix_err = 0, .msg = R"(The bucket array must be grown. Retry transaction after doing so.)"},
	{.status = 603, .dos_err = 317, .posix_err = 0, .msg = R"(The user/kernel marshalling buffer has overflowed.)"},
	{.status = 604, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied variant structure contains invalid data.)"},
	{.status = 605, .dos_err = 317, .posix_err = 0, .msg = R"(The specified buffer contains ill-formed data.)"},
	{.status = 606, .dos_err = 317, .posix_err = 0, .msg = R"({Audit Failed}

An attempt to generate a security audit failed.)"},
	{.status = 607, .dos_err = 317, .posix_err = 0, .msg = R"(The timer resolution was not previously set by the current process.)"},
	{.status = 608, .dos_err = 317, .posix_err = 0, .msg = R"(There is insufficient account information to log you on.)"},
	{.status = 609, .dos_err = 317, .posix_err = 0, .msg = R"({Invalid DLL Entrypoint}

The dynamic link library %hs is not written correctly. The stack pointer has been left in an inconsistent state.

The entrypoint should be declared as WINAPI or STDCALL. Select YES to fail the DLL load. Select NO to continue execution. Selecting NO may cause the application to operate incorrectly.)"},
	{.status = 610, .dos_err = 317, .posix_err = 0, .msg = R"({Invalid Service Callback Entrypoint}

The %hs service is not written correctly. The stack pointer has been left in an inconsistent state.

The callback entrypoint should be declared as WINAPI or STDCALL. Selecting OK will cause the service to continue operation. However, the service process may operate incorrectly.)"},
	{.status = 611, .dos_err = 317, .posix_err = 0, .msg = R"(There is an IP address conflict with another system on the network)"},
	{.status = 612, .dos_err = 317, .posix_err = 0, .msg = R"(There is an IP address conflict with another system on the network)"},
	{.status = 613, .dos_err = 317, .posix_err = 0, .msg = R"({Low On Registry Space}

The system has reached the maximum size allowed for the system part of the registry. Additional storage requests will be ignored.)"},
	{.status = 614, .dos_err = 317, .posix_err = 0, .msg = R"(A callback return system service cannot be executed when no callback is active.)"},
	{.status = 615, .dos_err = 317, .posix_err = 0, .msg = R"(The password provided is too short to meet the policy of your user account.

Please choose a longer password.)"},
	{.status = 616, .dos_err = 317, .posix_err = 0, .msg = R"(The policy of your user account does not allow you to change passwords too frequently.

This is done to prevent users from changing back to a familiar, but potentially discovered, password.

If you feel your password has been compromised then please contact your administrator immediately to have a new one assigned.)"},
	{.status = 617, .dos_err = 317, .posix_err = 0, .msg = R"(You have attempted to change your password to one that you have used in the past.

The policy of your user account does not allow this. Please select a password that you have not previously used.)"},
	{.status = 618, .dos_err = 317, .posix_err = 0, .msg = R"(The specified compression format is unsupported.)"},
	{.status = 619, .dos_err = 317, .posix_err = 0, .msg = R"(The specified hardware profile configuration is invalid.)"},
	{.status = 620, .dos_err = 317, .posix_err = 0, .msg = R"(The specified Plug and Play registry device path is invalid.)"},
	{.status = 621, .dos_err = 317, .posix_err = 0, .msg = R"(The specified quota list is internally inconsistent with its descriptor.)"},
	{.status = 622, .dos_err = 317, .posix_err = 0, .msg = R"({Windows Evaluation Notification}

The evaluation period for this installation of Windows has expired. This system will shutdown in 1 hour. To restore access to this installation of Windows, please upgrade this installation using a licensed distribution of this product.)"},
	{.status = 623, .dos_err = 317, .posix_err = 0, .msg = R"({Illegal System DLL Relocation}

The system DLL %hs was relocated in memory. The application will not run properly.

The relocation occurred because the DLL %hs occupied an address range reserved for Windows system DLLs. The vendor supplying the DLL should be contacted for a new DLL.)"},
	{.status = 624, .dos_err = 317, .posix_err = 0, .msg = R"({DLL Initialization Failed}

The application failed to initialize because the window station is shutting down.)"},
	{.status = 625, .dos_err = 317, .posix_err = 0, .msg = R"(The validation process needs to continue on to the next step.)"},
	{.status = 626, .dos_err = 317, .posix_err = 0, .msg = R"(There are no more matches for the current index enumeration.)"},
	{.status = 627, .dos_err = 317, .posix_err = 0, .msg = R"(The range could not be added to the range list because of a conflict.)"},
	{.status = 628, .dos_err = 317, .posix_err = 0, .msg = R"(The server process is running under a SID different than that required by client.)"},
	{.status = 629, .dos_err = 317, .posix_err = 0, .msg = R"(A group marked use for deny only cannot be enabled.)"},
	{.status = 630, .dos_err = 317, .posix_err = 0, .msg = R"({EXCEPTION}

Multiple floating point faults.)"},
	{.status = 631, .dos_err = 317, .posix_err = 0, .msg = R"({EXCEPTION}

Multiple floating point traps.)"},
	{.status = 632, .dos_err = 317, .posix_err = 0, .msg = R"(The requested interface is not supported.)"},
	{.status = 633, .dos_err = 317, .posix_err = 0, .msg = R"({System Standby Failed}

The driver %hs does not support standby mode. Updating this driver may allow the system to go to standby mode.)"},
	{.status = 634, .dos_err = 317, .posix_err = 0, .msg = R"(The system file %1 has become corrupt and has been replaced.)"},
	{.status = 635, .dos_err = 317, .posix_err = 0, .msg = R"({Virtual Memory Minimum Too Low}

Your system is low on virtual memory. Windows is increasing the size of your virtual memory paging file.

During this process, memory requests for some applications may be denied. For more information, see Help.)"},
	{.status = 636, .dos_err = 317, .posix_err = 0, .msg = R"(A device was removed so enumeration must be restarted.)"},
	{.status = 637, .dos_err = 317, .posix_err = 0, .msg = R"({Fatal System Error}

The system image %s is not properly signed.

The file has been replaced with the signed file.

The system has been shut down.)"},
	{.status = 638, .dos_err = 317, .posix_err = 0, .msg = R"(Device will not start without a reboot.)"},
	{.status = 639, .dos_err = 317, .posix_err = 0, .msg = R"(There is not enough power to complete the requested operation.)"},
	{.status = 640, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_MULTIPLE_FAULT_VIOLATION)"},
	{.status = 641, .dos_err = 317, .posix_err = 0, .msg = R"(The system is in the process of shutting down.)"},
	{.status = 642, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to remove a processes DebugPort was made, but a port was not already associated with the process.)"},
	{.status = 643, .dos_err = 317, .posix_err = 0, .msg = R"(This version of Windows is not compatible with the behavior version of directory forest, domain or domain controller.)"},
	{.status = 644, .dos_err = 317, .posix_err = 0, .msg = R"(The specified range could not be found in the range list.)"},
	{.status = 646, .dos_err = 317, .posix_err = 0, .msg = R"(The driver was not loaded because the system is booting into safe mode.)"},
	{.status = 647, .dos_err = 317, .posix_err = 0, .msg = R"(The driver was not loaded because it failed its initialization call.)"},
	{.status = 648, .dos_err = 317, .posix_err = 0, .msg = R"(The "%hs" encountered an error while applying power or reading the device configuration.

This may be caused by a failure of your hardware or by a poor connection.)"},
	{.status = 649, .dos_err = 317, .posix_err = 0, .msg = R"(The create operation failed because the name contained at least one mount point which resolves to a volume to which the specified device object is not attached.)"},
	{.status = 650, .dos_err = 317, .posix_err = 0, .msg = R"(The device object parameter is either not a valid device object or is not attached to the volume specified by the file name.)"},
	{.status = 651, .dos_err = 317, .posix_err = 0, .msg = R"(A Machine Check Error has occurred. Please check the system eventlog for additional information.)"},
	{.status = 652, .dos_err = 317, .posix_err = 0, .msg = R"(There was error [%2] processing the driver database.)"},
	{.status = 653, .dos_err = 317, .posix_err = 0, .msg = R"(System hive size has exceeded its limit.)"},
	{.status = 654, .dos_err = 317, .posix_err = 0, .msg = R"(The driver could not be loaded because a previous version of the driver is still in memory.)"},
	{.status = 655, .dos_err = 317, .posix_err = 0, .msg = R"({Volume Shadow Copy Service}

Please wait while the Volume Shadow Copy Service prepares volume %hs for hibernation.)"},
	{.status = 656, .dos_err = 317, .posix_err = 0, .msg = R"(The system has failed to hibernate (The error code is %hs). Hibernation will be disabled until the system is restarted.)"},
	{.status = 657, .dos_err = 317, .posix_err = 0, .msg = R"(The password provided is too long to meet the policy of your user account.

Please choose a shorter password.)"},
	{.status = 665, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation could not be completed due to a file system limitation)"},
	{.status = 668, .dos_err = 317, .posix_err = 0, .msg = R"(An assertion failure has occurred.)"},
	{.status = 669, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred in the ACPI subsystem.)"},
	{.status = 670, .dos_err = 317, .posix_err = 0, .msg = R"(WOW Assertion Error.)"},
	{.status = 671, .dos_err = 317, .posix_err = 0, .msg = R"(A device is missing in the system BIOS MPS table. This device will not be used.

Please contact your system vendor for system BIOS update.)"},
	{.status = 672, .dos_err = 317, .posix_err = 0, .msg = R"(A translator failed to translate resources.)"},
	{.status = 673, .dos_err = 317, .posix_err = 0, .msg = R"(A IRQ translator failed to translate resources.)"},
	{.status = 674, .dos_err = 317, .posix_err = 0, .msg = R"(Driver %2 returned invalid ID for a child device (%3).)"},
	{.status = 675, .dos_err = 317, .posix_err = 0, .msg = R"({Kernel Debugger Awakened}

the system debugger was awakened by an interrupt.)"},
	{.status = 676, .dos_err = 317, .posix_err = 0, .msg = R"({Handles Closed}

Handles to objects have been automatically closed as a result of the requested operation.)"},
	{.status = 677, .dos_err = 317, .posix_err = 0, .msg = R"({Too Much Information}

The specified access control list (ACL) contained more information than was expected.)"},
	{.status = 678, .dos_err = 317, .posix_err = 0, .msg = R"(This warning level status indicates that the transaction state already exists for the registry sub-tree, but that a transaction commit was previously aborted.

The commit has NOT been completed, but has not been rolled back either (so it may still be committed if desired).)"},
	{.status = 679, .dos_err = 317, .posix_err = 0, .msg = R"({Media Changed}

The media may have changed.)"},
	{.status = 680, .dos_err = 317, .posix_err = 0, .msg = R"({GUID Substitution}

During the translation of a global identifier (GUID) to a Windows security ID (SID), no administratively-defined GUID prefix was found.

A substitute prefix was used, which will not compromise system security. However, this may provide a more restrictive access than intended.)"},
	{.status = 681, .dos_err = 317, .posix_err = 0, .msg = R"(The create operation stopped after reaching a symbolic link)"},
	{.status = 682, .dos_err = 317, .posix_err = 0, .msg = R"(A long jump has been executed.)"},
	{.status = 683, .dos_err = 317, .posix_err = 0, .msg = R"(The Plug and Play query operation was not successful.)"},
	{.status = 684, .dos_err = 317, .posix_err = 0, .msg = R"(A frame consolidation has been executed.)"},
	{.status = 685, .dos_err = 317, .posix_err = 0, .msg = R"({Registry Hive Recovered}

Registry hive (file):

%hs

was corrupted and it has been recovered. Some data might have been lost.)"},
	{.status = 686, .dos_err = 317, .posix_err = 0, .msg = R"(The application is attempting to run executable code from the module %hs. This may be insecure. An alternative, %hs, is available. Should the application use the secure module %hs?)"},
	{.status = 687, .dos_err = 317, .posix_err = 0, .msg = R"(The application is loading executable code from the module %hs. This is secure, but may be incompatible with previous releases of the operating system. An alternative, %hs, is available. Should the application use the secure module %hs?)"},
	{.status = 688, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger did not handle the exception.)"},
	{.status = 689, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger will reply later.)"},
	{.status = 690, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger cannot provide handle.)"},
	{.status = 691, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger terminated thread.)"},
	{.status = 692, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger terminated process.)"},
	{.status = 693, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger got control C.)"},
	{.status = 694, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger printed exception on control C.)"},
	{.status = 695, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger received RIP exception.)"},
	{.status = 696, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger received control break.)"},
	{.status = 697, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger command communication exception.)"},
	{.status = 698, .dos_err = 317, .posix_err = 0, .msg = R"({Object Exists}

An attempt was made to create an object and the object name already existed.)"},
	{.status = 699, .dos_err = 317, .posix_err = 0, .msg = R"({Thread Suspended}

A thread termination occurred while the thread was suspended. The thread was resumed, and termination proceeded.)"},
	{.status = 700, .dos_err = 317, .posix_err = 0, .msg = R"({Image Relocated}

An image file could not be mapped at the address specified in the image file. Local fixups must be performed on this image.)"},
	{.status = 701, .dos_err = 317, .posix_err = 0, .msg = R"(This informational level status indicates that a specified registry sub-tree transaction state did not yet exist and had to be created.)"},
	{.status = 702, .dos_err = 317, .posix_err = 0, .msg = R"({Segment Load}

A virtual DOS machine (VDM) is loading, unloading, or moving an MS-DOS or Win16 program segment image.

An exception is raised so a debugger can load, unload or track symbols and breakpoints within these 16-bit segments.)"},
	{.status = 703, .dos_err = 317, .posix_err = 0, .msg = R"({Invalid Current Directory}

The process cannot switch to the startup current directory %hs.

Select OK to set current directory to %hs, or select CANCEL to exit.)"},
	{.status = 704, .dos_err = 317, .posix_err = 0, .msg = R"({Redundant Read}

To satisfy a read request, the NT fault-tolerant file system successfully read the requested data from a redundant copy.

This was done because the file system encountered a failure on a member of the fault-tolerant volume, but was unable to reassign the failing area of the device.)"},
	{.status = 705, .dos_err = 317, .posix_err = 0, .msg = R"({Redundant Write}

To satisfy a write request, the NT fault-tolerant file system successfully wrote a redundant copy of the information.

This was done because the file system encountered a failure on a member of the fault-tolerant volume, but was not able to reassign the failing area of the device.)"},
	{.status = 706, .dos_err = 317, .posix_err = 0, .msg = R"({Machine Type Mismatch}

The image file %hs is valid, but is for a machine type other than the current machine. Select OK to continue, or CANCEL to fail the DLL load.)"},
	{.status = 707, .dos_err = 317, .posix_err = 0, .msg = R"({Partial Data Received}

The network transport returned partial data to its client. The remaining data will be sent later.)"},
	{.status = 708, .dos_err = 317, .posix_err = 0, .msg = R"({Expedited Data Received}

The network transport returned data to its client that was marked as expedited by the remote system.)"},
	{.status = 709, .dos_err = 317, .posix_err = 0, .msg = R"({Partial Expedited Data Received}

The network transport returned partial data to its client and this data was marked as expedited by the remote system. The remaining data will be sent later.)"},
	{.status = 710, .dos_err = 317, .posix_err = 0, .msg = R"({TDI Event Done}

The TDI indication has completed successfully.)"},
	{.status = 711, .dos_err = 317, .posix_err = 0, .msg = R"({TDI Event Pending}

The TDI indication has entered the pending state.)"},
	{.status = 712, .dos_err = 317, .posix_err = 0, .msg = R"(Checking file system on %wZ)"},
	{.status = 713, .dos_err = 317, .posix_err = 0, .msg = R"({Fatal Application Exit}

%hs)"},
	{.status = 714, .dos_err = 317, .posix_err = 0, .msg = R"(The specified registry key is referenced by a predefined handle.)"},
	{.status = 715, .dos_err = 317, .posix_err = 0, .msg = R"({Page Unlocked}

The page protection of a locked page was changed to 'No Access' and the page was unlocked from memory and from the process.)"},
	{.status = 716, .dos_err = 317, .posix_err = 0, .msg = R"(%hs)"},
	{.status = 717, .dos_err = 317, .posix_err = 0, .msg = R"({Page Locked}

One of the pages to lock was already locked.)"},
	{.status = 718, .dos_err = 317, .posix_err = 0, .msg = R"(Application popup: %1 : %2)"},
	{.status = 719, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_ALREADY_WIN32)"},
	{.status = 720, .dos_err = 317, .posix_err = 0, .msg = R"({Machine Type Mismatch}

The image file %hs is valid, but is for a machine type other than the current machine.)"},
	{.status = 721, .dos_err = 317, .posix_err = 0, .msg = R"(A yield execution was performed and no thread was available to run.)"},
	{.status = 722, .dos_err = 317, .posix_err = 0, .msg = R"(The resumable flag to a timer API was ignored.)"},
	{.status = 723, .dos_err = 317, .posix_err = 0, .msg = R"(The arbiter has deferred arbitration of these resources to its parent)"},
	{.status = 724, .dos_err = 317, .posix_err = 0, .msg = R"(The inserted CardBus device cannot be started because of a configuration error on "%hs".)"},
	{.status = 725, .dos_err = 317, .posix_err = 0, .msg = R"(The CPUs in this multiprocessor system are not all the same revision level. To use all processors the operating system restricts itself to the features of the least capable processor in the system. Should problems occur with this system, contact the CPU manufacturer to see if this mix of processors is supported.)"},
	{.status = 726, .dos_err = 317, .posix_err = 0, .msg = R"(The system was put into hibernation.)"},
	{.status = 727, .dos_err = 317, .posix_err = 0, .msg = R"(The system was resumed from hibernation.)"},
	{.status = 728, .dos_err = 317, .posix_err = 0, .msg = R"(Windows has detected that the system firmware (BIOS) was updated [previous firmware date = %2, current firmware date %3].)"},
	{.status = 729, .dos_err = 317, .posix_err = 0, .msg = R"(A device driver is leaking locked I/O pages causing system degradation. The system has automatically enabled tracking code in order to try and catch the culprit.)"},
	{.status = 730, .dos_err = 317, .posix_err = 0, .msg = R"(The system has awoken)"},
	{.status = 731, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_WAIT_1)"},
	{.status = 732, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_WAIT_2)"},
	{.status = 733, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_WAIT_3)"},
	{.status = 734, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_WAIT_63)"},
	{.status = 735, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_ABANDONED_WAIT_0)"},
	{.status = 736, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_ABANDONED_WAIT_63)"},
	{.status = 737, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_USER_APC)"},
	{.status = 738, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_KERNEL_APC)"},
	{.status = 739, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_ALERTED)"},
	{.status = 740, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation requires elevation.)"},
	{.status = 741, .dos_err = 317, .posix_err = 0, .msg = R"(A reparse should be performed by the Object Manager since the name of the file resulted in a symbolic link.)"},
	{.status = 742, .dos_err = 317, .posix_err = 0, .msg = R"(An open/create operation completed while an oplock break is underway.)"},
	{.status = 743, .dos_err = 317, .posix_err = 0, .msg = R"(A new volume has been mounted by a file system.)"},
	{.status = 744, .dos_err = 317, .posix_err = 0, .msg = R"(This success level status indicates that the transaction state already exists for the registry sub-tree, but that a transaction commit was previously aborted.

The commit has now been completed.)"},
	{.status = 745, .dos_err = 317, .posix_err = 0, .msg = R"(This indicates that a notify change request has been completed due to closing the handle which made the notify change request.)"},
	{.status = 746, .dos_err = 317, .posix_err = 0, .msg = R"({Connect Failure on Primary Transport}

An attempt was made to connect to the remote server %hs on the primary transport, but the connection failed.

The computer WAS able to connect on a secondary transport.)"},
	{.status = 747, .dos_err = 317, .posix_err = 0, .msg = R"(Page fault was a transition fault.)"},
	{.status = 748, .dos_err = 317, .posix_err = 0, .msg = R"(Page fault was a demand zero fault.)"},
	{.status = 749, .dos_err = 317, .posix_err = 0, .msg = R"(Page fault was a demand zero fault.)"},
	{.status = 750, .dos_err = 317, .posix_err = 0, .msg = R"(Page fault was a demand zero fault.)"},
	{.status = 751, .dos_err = 317, .posix_err = 0, .msg = R"(Page fault was satisfied by reading from a secondary storage device.)"},
	{.status = 752, .dos_err = 317, .posix_err = 0, .msg = R"(Cached page was locked during operation.)"},
	{.status = 753, .dos_err = 317, .posix_err = 0, .msg = R"(Crash dump exists in paging file.)"},
	{.status = 754, .dos_err = 317, .posix_err = 0, .msg = R"(Specified buffer contains all zeros.)"},
	{.status = 755, .dos_err = 317, .posix_err = 0, .msg = R"(A reparse should be performed by the Object Manager since the name of the file resulted in a symbolic link.)"},
	{.status = 756, .dos_err = 317, .posix_err = 0, .msg = R"(The device has succeeded a query-stop and its resource requirements have changed.)"},
	{.status = 757, .dos_err = 317, .posix_err = 0, .msg = R"(The translator has translated these resources into the global space and no further translations should be performed.)"},
	{.status = 758, .dos_err = 317, .posix_err = 0, .msg = R"(A process being terminated has no threads to terminate.)"},
	{.status = 759, .dos_err = 317, .posix_err = 0, .msg = R"(The specified process is not part of a job.)"},
	{.status = 760, .dos_err = 317, .posix_err = 0, .msg = R"(The specified process is part of a job.)"},
	{.status = 761, .dos_err = 317, .posix_err = 0, .msg = R"({Volume Shadow Copy Service}

The system is now ready for hibernation.)"},
	{.status = 762, .dos_err = 317, .posix_err = 0, .msg = R"(A file system or file system filter driver has successfully completed an FsFilter operation.)"},
	{.status = 763, .dos_err = 317, .posix_err = 0, .msg = R"(The specified interrupt vector was already connected.)"},
	{.status = 764, .dos_err = 317, .posix_err = 0, .msg = R"(The specified interrupt vector is still connected.)"},
	{.status = 765, .dos_err = 317, .posix_err = 0, .msg = R"(An operation is blocked waiting for an oplock.)"},
	{.status = 766, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger handled exception)"},
	{.status = 767, .dos_err = 317, .posix_err = 0, .msg = R"(Debugger continued)"},
	{.status = 768, .dos_err = 317, .posix_err = 0, .msg = R"(An exception occurred in a user mode callback and the kernel callback frame should be removed.)"},
	{.status = 769, .dos_err = 317, .posix_err = 0, .msg = R"(Compression is disabled for this volume.)"},
	{.status = 770, .dos_err = 317, .posix_err = 0, .msg = R"(The data provider cannot fetch backwards through a result set.)"},
	{.status = 771, .dos_err = 317, .posix_err = 0, .msg = R"(The data provider cannot scroll backwards through a result set.)"},
	{.status = 772, .dos_err = 317, .posix_err = 0, .msg = R"(The data provider requires that previously fetched data is released before asking for more data.)"},
	{.status = 773, .dos_err = 317, .posix_err = 0, .msg = R"(The data provider was not able to interpret the flags set for a column binding in an accessor.)"},
	{.status = 774, .dos_err = 317, .posix_err = 0, .msg = R"(One or more errors occurred while processing the request.)"},
	{.status = 775, .dos_err = 317, .posix_err = 0, .msg = R"(The implementation is not capable of performing the request.)"},
	{.status = 776, .dos_err = 317, .posix_err = 0, .msg = R"(The client of a component requested an operation which is not valid given the state of the component instance.)"},
	{.status = 777, .dos_err = 317, .posix_err = 0, .msg = R"(A version number could not be parsed.)"},
	{.status = 778, .dos_err = 317, .posix_err = 0, .msg = R"(The iterator's start position is invalid.)"},
	{.status = 779, .dos_err = 317, .posix_err = 0, .msg = R"(The hardware has reported an uncorrectable memory error.)"},
	{.status = 780, .dos_err = 317, .posix_err = 0, .msg = R"(The attempted operation required self healing to be enabled.)"},
	{.status = 781, .dos_err = 317, .posix_err = 0, .msg = R"(The Desktop heap encountered an error while allocating session memory. There is more information in the system event log.)"},
	{.status = 782, .dos_err = 317, .posix_err = 0, .msg = R"(The system power state is transitioning from %2 to %3.)"},
	{.status = 783, .dos_err = 317, .posix_err = 0, .msg = R"(The system power state is transitioning from %2 to %3 but could enter %4.)"},
	{.status = 784, .dos_err = 317, .posix_err = 0, .msg = R"(A thread is getting dispatched with MCA EXCEPTION because of MCA.)"},
	{.status = 785, .dos_err = 317, .posix_err = 0, .msg = R"(Access to %1 is monitored by policy rule %2.)"},
	{.status = 786, .dos_err = 317, .posix_err = 0, .msg = R"(Access to %1 has been restricted by your Administrator by policy rule %2.)"},
	{.status = 787, .dos_err = 317, .posix_err = 0, .msg = R"(A valid hibernation file has been invalidated and should be abandoned.)"},
	{.status = 788, .dos_err = 317, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs; the data has been lost.

This error may be caused by network connectivity issues. Please try to save this file elsewhere.)"},
	{.status = 789, .dos_err = 317, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs; the data has been lost.

This error was returned by the server on which the file exists. Please try to save this file elsewhere.)"},
	{.status = 790, .dos_err = 317, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs; the data has been lost.

This error may be caused if the device has been removed or the media is write-protected.)"},
	{.status = 791, .dos_err = 317, .posix_err = 0, .msg = R"(The resources required for this device conflict with the MCFG table.)"},
	{.status = 792, .dos_err = 317, .posix_err = 0, .msg = R"(The volume repair could not be performed while it is online.

Please schedule to take the volume offline so that it can be repaired.)"},
	{.status = 793, .dos_err = 317, .posix_err = 0, .msg = R"(The volume repair was not successful.)"},
	{.status = 794, .dos_err = 317, .posix_err = 0, .msg = R"(One of the volume corruption logs is full. Further corruptions that may be detected won't be logged.)"},
	{.status = 795, .dos_err = 317, .posix_err = 0, .msg = R"(One of the volume corruption logs is internally corrupted and needs to be recreated. The volume may contain undetected corruptions and must be scanned.)"},
	{.status = 796, .dos_err = 317, .posix_err = 0, .msg = R"(One of the volume corruption logs is unavailable for being operated on.)"},
	{.status = 797, .dos_err = 317, .posix_err = 0, .msg = R"(One of the volume corruption logs was deleted while still having corruption records in them. The volume contains detected corruptions and must be scanned.)"},
	{.status = 798, .dos_err = 317, .posix_err = 0, .msg = R"(One of the volume corruption logs was cleared by chkdsk and no longer contains real corruptions.)"},
	{.status = 799, .dos_err = 317, .posix_err = 0, .msg = R"(Orphaned files exist on the volume but could not be recovered because no more new names could be created in the recovery directory. Files must be moved from the recovery directory.)"},
	{.status = 800, .dos_err = 317, .posix_err = 0, .msg = R"(The oplock that was associated with this handle is now associated with a different handle.)"},
	{.status = 801, .dos_err = 317, .posix_err = 0, .msg = R"(An oplock of the requested level cannot be granted.  An oplock of a lower level may be available.)"},
	{.status = 802, .dos_err = 317, .posix_err = 0, .msg = R"(The operation did not complete successfully because it would cause an oplock to be broken. The caller has requested that existing oplocks not be broken.)"},
	{.status = 803, .dos_err = 317, .posix_err = 0, .msg = R"(The handle with which this oplock was associated has been closed.  The oplock is now broken.)"},
	{.status = 804, .dos_err = 317, .posix_err = 0, .msg = R"(The specified access control entry (ACE) does not contain a condition.)"},
	{.status = 805, .dos_err = 317, .posix_err = 0, .msg = R"(The specified access control entry (ACE) contains an invalid condition.)"},
	{.status = 806, .dos_err = 317, .posix_err = 0, .msg = R"(Access to the specified file handle has been revoked.)"},
	{.status = 807, .dos_err = 317, .posix_err = 0, .msg = R"({Image Relocated}

An image file was mapped at a different address from the one specified in the image file but fixups will still be automatically performed on the image.)"},
	{.status = 808, .dos_err = 317, .posix_err = 0, .msg = R"(The read or write operation to an encrypted file could not be completed because the file has not been opened for data access.)"},
	{.status = 809, .dos_err = 317, .posix_err = 0, .msg = R"(File metadata optimization is already in progress.)"},
	{.status = 810, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation failed due to quota operation is still in progress.)"},
	{.status = 811, .dos_err = 317, .posix_err = 0, .msg = R"(Access to the specified handle has been revoked.)"},
	{.status = 812, .dos_err = 317, .posix_err = 0, .msg = R"(The callback function must be invoked inline.)"},
	{.status = 813, .dos_err = 317, .posix_err = 0, .msg = R"(The specified CPU Set IDs are invalid.)"},
	{.status = 814, .dos_err = 317, .posix_err = 0, .msg = R"(The specified enclave has not yet been terminated.)"},
	{.status = 815, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to access protected memory in violation of its secure access policy.)"},
	{.status = 871, .dos_err = 765, .posix_err = 0, .msg = R"(An operation is blocked waiting for an oplock.)"},
	{.status = 872, .dos_err = 741, .posix_err = 0, .msg = R"(A reparse should be performed by the Object Manager from the global root to escape the container name space.)"},
	{.status = 994, .dos_err = 317, .posix_err = 0, .msg = R"(Access to the extended attribute was denied.)"},
	{.status = 995, .dos_err = 317, .posix_err = 0, .msg = R"(The I/O operation has been aborted because of either a thread exit or an application request.)"},
	{.status = 996, .dos_err = 317, .posix_err = 0, .msg = R"(Overlapped I/O event is not in a signaled state.)"},
	{.status = 997, .dos_err = 317, .posix_err = 0, .msg = R"(Overlapped I/O operation is in progress.)"},
	{.status = 998, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid access to memory location.)"},
	{.status = 999, .dos_err = 317, .posix_err = 0, .msg = R"(Error performing inpage operation.)"},
	{.status = 1001, .dos_err = 317, .posix_err = 0, .msg = R"(Recursion too deep; the stack overflowed.)"},
	{.status = 1002, .dos_err = 317, .posix_err = 0, .msg = R"(The window cannot act on the sent message.)"},
	{.status = 1003, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot complete this function.)"},
	{.status = 1004, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid flags.)"},
	{.status = 1005, .dos_err = 317, .posix_err = 0, .msg = R"(The volume does not contain a recognized file system.

Please make sure that all required file system drivers are loaded and that the volume is not corrupted.)"},
	{.status = 1006, .dos_err = 317, .posix_err = 0, .msg = R"(The volume for a file has been externally altered so that the opened file is no longer valid.)"},
	{.status = 1007, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation cannot be performed in full-screen mode.)"},
	{.status = 1008, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to reference a token that does not exist.)"},
	{.status = 1009, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration registry database is corrupt.)"},
	{.status = 1010, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration registry key is invalid.)"},
	{.status = 1011, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration registry key could not be opened.)"},
	{.status = 1012, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration registry key could not be read.)"},
	{.status = 1013, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration registry key could not be written.)"},
	{.status = 1014, .dos_err = 317, .posix_err = 0, .msg = R"(One of the files in the registry database had to be recovered by use of a log or alternate copy. The recovery was successful.)"},
	{.status = 1015, .dos_err = 317, .posix_err = 0, .msg = R"(The registry is corrupted. The structure of one of the files containing registry data is corrupted, or the system's memory image of the file is corrupted, or the file could not be recovered because the alternate copy or log was absent or corrupted.)"},
	{.status = 1016, .dos_err = 317, .posix_err = 0, .msg = R"(An I/O operation initiated by the registry failed unrecoverably. The registry could not read in, or write out, or flush, one of the files that contain the system's image of the registry.)"},
	{.status = 1017, .dos_err = 317, .posix_err = 0, .msg = R"(The system has attempted to load or restore a file into the registry, but the specified file is not in a registry file format.)"},
	{.status = 1018, .dos_err = 317, .posix_err = 0, .msg = R"(Illegal operation attempted on a registry key that has been marked for deletion.)"},
	{.status = 1019, .dos_err = 317, .posix_err = 0, .msg = R"(System could not allocate the required space in a registry log.)"},
	{.status = 1020, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot create a symbolic link in a registry key that already has subkeys or values.)"},
	{.status = 1021, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot create a stable subkey under a volatile parent key.)"},
	{.status = 1022, .dos_err = 317, .posix_err = 0, .msg = R"(A notify change request is being completed and the information is not being returned in the caller's buffer. The caller now needs to enumerate the files to find the changes.)"},
	{.status = 1051, .dos_err = 317, .posix_err = 0, .msg = R"(A stop control has been sent to a service that other running services are dependent on.)"},
	{.status = 1052, .dos_err = 317, .posix_err = 0, .msg = R"(The requested control is not valid for this service.)"},
	{.status = 1053, .dos_err = 317, .posix_err = 0, .msg = R"(The service did not respond to the start or control request in a timely fashion.)"},
	{.status = 1054, .dos_err = 317, .posix_err = 0, .msg = R"(A thread could not be created for the service.)"},
	{.status = 1055, .dos_err = 317, .posix_err = 0, .msg = R"(The service database is locked.)"},
	{.status = 1056, .dos_err = 317, .posix_err = 0, .msg = R"(An instance of the service is already running.)"},
	{.status = 1057, .dos_err = 317, .posix_err = 0, .msg = R"(The account name is invalid or does not exist, or the password is invalid for the account name specified.)"},
	{.status = 1058, .dos_err = 317, .posix_err = 0, .msg = R"(The service cannot be started, either because it is disabled or because it has no enabled devices associated with it.)"},
	{.status = 1059, .dos_err = 317, .posix_err = 0, .msg = R"(Circular service dependency was specified.)"},
	{.status = 1060, .dos_err = 317, .posix_err = 0, .msg = R"(The specified service does not exist as an installed service.)"},
	{.status = 1061, .dos_err = 317, .posix_err = 0, .msg = R"(The service cannot accept control messages at this time.)"},
	{.status = 1062, .dos_err = 317, .posix_err = 0, .msg = R"(The service has not been started.)"},
	{.status = 1063, .dos_err = 317, .posix_err = 0, .msg = R"(The service process could not connect to the service controller.)"},
	{.status = 1064, .dos_err = 317, .posix_err = 0, .msg = R"(An exception occurred in the service when handling the control request.)"},
	{.status = 1065, .dos_err = 317, .posix_err = 0, .msg = R"(The database specified does not exist.)"},
	{.status = 1066, .dos_err = 317, .posix_err = 0, .msg = R"(The service has returned a service-specific error code.)"},
	{.status = 1067, .dos_err = 317, .posix_err = 0, .msg = R"(The process terminated unexpectedly.)"},
	{.status = 1068, .dos_err = 317, .posix_err = 0, .msg = R"(The dependency service or group failed to start.)"},
	{.status = 1069, .dos_err = 317, .posix_err = 0, .msg = R"(The service did not start due to a logon failure.)"},
	{.status = 1070, .dos_err = 317, .posix_err = 0, .msg = R"(After starting, the service hung in a start-pending state.)"},
	{.status = 1071, .dos_err = 317, .posix_err = 0, .msg = R"(The specified service database lock is invalid.)"},
	{.status = 1072, .dos_err = 317, .posix_err = 0, .msg = R"(The specified service has been marked for deletion.)"},
	{.status = 1073, .dos_err = 317, .posix_err = 0, .msg = R"(The specified service already exists.)"},
	{.status = 1074, .dos_err = 317, .posix_err = 0, .msg = R"(The system is currently running with the last-known-good configuration.)"},
	{.status = 1075, .dos_err = 317, .posix_err = 0, .msg = R"(The dependency service does not exist or has been marked for deletion.)"},
	{.status = 1076, .dos_err = 317, .posix_err = 0, .msg = R"(The current boot has already been accepted for use as the last-known-good control set.)"},
	{.status = 1077, .dos_err = 317, .posix_err = 0, .msg = R"(No attempts to start the service have been made since the last boot.)"},
	{.status = 1078, .dos_err = 317, .posix_err = 0, .msg = R"(The name is already in use as either a service name or a service display name.)"},
	{.status = 1079, .dos_err = 317, .posix_err = 0, .msg = R"(The account specified for this service is different from the account specified for other services running in the same process.)"},
	{.status = 1080, .dos_err = 317, .posix_err = 0, .msg = R"(Failure actions can only be set for Win32 services, not for drivers.)"},
	{.status = 1081, .dos_err = 317, .posix_err = 0, .msg = R"(This service runs in the same process as the service control manager.

Therefore, the service control manager cannot take action if this service's process terminates unexpectedly.)"},
	{.status = 1082, .dos_err = 317, .posix_err = 0, .msg = R"(No recovery program has been configured for this service.)"},
	{.status = 1083, .dos_err = 317, .posix_err = 0, .msg = R"(The executable program that this service is configured to run in does not implement the service.)"},
	{.status = 1084, .dos_err = 317, .posix_err = 0, .msg = R"(This service cannot be started in Safe Mode)"},
	{.status = 1100, .dos_err = 317, .posix_err = 0, .msg = R"(The physical end of the tape has been reached.)"},
	{.status = 1101, .dos_err = 317, .posix_err = 0, .msg = R"(A tape access reached a filemark.)"},
	{.status = 1102, .dos_err = 317, .posix_err = 0, .msg = R"(The beginning of the tape or a partition was encountered.)"},
	{.status = 1103, .dos_err = 317, .posix_err = 0, .msg = R"(A tape access reached the end of a set of files.)"},
	{.status = 1104, .dos_err = 317, .posix_err = 0, .msg = R"(No more data is on the tape.)"},
	{.status = 1105, .dos_err = 317, .posix_err = 0, .msg = R"(Tape could not be partitioned.)"},
	{.status = 1106, .dos_err = 317, .posix_err = 0, .msg = R"(When accessing a new tape of a multivolume partition, the current block size is incorrect.)"},
	{.status = 1107, .dos_err = 317, .posix_err = 0, .msg = R"(Tape partition information could not be found when loading a tape.)"},
	{.status = 1108, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to lock the media eject mechanism.)"},
	{.status = 1109, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to unload the media.)"},
	{.status = 1110, .dos_err = 317, .posix_err = 0, .msg = R"(The media in the drive may have changed.)"},
	{.status = 1111, .dos_err = 317, .posix_err = 0, .msg = R"(The I/O bus was reset.)"},
	{.status = 1112, .dos_err = 317, .posix_err = 0, .msg = R"(No media in drive.)"},
	{.status = 1113, .dos_err = 317, .posix_err = 0, .msg = R"(No mapping for the Unicode character exists in the target multi-byte code page.)"},
	{.status = 1114, .dos_err = 317, .posix_err = 0, .msg = R"(A dynamic link library (DLL) initialization routine failed.)"},
	{.status = 1115, .dos_err = 317, .posix_err = 0, .msg = R"(A system shutdown is in progress.)"},
	{.status = 1116, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to abort the system shutdown because no shutdown was in progress.)"},
	{.status = 1117, .dos_err = 317, .posix_err = 0, .msg = R"(The request could not be performed because of an I/O device error.)"},
	{.status = 1118, .dos_err = 317, .posix_err = 0, .msg = R"(No serial device was successfully initialized. The serial driver will unload.)"},
	{.status = 1119, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to open a device that was sharing an interrupt request (IRQ) with other devices. At least one other device that uses that IRQ was already opened.)"},
	{.status = 1120, .dos_err = 317, .posix_err = 0, .msg = R"(A serial I/O operation was completed by another write to the serial port.

(The IOCTL_SERIAL_XOFF_COUNTER reached zero.))"},
	{.status = 1121, .dos_err = 317, .posix_err = 0, .msg = R"(A serial I/O operation completed because the timeout period expired.

(The IOCTL_SERIAL_XOFF_COUNTER did not reach zero.))"},
	{.status = 1122, .dos_err = 317, .posix_err = 0, .msg = R"(No ID address mark was found on the floppy disk.)"},
	{.status = 1123, .dos_err = 317, .posix_err = 0, .msg = R"(Mismatch between the floppy disk sector ID field and the floppy disk controller track address.)"},
	{.status = 1124, .dos_err = 317, .posix_err = 0, .msg = R"(The floppy disk controller reported an error that is not recognized by the floppy disk driver.)"},
	{.status = 1125, .dos_err = 317, .posix_err = 0, .msg = R"(The floppy disk controller returned inconsistent results in its registers.)"},
	{.status = 1126, .dos_err = 317, .posix_err = 0, .msg = R"(While accessing the hard disk, a recalibrate operation failed, even after retries.)"},
	{.status = 1127, .dos_err = 317, .posix_err = 0, .msg = R"(While accessing the hard disk, a disk operation failed even after retries.)"},
	{.status = 1128, .dos_err = 317, .posix_err = 0, .msg = R"(While accessing the hard disk, a disk controller reset was needed, but even that failed.)"},
	{.status = 1129, .dos_err = 317, .posix_err = 0, .msg = R"(Physical end of tape encountered.)"},
	{.status = 1130, .dos_err = 317, .posix_err = 0, .msg = R"(Not enough server memory resources are available to process this command.)"},
	{.status = 1131, .dos_err = 317, .posix_err = 0, .msg = R"(A potential deadlock condition has been detected.)"},
	{.status = 1132, .dos_err = 317, .posix_err = 0, .msg = R"(The base address or the file offset specified does not have the proper alignment.)"},
	{.status = 1140, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to change the system power state was vetoed by another application or driver.)"},
	{.status = 1141, .dos_err = 317, .posix_err = 0, .msg = R"(The system BIOS failed an attempt to change the system power state.)"},
	{.status = 1142, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to create more links on a file than the file system supports.)"},
	{.status = 1150, .dos_err = 317, .posix_err = 0, .msg = R"(The specified program requires a newer version of Windows.)"},
	{.status = 1151, .dos_err = 317, .posix_err = 0, .msg = R"(The specified program is not a Windows or MS-DOS program.)"},
	{.status = 1152, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot start more than one instance of the specified program.)"},
	{.status = 1153, .dos_err = 317, .posix_err = 0, .msg = R"(The specified program was written for an earlier version of Windows.)"},
	{.status = 1154, .dos_err = 317, .posix_err = 0, .msg = R"(One of the library files needed to run this application is damaged.)"},
	{.status = 1155, .dos_err = 317, .posix_err = 0, .msg = R"(No application is associated with the specified file for this operation.)"},
	{.status = 1156, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred in sending the command to the application.)"},
	{.status = 1157, .dos_err = 317, .posix_err = 0, .msg = R"(One of the library files needed to run this application cannot be found.)"},
	{.status = 1158, .dos_err = 317, .posix_err = 0, .msg = R"(The current process has used all of its system allowance of handles for Window Manager objects.)"},
	{.status = 1159, .dos_err = 317, .posix_err = 0, .msg = R"(The message can be used only with synchronous operations.)"},
	{.status = 1160, .dos_err = 317, .posix_err = 0, .msg = R"(The indicated source element has no media.)"},
	{.status = 1161, .dos_err = 317, .posix_err = 0, .msg = R"(The indicated destination element already contains media.)"},
	{.status = 1162, .dos_err = 317, .posix_err = 0, .msg = R"(The indicated element does not exist.)"},
	{.status = 1163, .dos_err = 317, .posix_err = 0, .msg = R"(The indicated element is part of a magazine that is not present.)"},
	{.status = 1164, .dos_err = 317, .posix_err = 0, .msg = R"(The indicated device requires reinitialization due to hardware errors.)"},
	{.status = 1165, .dos_err = 317, .posix_err = 0, .msg = R"(The device has indicated that cleaning is required before further operations are attempted.)"},
	{.status = 1166, .dos_err = 317, .posix_err = 0, .msg = R"(The device has indicated that its door is open.)"},
	{.status = 1167, .dos_err = 317, .posix_err = 0, .msg = R"(The device is not connected.)"},
	{.status = 1168, .dos_err = 317, .posix_err = 0, .msg = R"(Element not found.)"},
	{.status = 1169, .dos_err = 317, .posix_err = 0, .msg = R"(There was no match for the specified key in the index.)"},
	{.status = 1170, .dos_err = 317, .posix_err = 0, .msg = R"(The property set specified does not exist on the object.)"},
	{.status = 1171, .dos_err = 317, .posix_err = 0, .msg = R"(The point passed to GetMouseMovePoints is not in the buffer.)"},
	{.status = 1172, .dos_err = 317, .posix_err = 0, .msg = R"(The tracking (workstation) service is not running.)"},
	{.status = 1173, .dos_err = 317, .posix_err = 0, .msg = R"(The Volume ID could not be found.)"},
	{.status = 1175, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to remove the file to be replaced.)"},
	{.status = 1176, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to move the replacement file to the file to be replaced. The file to be replaced has retained its original name.)"},
	{.status = 1177, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to move the replacement file to the file to be replaced. The file to be replaced has been renamed using the backup name.)"},
	{.status = 1178, .dos_err = 317, .posix_err = 0, .msg = R"(The volume change journal is being deleted.)"},
	{.status = 1179, .dos_err = 317, .posix_err = 0, .msg = R"(The volume change journal is not active.)"},
	{.status = 1180, .dos_err = 317, .posix_err = 0, .msg = R"(A file was found, but it may not be the correct file.)"},
	{.status = 1181, .dos_err = 317, .posix_err = 0, .msg = R"(The journal entry has been deleted from the journal.)"},
	{.status = 1183, .dos_err = 317, .posix_err = 0, .msg = R"(Driver Verifier Volatile settings cannot be set when CFG and IO are enabled.)"},
	{.status = 1184, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to access a partition that has begun termination.)"},
	{.status = 1190, .dos_err = 317, .posix_err = 0, .msg = R"(A system shutdown has already been scheduled.)"},
	{.status = 1191, .dos_err = 317, .posix_err = 0, .msg = R"(The system shutdown cannot be initiated because there are other users logged on to the computer.)"},
	{.status = 1200, .dos_err = 317, .posix_err = 0, .msg = R"(The specified device name is invalid.)"},
	{.status = 1201, .dos_err = 317, .posix_err = 0, .msg = R"(The device is not currently connected but it is a remembered connection.)"},
	{.status = 1202, .dos_err = 317, .posix_err = 0, .msg = R"(The local device name has a remembered connection to another network resource.)"},
	{.status = 1203, .dos_err = 317, .posix_err = 0, .msg = R"(The network path was either typed incorrectly, does not exist, or the network provider is not currently available. Please try retyping the path or contact your network administrator.)"},
	{.status = 1204, .dos_err = 317, .posix_err = 0, .msg = R"(The specified network provider name is invalid.)"},
	{.status = 1205, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to open the network connection profile.)"},
	{.status = 1206, .dos_err = 317, .posix_err = 0, .msg = R"(The network connection profile is corrupted.)"},
	{.status = 1207, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot enumerate a noncontainer.)"},
	{.status = 1208, .dos_err = 317, .posix_err = 0, .msg = R"(An extended error has occurred.)"},
	{.status = 1209, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified group name is invalid.)"},
	{.status = 1210, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified computer name is invalid.)"},
	{.status = 1211, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified event name is invalid.)"},
	{.status = 1212, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified domain name is invalid.)"},
	{.status = 1213, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified service name is invalid.)"},
	{.status = 1214, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified network name is invalid.)"},
	{.status = 1215, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified share name is invalid.)"},
	{.status = 1216, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified password is invalid.)"},
	{.status = 1217, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified message name is invalid.)"},
	{.status = 1218, .dos_err = 317, .posix_err = 0, .msg = R"(The format of the specified message destination is invalid.)"},
	{.status = 1219, .dos_err = 317, .posix_err = 0, .msg = R"(Multiple connections to a server or shared resource by the same user, using more than one user name, are not allowed. Disconnect all previous connections to the server or shared resource and try again.)"},
	{.status = 1220, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to establish a session to a network server, but there are already too many sessions established to that server.)"},
	{.status = 1221, .dos_err = 317, .posix_err = 0, .msg = R"(The workgroup or domain name is already in use by another computer on the network.)"},
	{.status = 1222, .dos_err = 317, .posix_err = 0, .msg = R"(The network is not present or not started.)"},
	{.status = 1223, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was canceled by the user.)"},
	{.status = 1224, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation cannot be performed on a file with a user-mapped section open.)"},
	{.status = 1225, .dos_err = 317, .posix_err = 0, .msg = R"(The remote computer refused the network connection.)"},
	{.status = 1226, .dos_err = 317, .posix_err = 0, .msg = R"(The network connection was gracefully closed.)"},
	{.status = 1227, .dos_err = 317, .posix_err = 0, .msg = R"(The network transport endpoint already has an address associated with it.)"},
	{.status = 1228, .dos_err = 317, .posix_err = 0, .msg = R"(An address has not yet been associated with the network endpoint.)"},
	{.status = 1229, .dos_err = 317, .posix_err = 0, .msg = R"(An operation was attempted on a nonexistent network connection.)"},
	{.status = 1230, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid operation was attempted on an active network connection.)"},
	{.status = 1231, .dos_err = 317, .posix_err = 0, .msg = R"(The network location cannot be reached. For information about network troubleshooting, see Windows Help.)"},
	{.status = 1232, .dos_err = 317, .posix_err = 0, .msg = R"(The network location cannot be reached. For information about network troubleshooting, see Windows Help.)"},
	{.status = 1233, .dos_err = 317, .posix_err = 0, .msg = R"(The network location cannot be reached. For information about network troubleshooting, see Windows Help.)"},
	{.status = 1234, .dos_err = 317, .posix_err = 0, .msg = R"(No service is operating at the destination network endpoint on the remote system.)"},
	{.status = 1235, .dos_err = 317, .posix_err = 0, .msg = R"(The request was aborted.)"},
	{.status = 1236, .dos_err = 317, .posix_err = 0, .msg = R"(The network connection was aborted by the local system.)"},
	{.status = 1237, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed. A retry should be performed.)"},
	{.status = 1238, .dos_err = 317, .posix_err = 0, .msg = R"(A connection to the server could not be made because the limit on the number of concurrent connections for this account has been reached.)"},
	{.status = 1239, .dos_err = 317, .posix_err = 0, .msg = R"(Attempting to log in during an unauthorized time of day for this account.)"},
	{.status = 1240, .dos_err = 317, .posix_err = 0, .msg = R"(The account is not authorized to log in from this station.)"},
	{.status = 1241, .dos_err = 317, .posix_err = 0, .msg = R"(The network address could not be used for the operation requested.)"},
	{.status = 1242, .dos_err = 317, .posix_err = 0, .msg = R"(The service is already registered.)"},
	{.status = 1243, .dos_err = 317, .posix_err = 0, .msg = R"(The specified service does not exist.)"},
	{.status = 1244, .dos_err = 317, .posix_err = 0, .msg = R"(The operation being requested was not performed because the user has not been authenticated.)"},
	{.status = 1245, .dos_err = 317, .posix_err = 0, .msg = R"(The operation being requested was not performed because the user has not logged on to the network. The specified service does not exist.)"},
	{.status = 1246, .dos_err = 317, .posix_err = 0, .msg = R"(Continue with work in progress.)"},
	{.status = 1247, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to perform an initialization operation when initialization has already been completed.)"},
	{.status = 1248, .dos_err = 317, .posix_err = 0, .msg = R"(No more local devices.)"},
	{.status = 1249, .dos_err = 317, .posix_err = 0, .msg = R"(The specified site does not exist.)"},
	{.status = 1250, .dos_err = 317, .posix_err = 0, .msg = R"(A domain controller with the specified name already exists.)"},
	{.status = 1251, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is supported only when you are connected to the server.)"},
	{.status = 1252, .dos_err = 317, .posix_err = 0, .msg = R"(The group policy framework should call the extension even if there are no changes.)"},
	{.status = 1253, .dos_err = 317, .posix_err = 0, .msg = R"(The specified user does not have a valid profile.)"},
	{.status = 1254, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not supported on a computer running Windows Server 2003 for Small Business Server)"},
	{.status = 1255, .dos_err = 317, .posix_err = 0, .msg = R"(The server machine is shutting down.)"},
	{.status = 1256, .dos_err = 317, .posix_err = 0, .msg = R"(The remote system is not available. For information about network troubleshooting, see Windows Help.)"},
	{.status = 1257, .dos_err = 317, .posix_err = 0, .msg = R"(The security identifier provided is not from an account domain.)"},
	{.status = 1258, .dos_err = 317, .posix_err = 0, .msg = R"(The security identifier provided does not have a domain component.)"},
	{.status = 1259, .dos_err = 317, .posix_err = 0, .msg = R"(AppHelp dialog canceled thus preventing the application from starting.)"},
	{.status = 1260, .dos_err = 317, .posix_err = 0, .msg = R"(This program is blocked by group policy. For more information, contact your system administrator.)"},
	{.status = 1261, .dos_err = 317, .posix_err = 0, .msg = R"(A program attempt to use an invalid register value. Normally caused by an uninitialized register. This error is Itanium specific.)"},
	{.status = 1262, .dos_err = 317, .posix_err = 0, .msg = R"(The share is currently offline or does not exist.)"},
	{.status = 1263, .dos_err = 317, .posix_err = 0, .msg = R"(The Kerberos protocol encountered an error while validating the KDC certificate during smartcard logon. There is more information in the system event log.)"},
	{.status = 1264, .dos_err = 317, .posix_err = 0, .msg = R"(The Kerberos protocol encountered an error while attempting to utilize the smartcard subsystem.)"},
	{.status = 1265, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot contact a domain controller to service the authentication request. Please try again later.)"},
	{.status = 1271, .dos_err = 317, .posix_err = 0, .msg = R"(The machine is locked and cannot be shut down without the force option.)"},
	{.status = 1272, .dos_err = 317, .posix_err = 0, .msg = R"(You can't access this shared folder because your organization's security policies block unauthenticated guest access. These policies help protect your PC from unsafe or malicious devices on the network.)"},
	{.status = 1273, .dos_err = 317, .posix_err = 0, .msg = R"(An application-defined callback gave invalid data when called.)"},
	{.status = 1274, .dos_err = 317, .posix_err = 0, .msg = R"(The group policy framework should call the extension in the synchronous foreground policy refresh.)"},
	{.status = 1275, .dos_err = 317, .posix_err = 0, .msg = R"(This driver has been blocked from loading)"},
	{.status = 1276, .dos_err = 317, .posix_err = 0, .msg = R"(A dynamic link library (DLL) referenced a module that was neither a DLL nor the process's executable image.)"},
	{.status = 1277, .dos_err = 317, .posix_err = 0, .msg = R"(Windows cannot open this program since it has been disabled.)"},
	{.status = 1278, .dos_err = 317, .posix_err = 0, .msg = R"(Windows cannot open this program because the license enforcement system has been tampered with or become corrupted.)"},
	{.status = 1279, .dos_err = 317, .posix_err = 0, .msg = R"(A transaction recover failed.)"},
	{.status = 1280, .dos_err = 317, .posix_err = 0, .msg = R"(The current thread has already been converted to a fiber.)"},
	{.status = 1281, .dos_err = 317, .posix_err = 0, .msg = R"(The current thread has already been converted from a fiber.)"},
	{.status = 1282, .dos_err = 317, .posix_err = 0, .msg = R"(The system detected an overrun of a stack-based buffer in this application. This overrun could potentially allow a malicious user to gain control of this application.)"},
	{.status = 1283, .dos_err = 317, .posix_err = 0, .msg = R"(Data present in one of the parameters is more than the function can operate on.)"},
	{.status = 1284, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to do an operation on a debug object failed because the object is in the process of being deleted.)"},
	{.status = 1285, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to delay-load a .dll or get a function address in a delay-loaded .dll failed.)"},
	{.status = 1286, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is a 16-bit application. You do not have permissions to execute 16-bit applications. Check your permissions with your system administrator.)"},
	{.status = 1287, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient information exists to identify the cause of failure.)"},
	{.status = 1288, .dos_err = 317, .posix_err = 0, .msg = R"(The parameter passed to a C runtime function is incorrect.)"},
	{.status = 1289, .dos_err = 317, .posix_err = 0, .msg = R"(The operation occurred beyond the valid data length of the file.)"},
	{.status = 1290, .dos_err = 317, .posix_err = 0, .msg = R"(The service start failed since one or more services in the same process have an incompatible service SID type setting. A service with restricted service SID type can only coexist in the same process with other services with a restricted SID type. If the service SID type for this service was just configured, the hosting process must be restarted in order to start this service.)"},
	{.status = 1291, .dos_err = 317, .posix_err = 0, .msg = R"(The process hosting the driver for this device has been terminated.)"},
	{.status = 1292, .dos_err = 317, .posix_err = 0, .msg = R"(An operation attempted to exceed an implementation-defined limit.)"},
	{.status = 1293, .dos_err = 317, .posix_err = 0, .msg = R"(Either the target process, or the target thread's containing process, is a protected process.)"},
	{.status = 1294, .dos_err = 317, .posix_err = 0, .msg = R"(The service notification client is lagging too far behind the current state of services in the machine.)"},
	{.status = 1295, .dos_err = 317, .posix_err = 0, .msg = R"(The requested file operation failed because the storage quota was exceeded.

To free up disk space, move files to a different location or delete unnecessary files. For more information, contact your system administrator.)"},
	{.status = 1296, .dos_err = 317, .posix_err = 0, .msg = R"(The requested file operation failed because the storage policy blocks that type of file. For more information, contact your system administrator.)"},
	{.status = 1297, .dos_err = 317, .posix_err = 0, .msg = R"(A privilege that the service requires to function properly does not exist in the service account configuration.

You may use the Services Microsoft Management Console (MMC) snap-in (services.msc) and the Local Security Settings MMC snap-in (secpol.msc) to view the service configuration and the account configuration.)"},
	{.status = 1298, .dos_err = 317, .posix_err = 0, .msg = R"(A thread involved in this operation appears to be unresponsive.)"},
	{.status = 1299, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates a particular Security ID may not be assigned as the label of an object.)"},
	{.status = 1300, .dos_err = 317, .posix_err = 0, .msg = R"(Not all privileges or groups referenced are assigned to the caller.)"},
	{.status = 1301, .dos_err = 317, .posix_err = 0, .msg = R"(Some mapping between account names and security IDs was not done.)"},
	{.status = 1302, .dos_err = 317, .posix_err = 0, .msg = R"(No system quota limits are specifically set for this account.)"},
	{.status = 1303, .dos_err = 317, .posix_err = 0, .msg = R"(No encryption key is available. A well-known encryption key was returned.)"},
	{.status = 1304, .dos_err = 317, .posix_err = 0, .msg = R"(The password is too complex to be converted to a LAN Manager password. The LAN Manager password returned is a NULL string.)"},
	{.status = 1305, .dos_err = 317, .posix_err = 0, .msg = R"(The revision level is unknown.)"},
	{.status = 1306, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates two revision levels are incompatible.)"},
	{.status = 1307, .dos_err = 317, .posix_err = 0, .msg = R"(This security ID may not be assigned as the owner of this object.)"},
	{.status = 1308, .dos_err = 317, .posix_err = 0, .msg = R"(This security ID may not be assigned as the primary group of an object.)"},
	{.status = 1309, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt has been made to operate on an impersonation token by a thread that is not currently impersonating a client.)"},
	{.status = 1310, .dos_err = 317, .posix_err = 0, .msg = R"(The group may not be disabled.)"},
	{.status = 1311, .dos_err = 317, .posix_err = 0, .msg = R"(We can't sign you in with this credential because your domain isn't available. Make sure your device is connected to your organization's network and try again. If you previously signed in on this device with another credential, you can sign in with that credential.)"},
	{.status = 1312, .dos_err = 317, .posix_err = 0, .msg = R"(A specified logon session does not exist. It may already have been terminated.)"},
	{.status = 1313, .dos_err = 317, .posix_err = 0, .msg = R"(A specified privilege does not exist.)"},
	{.status = 1314, .dos_err = 317, .posix_err = 0, .msg = R"(A required privilege is not held by the client.)"},
	{.status = 1315, .dos_err = 317, .posix_err = 0, .msg = R"(The name provided is not a properly formed account name.)"},
	{.status = 1316, .dos_err = 317, .posix_err = 0, .msg = R"(The specified account already exists.)"},
	{.status = 1317, .dos_err = 317, .posix_err = 0, .msg = R"(The specified account does not exist.)"},
	{.status = 1318, .dos_err = 317, .posix_err = 0, .msg = R"(The specified group already exists.)"},
	{.status = 1319, .dos_err = 317, .posix_err = 0, .msg = R"(The specified group does not exist.)"},
	{.status = 1320, .dos_err = 317, .posix_err = 0, .msg = R"(Either the specified user account is already a member of the specified group, or the specified group cannot be deleted because it contains a member.)"},
	{.status = 1321, .dos_err = 317, .posix_err = 0, .msg = R"(The specified user account is not a member of the specified group account.)"},
	{.status = 1322, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is disallowed as it could result in an administration account being disabled, deleted or unable to logon.)"},
	{.status = 1323, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to update the password. The value provided as the current password is incorrect.)"},
	{.status = 1324, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to update the password. The value provided for the new password contains values that are not allowed in passwords.)"},
	{.status = 1325, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to update the password. The value provided for the new password does not meet the length, complexity, or history requirements of the domain.)"},
	{.status = 1326, .dos_err = 317, .posix_err = 0, .msg = R"(The user name or password is incorrect.)"},
	{.status = 1327, .dos_err = 317, .posix_err = 0, .msg = R"(Account restrictions are preventing this user from signing in. For example: blank passwords aren't allowed, sign-in times are limited, or a policy restriction has been enforced.)"},
	{.status = 1328, .dos_err = 317, .posix_err = 0, .msg = R"(Your account has time restrictions that keep you from signing in right now.)"},
	{.status = 1329, .dos_err = 317, .posix_err = 0, .msg = R"(This user isn't allowed to sign in to this computer.)"},
	{.status = 1330, .dos_err = 317, .posix_err = 0, .msg = R"(The password for this account has expired.)"},
	{.status = 1331, .dos_err = 317, .posix_err = 0, .msg = R"(This user can't sign in because this account is currently disabled.)"},
	{.status = 1332, .dos_err = 317, .posix_err = 0, .msg = R"(No mapping between account names and security IDs was done.)"},
	{.status = 1333, .dos_err = 317, .posix_err = 0, .msg = R"(Too many local user identifiers (LUIDs) were requested at one time.)"},
	{.status = 1334, .dos_err = 317, .posix_err = 0, .msg = R"(No more local user identifiers (LUIDs) are available.)"},
	{.status = 1335, .dos_err = 317, .posix_err = 0, .msg = R"(The subauthority part of a security ID is invalid for this particular use.)"},
	{.status = 1336, .dos_err = 317, .posix_err = 0, .msg = R"(The access control list (ACL) structure is invalid.)"},
	{.status = 1337, .dos_err = 317, .posix_err = 0, .msg = R"(The security ID structure is invalid.)"},
	{.status = 1338, .dos_err = 317, .posix_err = 0, .msg = R"(The security descriptor structure is invalid.)"},
	{.status = 1340, .dos_err = 317, .posix_err = 0, .msg = R"(The inherited access control list (ACL) or access control entry (ACE) could not be built.)"},
	{.status = 1341, .dos_err = 317, .posix_err = 0, .msg = R"(The server is currently disabled.)"},
	{.status = 1342, .dos_err = 317, .posix_err = 0, .msg = R"(The server is currently enabled.)"},
	{.status = 1343, .dos_err = 317, .posix_err = 0, .msg = R"(The value provided was an invalid value for an identifier authority.)"},
	{.status = 1344, .dos_err = 317, .posix_err = 0, .msg = R"(No more memory is available for security information updates.)"},
	{.status = 1345, .dos_err = 317, .posix_err = 0, .msg = R"(The specified attributes are invalid, or incompatible with the attributes for the group as a whole.)"},
	{.status = 1346, .dos_err = 317, .posix_err = 0, .msg = R"(Either a required impersonation level was not provided, or the provided impersonation level is invalid.)"},
	{.status = 1347, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot open an anonymous level security token.)"},
	{.status = 1348, .dos_err = 317, .posix_err = 0, .msg = R"(The validation information class requested was invalid.)"},
	{.status = 1349, .dos_err = 317, .posix_err = 0, .msg = R"(The type of the token is inappropriate for its attempted use.)"},
	{.status = 1350, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to perform a security operation on an object that has no associated security.)"},
	{.status = 1351, .dos_err = 317, .posix_err = 0, .msg = R"(Configuration information could not be read from the domain controller, either because the machine is unavailable, or access has been denied.)"},
	{.status = 1352, .dos_err = 317, .posix_err = 0, .msg = R"(The security account manager (SAM) or local security authority (LSA) server was in the wrong state to perform the security operation.)"},
	{.status = 1353, .dos_err = 317, .posix_err = 0, .msg = R"(The domain was in the wrong state to perform the security operation.)"},
	{.status = 1354, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is only allowed for the Primary Domain Controller of the domain.)"},
	{.status = 1355, .dos_err = 317, .posix_err = 0, .msg = R"(The specified domain either does not exist or could not be contacted.)"},
	{.status = 1356, .dos_err = 317, .posix_err = 0, .msg = R"(The specified domain already exists.)"},
	{.status = 1357, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to exceed the limit on the number of domains per server.)"},
	{.status = 1358, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to complete the requested operation because of either a catastrophic media failure or a data structure corruption on the disk.)"},
	{.status = 1359, .dos_err = 317, .posix_err = 0, .msg = R"(An internal error occurred.)"},
	{.status = 1360, .dos_err = 317, .posix_err = 0, .msg = R"(Generic access types were contained in an access mask which should already be mapped to nongeneric types.)"},
	{.status = 1361, .dos_err = 317, .posix_err = 0, .msg = R"(A security descriptor is not in the right format (absolute or self-relative).)"},
	{.status = 1362, .dos_err = 317, .posix_err = 0, .msg = R"(The requested action is restricted for use by logon processes only. The calling process has not registered as a logon process.)"},
	{.status = 1363, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot start a new logon session with an ID that is already in use.)"},
	{.status = 1364, .dos_err = 317, .posix_err = 0, .msg = R"(A specified authentication package is unknown.)"},
	{.status = 1365, .dos_err = 317, .posix_err = 0, .msg = R"(The logon session is not in a state that is consistent with the requested operation.)"},
	{.status = 1366, .dos_err = 317, .posix_err = 0, .msg = R"(The logon session ID is already in use.)"},
	{.status = 1367, .dos_err = 317, .posix_err = 0, .msg = R"(A logon request contained an invalid logon type value.)"},
	{.status = 1368, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to impersonate using a named pipe until data has been read from that pipe.)"},
	{.status = 1369, .dos_err = 317, .posix_err = 0, .msg = R"(The transaction state of a registry subtree is incompatible with the requested operation.)"},
	{.status = 1370, .dos_err = 317, .posix_err = 0, .msg = R"(An internal security database corruption has been encountered.)"},
	{.status = 1371, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot perform this operation on built-in accounts.)"},
	{.status = 1372, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot perform this operation on this built-in special group.)"},
	{.status = 1373, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot perform this operation on this built-in special user.)"},
	{.status = 1374, .dos_err = 317, .posix_err = 0, .msg = R"(The user cannot be removed from a group because the group is currently the user's primary group.)"},
	{.status = 1375, .dos_err = 317, .posix_err = 0, .msg = R"(The token is already in use as a primary token.)"},
	{.status = 1376, .dos_err = 317, .posix_err = 0, .msg = R"(The specified local group does not exist.)"},
	{.status = 1377, .dos_err = 317, .posix_err = 0, .msg = R"(The specified account name is not a member of the group.)"},
	{.status = 1378, .dos_err = 317, .posix_err = 0, .msg = R"(The specified account name is already a member of the group.)"},
	{.status = 1379, .dos_err = 317, .posix_err = 0, .msg = R"(The specified local group already exists.)"},
	{.status = 1380, .dos_err = 317, .posix_err = 0, .msg = R"(Logon failure: the user has not been granted the requested logon type at this computer.)"},
	{.status = 1381, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of secrets that may be stored in a single system has been exceeded.)"},
	{.status = 1382, .dos_err = 317, .posix_err = 0, .msg = R"(The length of a secret exceeds the maximum length allowed.)"},
	{.status = 1383, .dos_err = 317, .posix_err = 0, .msg = R"(The local security authority database contains an internal inconsistency.)"},
	{.status = 1384, .dos_err = 317, .posix_err = 0, .msg = R"(During a logon attempt, the user's security context accumulated too many security IDs.)"},
	{.status = 1385, .dos_err = 317, .posix_err = 0, .msg = R"(Logon failure: the user has not been granted the requested logon type at this computer.)"},
	{.status = 1386, .dos_err = 317, .posix_err = 0, .msg = R"(A cross-encrypted password is necessary to change a user password.)"},
	{.status = 1387, .dos_err = 317, .posix_err = 0, .msg = R"(A member could not be added to or removed from the local group because the member does not exist.)"},
	{.status = 1388, .dos_err = 317, .posix_err = 0, .msg = R"(A new member could not be added to a local group because the member has the wrong account type.)"},
	{.status = 1389, .dos_err = 317, .posix_err = 0, .msg = R"(Too many security IDs have been specified.)"},
	{.status = 1390, .dos_err = 317, .posix_err = 0, .msg = R"(A cross-encrypted password is necessary to change this user password.)"},
	{.status = 1391, .dos_err = 317, .posix_err = 0, .msg = R"(Indicates an ACL contains no inheritable components.)"},
	{.status = 1392, .dos_err = 317, .posix_err = 0, .msg = R"(The file or directory is corrupted and unreadable.)"},
	{.status = 1393, .dos_err = 317, .posix_err = 0, .msg = R"(The disk structure is corrupted and unreadable.)"},
	{.status = 1394, .dos_err = 317, .posix_err = 0, .msg = R"(There is no user session key for the specified logon session.)"},
	{.status = 1395, .dos_err = 317, .posix_err = 0, .msg = R"(The service being accessed is licensed for a particular number of connections. No more connections can be made to the service at this time because there are already as many connections as the service can accept.)"},
	{.status = 1396, .dos_err = 317, .posix_err = 0, .msg = R"(The target account name is incorrect.)"},
	{.status = 1397, .dos_err = 317, .posix_err = 0, .msg = R"(Mutual Authentication failed. The server's password is out of date at the domain controller.)"},
	{.status = 1398, .dos_err = 317, .posix_err = 0, .msg = R"(There is a time and/or date difference between the client and server.)"},
	{.status = 1399, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed on the current domain.)"},
	{.status = 1400, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid window handle.)"},
	{.status = 1401, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid menu handle.)"},
	{.status = 1402, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid cursor handle.)"},
	{.status = 1403, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid accelerator table handle.)"},
	{.status = 1404, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid hook handle.)"},
	{.status = 1405, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid handle to a multiple-window position structure.)"},
	{.status = 1406, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot create a top-level child window.)"},
	{.status = 1407, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot find window class.)"},
	{.status = 1408, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid window; it belongs to other thread.)"},
	{.status = 1409, .dos_err = 317, .posix_err = 0, .msg = R"(Hot key is already registered.)"},
	{.status = 1410, .dos_err = 317, .posix_err = 0, .msg = R"(Class already exists.)"},
	{.status = 1411, .dos_err = 317, .posix_err = 0, .msg = R"(Class does not exist.)"},
	{.status = 1412, .dos_err = 317, .posix_err = 0, .msg = R"(Class still has open windows.)"},
	{.status = 1413, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid index.)"},
	{.status = 1414, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid icon handle.)"},
	{.status = 1415, .dos_err = 317, .posix_err = 0, .msg = R"(Using private DIALOG window words.)"},
	{.status = 1416, .dos_err = 317, .posix_err = 0, .msg = R"(The list box identifier was not found.)"},
	{.status = 1417, .dos_err = 317, .posix_err = 0, .msg = R"(No wildcards were found.)"},
	{.status = 1418, .dos_err = 317, .posix_err = 0, .msg = R"(Thread does not have a clipboard open.)"},
	{.status = 1419, .dos_err = 317, .posix_err = 0, .msg = R"(Hot key is not registered.)"},
	{.status = 1420, .dos_err = 317, .posix_err = 0, .msg = R"(The window is not a valid dialog window.)"},
	{.status = 1421, .dos_err = 317, .posix_err = 0, .msg = R"(Control ID not found.)"},
	{.status = 1422, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid message for a combo box because it does not have an edit control.)"},
	{.status = 1423, .dos_err = 317, .posix_err = 0, .msg = R"(The window is not a combo box.)"},
	{.status = 1424, .dos_err = 317, .posix_err = 0, .msg = R"(Height must be less than 256.)"},
	{.status = 1425, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid device context (DC) handle.)"},
	{.status = 1426, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid hook procedure type.)"},
	{.status = 1427, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid hook procedure.)"},
	{.status = 1428, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot set nonlocal hook without a module handle.)"},
	{.status = 1429, .dos_err = 317, .posix_err = 0, .msg = R"(This hook procedure can only be set globally.)"},
	{.status = 1430, .dos_err = 317, .posix_err = 0, .msg = R"(The journal hook procedure is already installed.)"},
	{.status = 1431, .dos_err = 317, .posix_err = 0, .msg = R"(The hook procedure is not installed.)"},
	{.status = 1432, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid message for single-selection list box.)"},
	{.status = 1433, .dos_err = 317, .posix_err = 0, .msg = R"(LB_SETCOUNT sent to non-lazy list box.)"},
	{.status = 1434, .dos_err = 317, .posix_err = 0, .msg = R"(This list box does not support tab stops.)"},
	{.status = 1435, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot destroy object created by another thread.)"},
	{.status = 1436, .dos_err = 317, .posix_err = 0, .msg = R"(Child windows cannot have menus.)"},
	{.status = 1437, .dos_err = 317, .posix_err = 0, .msg = R"(The window does not have a system menu.)"},
	{.status = 1438, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid message box style.)"},
	{.status = 1439, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid system-wide (SPI_*) parameter.)"},
	{.status = 1440, .dos_err = 317, .posix_err = 0, .msg = R"(Screen already locked.)"},
	{.status = 1441, .dos_err = 317, .posix_err = 0, .msg = R"(All handles to windows in a multiple-window position structure must have the same parent.)"},
	{.status = 1442, .dos_err = 317, .posix_err = 0, .msg = R"(The window is not a child window.)"},
	{.status = 1443, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid GW_* command.)"},
	{.status = 1444, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid thread identifier.)"},
	{.status = 1445, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot process a message from a window that is not a multiple document interface (MDI) window.)"},
	{.status = 1446, .dos_err = 317, .posix_err = 0, .msg = R"(Popup menu already active.)"},
	{.status = 1447, .dos_err = 317, .posix_err = 0, .msg = R"(The window does not have scroll bars.)"},
	{.status = 1448, .dos_err = 317, .posix_err = 0, .msg = R"(Scroll bar range cannot be greater than MAXLONG.)"},
	{.status = 1449, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot show or remove the window in the way specified.)"},
	{.status = 1450, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient system resources exist to complete the requested service.)"},
	{.status = 1451, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient system resources exist to complete the requested service.)"},
	{.status = 1452, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient system resources exist to complete the requested service.)"},
	{.status = 1453, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient quota to complete the requested service.)"},
	{.status = 1454, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient quota to complete the requested service.)"},
	{.status = 1455, .dos_err = 317, .posix_err = 0, .msg = R"(The paging file is too small for this operation to complete.)"},
	{.status = 1456, .dos_err = 317, .posix_err = 0, .msg = R"(A menu item was not found.)"},
	{.status = 1457, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid keyboard layout handle.)"},
	{.status = 1458, .dos_err = 317, .posix_err = 0, .msg = R"(Hook type not allowed.)"},
	{.status = 1459, .dos_err = 317, .posix_err = 0, .msg = R"(This operation requires an interactive window station.)"},
	{.status = 1460, .dos_err = 317, .posix_err = 0, .msg = R"(This operation returned because the timeout period expired.)"},
	{.status = 1461, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid monitor handle.)"},
	{.status = 1462, .dos_err = 317, .posix_err = 0, .msg = R"(Incorrect size argument.)"},
	{.status = 1463, .dos_err = 317, .posix_err = 0, .msg = R"(The symbolic link cannot be followed because its type is disabled.)"},
	{.status = 1464, .dos_err = 317, .posix_err = 0, .msg = R"(This application does not support the current operation on symbolic links.)"},
	{.status = 1465, .dos_err = 317, .posix_err = 0, .msg = R"(Windows was unable to parse the requested XML data.)"},
	{.status = 1466, .dos_err = 317, .posix_err = 0, .msg = R"(An error was encountered while processing an XML digital signature.)"},
	{.status = 1467, .dos_err = 317, .posix_err = 0, .msg = R"(This application must be restarted.)"},
	{.status = 1468, .dos_err = 317, .posix_err = 0, .msg = R"(The caller made the connection request in the wrong routing compartment.)"},
	{.status = 1469, .dos_err = 317, .posix_err = 0, .msg = R"(There was an AuthIP failure when attempting to connect to the remote host.)"},
	{.status = 1470, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient NVRAM resources exist to complete the requested service. A reboot might be required.)"},
	{.status = 1471, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to finish the requested operation because the specified process is not a GUI process.)"},
	{.status = 1500, .dos_err = 317, .posix_err = 0, .msg = R"(The event log file is corrupted.)"},
	{.status = 1501, .dos_err = 317, .posix_err = 0, .msg = R"(No event log file could be opened, so the event logging service did not start.)"},
	{.status = 1502, .dos_err = 317, .posix_err = 0, .msg = R"(The event log file is full.)"},
	{.status = 1503, .dos_err = 317, .posix_err = 0, .msg = R"(The event log file has changed between read operations.)"},
	{.status = 1504, .dos_err = 317, .posix_err = 0, .msg = R"(The specified Job already has a container assigned to it.)"},
	{.status = 1505, .dos_err = 317, .posix_err = 0, .msg = R"(The specified Job does not have a container assigned to it.)"},
	{.status = 1550, .dos_err = 317, .posix_err = 0, .msg = R"(The specified task name is invalid.)"},
	{.status = 1551, .dos_err = 317, .posix_err = 0, .msg = R"(The specified task index is invalid.)"},
	{.status = 1552, .dos_err = 317, .posix_err = 0, .msg = R"(The specified thread is already joining a task.)"},
	{.status = 1601, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows Installer Service could not be accessed. This can occur if the Windows Installer is not correctly installed. Contact your support personnel for assistance.)"},
	{.status = 1602, .dos_err = 317, .posix_err = 0, .msg = R"(User cancelled installation.)"},
	{.status = 1603, .dos_err = 317, .posix_err = 0, .msg = R"(Fatal error during installation.)"},
	{.status = 1604, .dos_err = 317, .posix_err = 0, .msg = R"(Installation suspended, incomplete.)"},
	{.status = 1605, .dos_err = 317, .posix_err = 0, .msg = R"(This action is only valid for products that are currently installed.)"},
	{.status = 1606, .dos_err = 317, .posix_err = 0, .msg = R"(Feature ID not registered.)"},
	{.status = 1607, .dos_err = 317, .posix_err = 0, .msg = R"(Component ID not registered.)"},
	{.status = 1608, .dos_err = 317, .posix_err = 0, .msg = R"(Unknown property.)"},
	{.status = 1609, .dos_err = 317, .posix_err = 0, .msg = R"(Handle is in an invalid state.)"},
	{.status = 1610, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration data for this product is corrupt. Contact your support personnel.)"},
	{.status = 1611, .dos_err = 317, .posix_err = 0, .msg = R"(Component qualifier not present.)"},
	{.status = 1612, .dos_err = 317, .posix_err = 0, .msg = R"(The installation source for this product is not available. Verify that the source exists and that you can access it.)"},
	{.status = 1613, .dos_err = 317, .posix_err = 0, .msg = R"(This installation package cannot be installed by the Windows Installer service. You must install a Windows service pack that contains a newer version of the Windows Installer service.)"},
	{.status = 1614, .dos_err = 317, .posix_err = 0, .msg = R"(Product is uninstalled.)"},
	{.status = 1615, .dos_err = 317, .posix_err = 0, .msg = R"(SQL query syntax invalid or unsupported.)"},
	{.status = 1616, .dos_err = 317, .posix_err = 0, .msg = R"(Record field does not exist.)"},
	{.status = 1617, .dos_err = 317, .posix_err = 0, .msg = R"(The device has been removed.)"},
	{.status = 1618, .dos_err = 317, .posix_err = 0, .msg = R"(Another installation is already in progress. Complete that installation before proceeding with this install.)"},
	{.status = 1619, .dos_err = 317, .posix_err = 0, .msg = R"(This installation package could not be opened. Verify that the package exists and that you can access it, or contact the application vendor to verify that this is a valid Windows Installer package.)"},
	{.status = 1620, .dos_err = 317, .posix_err = 0, .msg = R"(This installation package could not be opened. Contact the application vendor to verify that this is a valid Windows Installer package.)"},
	{.status = 1621, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error starting the Windows Installer service user interface. Contact your support personnel.)"},
	{.status = 1622, .dos_err = 317, .posix_err = 0, .msg = R"(Error opening installation log file. Verify that the specified log file location exists and that you can write to it.)"},
	{.status = 1623, .dos_err = 317, .posix_err = 0, .msg = R"(The language of this installation package is not supported by your system.)"},
	{.status = 1624, .dos_err = 317, .posix_err = 0, .msg = R"(Error applying transforms. Verify that the specified transform paths are valid.)"},
	{.status = 1625, .dos_err = 317, .posix_err = 0, .msg = R"(This installation is forbidden by system policy. Contact your system administrator.)"},
	{.status = 1626, .dos_err = 317, .posix_err = 0, .msg = R"(Function could not be executed.)"},
	{.status = 1627, .dos_err = 317, .posix_err = 0, .msg = R"(Function failed during execution.)"},
	{.status = 1628, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid or unknown table specified.)"},
	{.status = 1629, .dos_err = 317, .posix_err = 0, .msg = R"(Data supplied is of wrong type.)"},
	{.status = 1630, .dos_err = 317, .posix_err = 0, .msg = R"(Data of this type is not supported.)"},
	{.status = 1631, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows Installer service failed to start. Contact your support personnel.)"},
	{.status = 1632, .dos_err = 317, .posix_err = 0, .msg = R"(The Temp folder is on a drive that is full or is inaccessible. Free up space on the drive or verify that you have write permission on the Temp folder.)"},
	{.status = 1633, .dos_err = 317, .posix_err = 0, .msg = R"(This installation package is not supported by this processor type. Contact your product vendor.)"},
	{.status = 1634, .dos_err = 317, .posix_err = 0, .msg = R"(Component not used on this computer.)"},
	{.status = 1635, .dos_err = 317, .posix_err = 0, .msg = R"(This update package could not be opened. Verify that the update package exists and that you can access it, or contact the application vendor to verify that this is a valid Windows Installer update package.)"},
	{.status = 1636, .dos_err = 317, .posix_err = 0, .msg = R"(This update package could not be opened. Contact the application vendor to verify that this is a valid Windows Installer update package.)"},
	{.status = 1637, .dos_err = 317, .posix_err = 0, .msg = R"(This update package cannot be processed by the Windows Installer service. You must install a Windows service pack that contains a newer version of the Windows Installer service.)"},
	{.status = 1638, .dos_err = 317, .posix_err = 0, .msg = R"(Another version of this product is already installed. Installation of this version cannot continue. To configure or remove the existing version of this product, use Add/Remove Programs on the Control Panel.)"},
	{.status = 1639, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid command line argument. Consult the Windows Installer SDK for detailed command line help.)"},
	{.status = 1640, .dos_err = 317, .posix_err = 0, .msg = R"(Only administrators have permission to add, remove, or configure server software during a Terminal services remote session. If you want to install or configure software on the server, contact your network administrator.)"},
	{.status = 1641, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation completed successfully. The system will be restarted so the changes can take effect.)"},
	{.status = 1642, .dos_err = 317, .posix_err = 0, .msg = R"(The upgrade cannot be installed by the Windows Installer service because the program to be upgraded may be missing, or the upgrade may update a different version of the program. Verify that the program to be upgraded exists on your computer and that you have the correct upgrade.)"},
	{.status = 1643, .dos_err = 317, .posix_err = 0, .msg = R"(The update package is not permitted by software restriction policy.)"},
	{.status = 1644, .dos_err = 317, .posix_err = 0, .msg = R"(One or more customizations are not permitted by software restriction policy.)"},
	{.status = 1645, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows Installer does not permit installation from a Remote Desktop Connection.)"},
	{.status = 1646, .dos_err = 317, .posix_err = 0, .msg = R"(Uninstallation of the update package is not supported.)"},
	{.status = 1647, .dos_err = 317, .posix_err = 0, .msg = R"(The update is not applied to this product.)"},
	{.status = 1648, .dos_err = 317, .posix_err = 0, .msg = R"(No valid sequence could be found for the set of updates.)"},
	{.status = 1649, .dos_err = 317, .posix_err = 0, .msg = R"(Update removal was disallowed by policy.)"},
	{.status = 1650, .dos_err = 317, .posix_err = 0, .msg = R"(The XML update data is invalid.)"},
	{.status = 1651, .dos_err = 317, .posix_err = 0, .msg = R"(Windows Installer does not permit updating of managed advertised products. At least one feature of the product must be installed before applying the update.)"},
	{.status = 1652, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows Installer service is not accessible in Safe Mode. Please try again when your computer is not in Safe Mode or you can use System Restore to return your machine to a previous good state.)"},
	{.status = 1653, .dos_err = 317, .posix_err = 0, .msg = R"(A fail fast exception occurred. Exception handlers will not be invoked and the process will be terminated immediately.)"},
	{.status = 1654, .dos_err = 317, .posix_err = 0, .msg = R"(The app that you are trying to run is not supported on this version of Windows.)"},
	{.status = 1655, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was blocked as the process prohibits dynamic code generation.)"},
	{.status = 1656, .dos_err = 317, .posix_err = 0, .msg = R"(The objects are not identical.)"},
	{.status = 1657, .dos_err = 317, .posix_err = 0, .msg = R"(The specified image file was blocked from loading because it does not enable a feature required by the process: Control Flow Guard.)"},
	{.status = 1660, .dos_err = 317, .posix_err = 0, .msg = R"(The thread context could not be updated because this has been restricted for the process.)"},
	{.status = 1661, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid cross-partition private file/section access was attempted.)"},
	{.status = 1662, .dos_err = 317, .posix_err = 0, .msg = R"(A return address hijack is being attempted. This is supported by the operating system when user-mode shadow stacks are enabled.)"},
	{.status = 1700, .dos_err = 317, .posix_err = 0, .msg = R"(The string binding is invalid.)"},
	{.status = 1701, .dos_err = 317, .posix_err = 0, .msg = R"(The binding handle is not the correct type.)"},
	{.status = 1702, .dos_err = 317, .posix_err = 0, .msg = R"(The binding handle is invalid.)"},
	{.status = 1703, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC protocol sequence is not supported.)"},
	{.status = 1704, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC protocol sequence is invalid.)"},
	{.status = 1705, .dos_err = 317, .posix_err = 0, .msg = R"(The string universal unique identifier (UUID) is invalid.)"},
	{.status = 1706, .dos_err = 317, .posix_err = 0, .msg = R"(The endpoint format is invalid.)"},
	{.status = 1707, .dos_err = 317, .posix_err = 0, .msg = R"(The network address is invalid.)"},
	{.status = 1708, .dos_err = 317, .posix_err = 0, .msg = R"(No endpoint was found.)"},
	{.status = 1709, .dos_err = 317, .posix_err = 0, .msg = R"(The timeout value is invalid.)"},
	{.status = 1710, .dos_err = 317, .posix_err = 0, .msg = R"(The object universal unique identifier (UUID) was not found.)"},
	{.status = 1711, .dos_err = 317, .posix_err = 0, .msg = R"(The object universal unique identifier (UUID) has already been registered.)"},
	{.status = 1712, .dos_err = 317, .posix_err = 0, .msg = R"(The type universal unique identifier (UUID) has already been registered.)"},
	{.status = 1713, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC server is already listening.)"},
	{.status = 1714, .dos_err = 317, .posix_err = 0, .msg = R"(No protocol sequences have been registered.)"},
	{.status = 1715, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC server is not listening.)"},
	{.status = 1716, .dos_err = 317, .posix_err = 0, .msg = R"(The manager type is unknown.)"},
	{.status = 1717, .dos_err = 317, .posix_err = 0, .msg = R"(The interface is unknown.)"},
	{.status = 1718, .dos_err = 317, .posix_err = 0, .msg = R"(There are no bindings.)"},
	{.status = 1719, .dos_err = 317, .posix_err = 0, .msg = R"(There are no protocol sequences.)"},
	{.status = 1720, .dos_err = 317, .posix_err = 0, .msg = R"(The endpoint cannot be created.)"},
	{.status = 1721, .dos_err = 317, .posix_err = 0, .msg = R"(Not enough resources are available to complete this operation.)"},
	{.status = 1722, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC server is unavailable.)"},
	{.status = 1723, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC server is too busy to complete this operation.)"},
	{.status = 1724, .dos_err = 317, .posix_err = 0, .msg = R"(The network options are invalid.)"},
	{.status = 1725, .dos_err = 317, .posix_err = 0, .msg = R"(There are no remote procedure calls active on this thread.)"},
	{.status = 1726, .dos_err = 317, .posix_err = 0, .msg = R"(The remote procedure call failed.)"},
	{.status = 1727, .dos_err = 317, .posix_err = 0, .msg = R"(The remote procedure call failed and did not execute.)"},
	{.status = 1728, .dos_err = 317, .posix_err = 0, .msg = R"(A remote procedure call (RPC) protocol error occurred.)"},
	{.status = 1729, .dos_err = 317, .posix_err = 0, .msg = R"(Access to the HTTP proxy is denied.)"},
	{.status = 1730, .dos_err = 317, .posix_err = 0, .msg = R"(The transfer syntax is not supported by the RPC server.)"},
	{.status = 1732, .dos_err = 317, .posix_err = 0, .msg = R"(The universal unique identifier (UUID) type is not supported.)"},
	{.status = 1733, .dos_err = 317, .posix_err = 0, .msg = R"(The tag is invalid.)"},
	{.status = 1734, .dos_err = 317, .posix_err = 0, .msg = R"(The array bounds are invalid.)"},
	{.status = 1735, .dos_err = 317, .posix_err = 0, .msg = R"(The binding does not contain an entry name.)"},
	{.status = 1736, .dos_err = 317, .posix_err = 0, .msg = R"(The name syntax is invalid.)"},
	{.status = 1737, .dos_err = 317, .posix_err = 0, .msg = R"(The name syntax is not supported.)"},
	{.status = 1739, .dos_err = 317, .posix_err = 0, .msg = R"(No network address is available to use to construct a universal unique identifier (UUID).)"},
	{.status = 1740, .dos_err = 317, .posix_err = 0, .msg = R"(The endpoint is a duplicate.)"},
	{.status = 1741, .dos_err = 317, .posix_err = 0, .msg = R"(The authentication type is unknown.)"},
	{.status = 1742, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of calls is too small.)"},
	{.status = 1743, .dos_err = 317, .posix_err = 0, .msg = R"(The string is too long.)"},
	{.status = 1744, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC protocol sequence was not found.)"},
	{.status = 1745, .dos_err = 317, .posix_err = 0, .msg = R"(The procedure number is out of range.)"},
	{.status = 1746, .dos_err = 317, .posix_err = 0, .msg = R"(The binding does not contain any authentication information.)"},
	{.status = 1747, .dos_err = 317, .posix_err = 0, .msg = R"(The authentication service is unknown.)"},
	{.status = 1748, .dos_err = 317, .posix_err = 0, .msg = R"(The authentication level is unknown.)"},
	{.status = 1749, .dos_err = 317, .posix_err = 0, .msg = R"(The security context is invalid.)"},
	{.status = 1750, .dos_err = 317, .posix_err = 0, .msg = R"(The authorization service is unknown.)"},
	{.status = 1751, .dos_err = 317, .posix_err = 0, .msg = R"(The entry is invalid.)"},
	{.status = 1752, .dos_err = 317, .posix_err = 0, .msg = R"(The server endpoint cannot perform the operation.)"},
	{.status = 1753, .dos_err = 317, .posix_err = 0, .msg = R"(There are no more endpoints available from the endpoint mapper.)"},
	{.status = 1754, .dos_err = 317, .posix_err = 0, .msg = R"(No interfaces have been exported.)"},
	{.status = 1755, .dos_err = 317, .posix_err = 0, .msg = R"(The entry name is incomplete.)"},
	{.status = 1756, .dos_err = 317, .posix_err = 0, .msg = R"(The version option is invalid.)"},
	{.status = 1757, .dos_err = 317, .posix_err = 0, .msg = R"(There are no more members.)"},
	{.status = 1758, .dos_err = 317, .posix_err = 0, .msg = R"(There is nothing to unexport.)"},
	{.status = 1759, .dos_err = 317, .posix_err = 0, .msg = R"(The interface was not found.)"},
	{.status = 1760, .dos_err = 317, .posix_err = 0, .msg = R"(The entry already exists.)"},
	{.status = 1761, .dos_err = 317, .posix_err = 0, .msg = R"(The entry is not found.)"},
	{.status = 1762, .dos_err = 317, .posix_err = 0, .msg = R"(The name service is unavailable.)"},
	{.status = 1763, .dos_err = 317, .posix_err = 0, .msg = R"(The network address family is invalid.)"},
	{.status = 1764, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is not supported.)"},
	{.status = 1765, .dos_err = 317, .posix_err = 0, .msg = R"(No security context is available to allow impersonation.)"},
	{.status = 1766, .dos_err = 317, .posix_err = 0, .msg = R"(An internal error occurred in a remote procedure call (RPC).)"},
	{.status = 1767, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC server attempted an integer division by zero.)"},
	{.status = 1768, .dos_err = 317, .posix_err = 0, .msg = R"(An addressing error occurred in the RPC server.)"},
	{.status = 1769, .dos_err = 317, .posix_err = 0, .msg = R"(A floating-point operation at the RPC server caused a division by zero.)"},
	{.status = 1770, .dos_err = 317, .posix_err = 0, .msg = R"(A floating-point underflow occurred at the RPC server.)"},
	{.status = 1771, .dos_err = 317, .posix_err = 0, .msg = R"(A floating-point overflow occurred at the RPC server.)"},
	{.status = 1772, .dos_err = 317, .posix_err = 0, .msg = R"(The list of RPC servers available for the binding of auto handles has been exhausted.)"},
	{.status = 1773, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to open the character translation table file.)"},
	{.status = 1774, .dos_err = 317, .posix_err = 0, .msg = R"(The file containing the character translation table has fewer than 512 bytes.)"},
	{.status = 1775, .dos_err = 317, .posix_err = 0, .msg = R"(A null context handle was passed from the client to the host during a remote procedure call.)"},
	{.status = 1777, .dos_err = 317, .posix_err = 0, .msg = R"(The context handle changed during a remote procedure call.)"},
	{.status = 1778, .dos_err = 317, .posix_err = 0, .msg = R"(The binding handles passed to a remote procedure call do not match.)"},
	{.status = 1779, .dos_err = 317, .posix_err = 0, .msg = R"(The stub is unable to get the remote procedure call handle.)"},
	{.status = 1780, .dos_err = 317, .posix_err = 0, .msg = R"(A null reference pointer was passed to the stub.)"},
	{.status = 1781, .dos_err = 317, .posix_err = 0, .msg = R"(The enumeration value is out of range.)"},
	{.status = 1782, .dos_err = 317, .posix_err = 0, .msg = R"(The byte count is too small.)"},
	{.status = 1783, .dos_err = 317, .posix_err = 0, .msg = R"(The stub received bad data.)"},
	{.status = 1784, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied user buffer is not valid for the requested operation.)"},
	{.status = 1785, .dos_err = 317, .posix_err = 0, .msg = R"(The disk media is not recognized. It may not be formatted.)"},
	{.status = 1786, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation does not have a trust secret.)"},
	{.status = 1787, .dos_err = 317, .posix_err = 0, .msg = R"(The security database on the server does not have a computer account for this workstation trust relationship.)"},
	{.status = 1788, .dos_err = 317, .posix_err = 0, .msg = R"(The trust relationship between the primary domain and the trusted domain failed.)"},
	{.status = 1789, .dos_err = 317, .posix_err = 0, .msg = R"(The trust relationship between this workstation and the primary domain failed.)"},
	{.status = 1790, .dos_err = 317, .posix_err = 0, .msg = R"(The network logon failed.)"},
	{.status = 1791, .dos_err = 317, .posix_err = 0, .msg = R"(A remote procedure call is already in progress for this thread.)"},
	{.status = 1792, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to logon, but the network logon service was not started.)"},
	{.status = 1793, .dos_err = 317, .posix_err = 0, .msg = R"(The user's account has expired.)"},
	{.status = 1794, .dos_err = 317, .posix_err = 0, .msg = R"(The redirector is in use and cannot be unloaded.)"},
	{.status = 1795, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer driver is already installed.)"},
	{.status = 1796, .dos_err = 317, .posix_err = 0, .msg = R"(The specified port is unknown.)"},
	{.status = 1797, .dos_err = 317, .posix_err = 0, .msg = R"(The printer driver is unknown.)"},
	{.status = 1798, .dos_err = 317, .posix_err = 0, .msg = R"(The print processor is unknown.)"},
	{.status = 1799, .dos_err = 317, .posix_err = 0, .msg = R"(The specified separator file is invalid.)"},
	{.status = 1800, .dos_err = 317, .posix_err = 0, .msg = R"(The specified priority is invalid.)"},
	{.status = 1801, .dos_err = 317, .posix_err = 0, .msg = R"(The printer name is invalid.)"},
	{.status = 1802, .dos_err = 317, .posix_err = 0, .msg = R"(The printer already exists.)"},
	{.status = 1803, .dos_err = 317, .posix_err = 0, .msg = R"(The printer command is invalid.)"},
	{.status = 1804, .dos_err = 317, .posix_err = 0, .msg = R"(The specified datatype is invalid.)"},
	{.status = 1805, .dos_err = 317, .posix_err = 0, .msg = R"(The environment specified is invalid.)"},
	{.status = 1806, .dos_err = 317, .posix_err = 0, .msg = R"(There are no more bindings.)"},
	{.status = 1807, .dos_err = 317, .posix_err = 0, .msg = R"(The account used is an interdomain trust account. Use your global user account or local user account to access this server.)"},
	{.status = 1808, .dos_err = 317, .posix_err = 0, .msg = R"(The account used is a computer account. Use your global user account or local user account to access this server.)"},
	{.status = 1809, .dos_err = 317, .posix_err = 0, .msg = R"(The account used is a server trust account. Use your global user account or local user account to access this server.)"},
	{.status = 1810, .dos_err = 317, .posix_err = 0, .msg = R"(The name or security ID (SID) of the domain specified is inconsistent with the trust information for that domain.)"},
	{.status = 1811, .dos_err = 317, .posix_err = 0, .msg = R"(The server is in use and cannot be unloaded.)"},
	{.status = 1812, .dos_err = 317, .posix_err = 0, .msg = R"(The specified image file did not contain a resource section.)"},
	{.status = 1813, .dos_err = 317, .posix_err = 0, .msg = R"(The specified resource type cannot be found in the image file.)"},
	{.status = 1814, .dos_err = 317, .posix_err = 0, .msg = R"(The specified resource name cannot be found in the image file.)"},
	{.status = 1815, .dos_err = 317, .posix_err = 0, .msg = R"(The specified resource language ID cannot be found in the image file.)"},
	{.status = 1816, .dos_err = 317, .posix_err = 0, .msg = R"(Not enough quota is available to process this command.)"},
	{.status = 1817, .dos_err = 317, .posix_err = 0, .msg = R"(No interfaces have been registered.)"},
	{.status = 1818, .dos_err = 317, .posix_err = 0, .msg = R"(The remote procedure call was cancelled.)"},
	{.status = 1819, .dos_err = 317, .posix_err = 0, .msg = R"(The binding handle does not contain all required information.)"},
	{.status = 1820, .dos_err = 317, .posix_err = 0, .msg = R"(A communications failure occurred during a remote procedure call.)"},
	{.status = 1821, .dos_err = 317, .posix_err = 0, .msg = R"(The requested authentication level is not supported.)"},
	{.status = 1822, .dos_err = 317, .posix_err = 0, .msg = R"(No principal name registered.)"},
	{.status = 1823, .dos_err = 317, .posix_err = 0, .msg = R"(The error specified is not a valid Windows RPC error code.)"},
	{.status = 1824, .dos_err = 317, .posix_err = 0, .msg = R"(A UUID that is valid only on this computer has been allocated.)"},
	{.status = 1825, .dos_err = 317, .posix_err = 0, .msg = R"(A security package specific error occurred.)"},
	{.status = 1826, .dos_err = 317, .posix_err = 0, .msg = R"(Thread is not canceled.)"},
	{.status = 1827, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid operation on the encoding/decoding handle.)"},
	{.status = 1828, .dos_err = 317, .posix_err = 0, .msg = R"(Incompatible version of the serializing package.)"},
	{.status = 1829, .dos_err = 317, .posix_err = 0, .msg = R"(Incompatible version of the RPC stub.)"},
	{.status = 1830, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC pipe object is invalid or corrupted.)"},
	{.status = 1831, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid operation was attempted on an RPC pipe object.)"},
	{.status = 1832, .dos_err = 317, .posix_err = 0, .msg = R"(Unsupported RPC pipe version.)"},
	{.status = 1833, .dos_err = 317, .posix_err = 0, .msg = R"(HTTP proxy server rejected the connection because the cookie authentication failed.)"},
	{.status = 1834, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC server is suspended, and could not be resumed for this request. The call did not execute.)"},
	{.status = 1835, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC call contains too many handles to be transmitted in a single request.)"},
	{.status = 1836, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC call contains a handle that differs from the declared handle type.)"},
	{.status = 1898, .dos_err = 317, .posix_err = 0, .msg = R"(The group member was not found.)"},
	{.status = 1899, .dos_err = 317, .posix_err = 0, .msg = R"(The endpoint mapper database entry could not be created.)"},
	{.status = 1900, .dos_err = 317, .posix_err = 0, .msg = R"(The object universal unique identifier (UUID) is the nil UUID.)"},
	{.status = 1901, .dos_err = 317, .posix_err = 0, .msg = R"(The specified time is invalid.)"},
	{.status = 1902, .dos_err = 317, .posix_err = 0, .msg = R"(The specified form name is invalid.)"},
	{.status = 1903, .dos_err = 317, .posix_err = 0, .msg = R"(The specified form size is invalid.)"},
	{.status = 1904, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer handle is already being waited on)"},
	{.status = 1905, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer has been deleted.)"},
	{.status = 1906, .dos_err = 317, .posix_err = 0, .msg = R"(The state of the printer is invalid.)"},
	{.status = 1907, .dos_err = 317, .posix_err = 0, .msg = R"(The user's password must be changed before signing in.)"},
	{.status = 1908, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find the domain controller for this domain.)"},
	{.status = 1909, .dos_err = 317, .posix_err = 0, .msg = R"(The referenced account is currently locked out and may not be logged on to.)"},
	{.status = 1910, .dos_err = 317, .posix_err = 0, .msg = R"(The object exporter specified was not found.)"},
	{.status = 1911, .dos_err = 317, .posix_err = 0, .msg = R"(The object specified was not found.)"},
	{.status = 1912, .dos_err = 317, .posix_err = 0, .msg = R"(The object resolver set specified was not found.)"},
	{.status = 1913, .dos_err = 317, .posix_err = 0, .msg = R"(Some data remains to be sent in the request buffer.)"},
	{.status = 1914, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid asynchronous remote procedure call handle.)"},
	{.status = 1915, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid asynchronous RPC call handle for this operation.)"},
	{.status = 1916, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC pipe object has already been closed.)"},
	{.status = 1917, .dos_err = 317, .posix_err = 0, .msg = R"(The RPC call completed before all pipes were processed.)"},
	{.status = 1918, .dos_err = 317, .posix_err = 0, .msg = R"(No more data is available from the RPC pipe.)"},
	{.status = 1919, .dos_err = 317, .posix_err = 0, .msg = R"(No site name is available for this machine.)"},
	{.status = 1920, .dos_err = 317, .posix_err = 0, .msg = R"(The file cannot be accessed by the system.)"},
	{.status = 1921, .dos_err = 317, .posix_err = 0, .msg = R"(The name of the file cannot be resolved by the system.)"},
	{.status = 1922, .dos_err = 317, .posix_err = 0, .msg = R"(The entry is not of the expected type.)"},
	{.status = 1923, .dos_err = 317, .posix_err = 0, .msg = R"(Not all object UUIDs could be exported to the specified entry.)"},
	{.status = 1924, .dos_err = 317, .posix_err = 0, .msg = R"(Interface could not be exported to the specified entry.)"},
	{.status = 1925, .dos_err = 317, .posix_err = 0, .msg = R"(The specified profile entry could not be added.)"},
	{.status = 1926, .dos_err = 317, .posix_err = 0, .msg = R"(The specified profile element could not be added.)"},
	{.status = 1927, .dos_err = 317, .posix_err = 0, .msg = R"(The specified profile element could not be removed.)"},
	{.status = 1928, .dos_err = 317, .posix_err = 0, .msg = R"(The group element could not be added.)"},
	{.status = 1929, .dos_err = 317, .posix_err = 0, .msg = R"(The group element could not be removed.)"},
	{.status = 1930, .dos_err = 317, .posix_err = 0, .msg = R"(The printer driver is not compatible with a policy enabled on your computer that blocks NT 4.0 drivers.)"},
	{.status = 1931, .dos_err = 317, .posix_err = 0, .msg = R"(The context has expired and can no longer be used.)"},
	{.status = 1932, .dos_err = 317, .posix_err = 0, .msg = R"(The current user's delegated trust creation quota has been exceeded.)"},
	{.status = 1933, .dos_err = 317, .posix_err = 0, .msg = R"(The total delegated trust creation quota has been exceeded.)"},
	{.status = 1934, .dos_err = 317, .posix_err = 0, .msg = R"(The current user's delegated trust deletion quota has been exceeded.)"},
	{.status = 1935, .dos_err = 317, .posix_err = 0, .msg = R"(The computer you are signing into is protected by an authentication firewall. The specified account is not allowed to authenticate to the computer.)"},
	{.status = 1936, .dos_err = 317, .posix_err = 0, .msg = R"(Remote connections to the Print Spooler are blocked by a policy set on your machine.)"},
	{.status = 1937, .dos_err = 317, .posix_err = 0, .msg = R"(Authentication failed because NTLM authentication has been disabled.)"},
	{.status = 1938, .dos_err = 317, .posix_err = 0, .msg = R"(Logon Failure: EAS policy requires that the user change their password before this operation can be performed.)"},
	{.status = 1939, .dos_err = 317, .posix_err = 0, .msg = R"(An administrator has restricted sign in. To sign in, make sure your device is connected to the Internet, and have your administrator sign in first.)"},
	{.status = 2000, .dos_err = 317, .posix_err = 0, .msg = R"(The pixel format is invalid.)"},
	{.status = 2001, .dos_err = 317, .posix_err = 0, .msg = R"(The specified driver is invalid.)"},
	{.status = 2002, .dos_err = 317, .posix_err = 0, .msg = R"(The window style or class attribute is invalid for this operation.)"},
	{.status = 2003, .dos_err = 317, .posix_err = 0, .msg = R"(The requested metafile operation is not supported.)"},
	{.status = 2004, .dos_err = 317, .posix_err = 0, .msg = R"(The requested transformation operation is not supported.)"},
	{.status = 2005, .dos_err = 317, .posix_err = 0, .msg = R"(The requested clipping operation is not supported.)"},
	{.status = 2010, .dos_err = 317, .posix_err = 0, .msg = R"(The specified color management module is invalid.)"},
	{.status = 2011, .dos_err = 317, .posix_err = 0, .msg = R"(The specified color profile is invalid.)"},
	{.status = 2012, .dos_err = 317, .posix_err = 0, .msg = R"(The specified tag was not found.)"},
	{.status = 2013, .dos_err = 317, .posix_err = 0, .msg = R"(A required tag is not present.)"},
	{.status = 2014, .dos_err = 317, .posix_err = 0, .msg = R"(The specified tag is already present.)"},
	{.status = 2015, .dos_err = 317, .posix_err = 0, .msg = R"(The specified color profile is not associated with the specified device.)"},
	{.status = 2016, .dos_err = 317, .posix_err = 0, .msg = R"(The specified color profile was not found.)"},
	{.status = 2017, .dos_err = 317, .posix_err = 0, .msg = R"(The specified color space is invalid.)"},
	{.status = 2018, .dos_err = 317, .posix_err = 0, .msg = R"(Image Color Management is not enabled.)"},
	{.status = 2019, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error while deleting the color transform.)"},
	{.status = 2020, .dos_err = 317, .posix_err = 0, .msg = R"(The specified color transform is invalid.)"},
	{.status = 2021, .dos_err = 317, .posix_err = 0, .msg = R"(The specified transform does not match the bitmap's color space.)"},
	{.status = 2022, .dos_err = 317, .posix_err = 0, .msg = R"(The specified named color index is not present in the profile.)"},
	{.status = 2023, .dos_err = 317, .posix_err = 0, .msg = R"(The specified profile is intended for a device of a different type than the specified device.)"},
	{.status = 2102, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation driver is not installed.)"},
	{.status = 2103, .dos_err = 317, .posix_err = 0, .msg = R"(The server could not be located.)"},
	{.status = 2104, .dos_err = 317, .posix_err = 0, .msg = R"(An internal error occurred.  The network cannot access a shared memory segment.)"},
	{.status = 2105, .dos_err = 317, .posix_err = 0, .msg = R"(A network resource shortage occurred .)"},
	{.status = 2106, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not supported on workstations.)"},
	{.status = 2107, .dos_err = 317, .posix_err = 0, .msg = R"(The device is not connected.)"},
	{.status = 2108, .dos_err = 317, .posix_err = 0, .msg = R"(The network connection was made successfully, but the user had to be prompted for a password other than the one originally specified.)"},
	{.status = 2109, .dos_err = 317, .posix_err = 0, .msg = R"(The network connection was made successfully using default credentials.)"},
	{.status = 2114, .dos_err = 317, .posix_err = 0, .msg = R"(The Server service is not started.)"},
	{.status = 2115, .dos_err = 317, .posix_err = 0, .msg = R"(The queue is empty.)"},
	{.status = 2116, .dos_err = 317, .posix_err = 0, .msg = R"(The device or directory does not exist.)"},
	{.status = 2117, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is invalid on a redirected resource.)"},
	{.status = 2118, .dos_err = 317, .posix_err = 0, .msg = R"(The name has already been shared.)"},
	{.status = 2119, .dos_err = 317, .posix_err = 0, .msg = R"(The server is currently out of the requested resource.)"},
	{.status = 2121, .dos_err = 317, .posix_err = 0, .msg = R"(Requested addition of items exceeds the maximum allowed.)"},
	{.status = 2122, .dos_err = 317, .posix_err = 0, .msg = R"(The Peer service supports only two simultaneous users.)"},
	{.status = 2123, .dos_err = 317, .posix_err = 0, .msg = R"(The API return buffer is too small.)"},
	{.status = 2127, .dos_err = 317, .posix_err = 0, .msg = R"(A remote API error occurred.)"},
	{.status = 2131, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred when opening or reading the configuration file.)"},
	{.status = 2136, .dos_err = 317, .posix_err = 0, .msg = R"(A general network error occurred.)"},
	{.status = 2137, .dos_err = 317, .posix_err = 0, .msg = R"(The Workstation service is in an inconsistent state. Restart the computer before restarting the Workstation service.)"},
	{.status = 2138, .dos_err = 317, .posix_err = 0, .msg = R"(The Workstation service has not been started.)"},
	{.status = 2139, .dos_err = 317, .posix_err = 0, .msg = R"(The requested information is not available.)"},
	{.status = 2140, .dos_err = 317, .posix_err = 0, .msg = R"(An internal Windows error occurred.)"},
	{.status = 2141, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not configured for transactions.)"},
	{.status = 2142, .dos_err = 317, .posix_err = 0, .msg = R"(The requested API is not supported on the remote server.)"},
	{.status = 2143, .dos_err = 317, .posix_err = 0, .msg = R"(The event name is invalid.)"},
	{.status = 2144, .dos_err = 317, .posix_err = 0, .msg = R"(The computer name already exists on the network. Change it and restart the computer.)"},
	{.status = 2146, .dos_err = 317, .posix_err = 0, .msg = R"(The specified component could not be found in the configuration information.)"},
	{.status = 2147, .dos_err = 317, .posix_err = 0, .msg = R"(The specified parameter could not be found in the configuration information.)"},
	{.status = 2149, .dos_err = 317, .posix_err = 0, .msg = R"(A line in the configuration file is too long.)"},
	{.status = 2150, .dos_err = 317, .posix_err = 0, .msg = R"(The printer does not exist.)"},
	{.status = 2151, .dos_err = 317, .posix_err = 0, .msg = R"(The print job does not exist.)"},
	{.status = 2152, .dos_err = 317, .posix_err = 0, .msg = R"(The printer destination cannot be found.)"},
	{.status = 2153, .dos_err = 317, .posix_err = 0, .msg = R"(The printer destination already exists.)"},
	{.status = 2154, .dos_err = 317, .posix_err = 0, .msg = R"(The printer queue already exists.)"},
	{.status = 2155, .dos_err = 317, .posix_err = 0, .msg = R"(No more printers can be added.)"},
	{.status = 2156, .dos_err = 317, .posix_err = 0, .msg = R"(No more print jobs can be added.)"},
	{.status = 2157, .dos_err = 317, .posix_err = 0, .msg = R"(No more printer destinations can be added.)"},
	{.status = 2158, .dos_err = 317, .posix_err = 0, .msg = R"(This printer destination is idle and cannot accept control operations.)"},
	{.status = 2159, .dos_err = 317, .posix_err = 0, .msg = R"(This printer destination request contains an invalid control function.)"},
	{.status = 2160, .dos_err = 317, .posix_err = 0, .msg = R"(The print processor is not responding.)"},
	{.status = 2161, .dos_err = 317, .posix_err = 0, .msg = R"(The spooler is not running.)"},
	{.status = 2162, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed on the print destination in its current state.)"},
	{.status = 2163, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed on the printer queue in its current state.)"},
	{.status = 2164, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed on the print job in its current state.)"},
	{.status = 2165, .dos_err = 317, .posix_err = 0, .msg = R"(A spooler memory allocation failure occurred.)"},
	{.status = 2166, .dos_err = 317, .posix_err = 0, .msg = R"(The device driver does not exist.)"},
	{.status = 2167, .dos_err = 317, .posix_err = 0, .msg = R"(The data type is not supported by the print processor.)"},
	{.status = 2168, .dos_err = 317, .posix_err = 0, .msg = R"(The print processor is not installed.)"},
	{.status = 2180, .dos_err = 317, .posix_err = 0, .msg = R"(The service database is locked.)"},
	{.status = 2181, .dos_err = 317, .posix_err = 0, .msg = R"(The service table is full.)"},
	{.status = 2182, .dos_err = 317, .posix_err = 0, .msg = R"(The requested service has already been started.)"},
	{.status = 2183, .dos_err = 317, .posix_err = 0, .msg = R"(The service does not respond to control actions.)"},
	{.status = 2184, .dos_err = 317, .posix_err = 0, .msg = R"(The service has not been started.)"},
	{.status = 2185, .dos_err = 317, .posix_err = 0, .msg = R"(The service name is invalid.)"},
	{.status = 2186, .dos_err = 317, .posix_err = 0, .msg = R"(The service is not responding to the control function.)"},
	{.status = 2187, .dos_err = 317, .posix_err = 0, .msg = R"(The service control is busy.)"},
	{.status = 2188, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration file contains an invalid service program name.)"},
	{.status = 2189, .dos_err = 317, .posix_err = 0, .msg = R"(The service could not be controlled in its present state.)"},
	{.status = 2190, .dos_err = 317, .posix_err = 0, .msg = R"(The service ended abnormally.)"},
	{.status = 2191, .dos_err = 317, .posix_err = 0, .msg = R"(The requested pause, continue, or stop is not valid for this service.)"},
	{.status = 2192, .dos_err = 317, .posix_err = 0, .msg = R"(The service control dispatcher could not find the service name in the dispatch table.)"},
	{.status = 2193, .dos_err = 317, .posix_err = 0, .msg = R"(The service control dispatcher pipe read failed.)"},
	{.status = 2194, .dos_err = 317, .posix_err = 0, .msg = R"(A thread for the new service could not be created.)"},
	{.status = 2200, .dos_err = 317, .posix_err = 0, .msg = R"(This workstation is already logged on to the local-area network.)"},
	{.status = 2201, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation is not logged on to the local-area network.)"},
	{.status = 2202, .dos_err = 317, .posix_err = 0, .msg = R"(The specified username is invalid.)"},
	{.status = 2203, .dos_err = 317, .posix_err = 0, .msg = R"(The password parameter is invalid.)"},
	{.status = 2204, .dos_err = 317, .posix_err = 0, .msg = R"(The logon processor did not add the message alias.)"},
	{.status = 2205, .dos_err = 317, .posix_err = 0, .msg = R"(The logon processor did not add the message alias.)"},
	{.status = 2206, .dos_err = 317, .posix_err = 0, .msg = R"(The logoff processor did not delete the message alias.)"},
	{.status = 2207, .dos_err = 317, .posix_err = 0, .msg = R"(The logoff processor did not delete the message alias.)"},
	{.status = 2209, .dos_err = 317, .posix_err = 0, .msg = R"(Network logons are paused.)"},
	{.status = 2210, .dos_err = 317, .posix_err = 0, .msg = R"(A centralized logon-server conflict occurred.)"},
	{.status = 2211, .dos_err = 317, .posix_err = 0, .msg = R"(The server is configured without a valid user path.)"},
	{.status = 2212, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while loading or running the logon script.)"},
	{.status = 2214, .dos_err = 317, .posix_err = 0, .msg = R"(The logon server was not specified.  Your computer will be logged on as STANDALONE.)"},
	{.status = 2215, .dos_err = 317, .posix_err = 0, .msg = R"(The logon server could not be found.)"},
	{.status = 2216, .dos_err = 317, .posix_err = 0, .msg = R"(There is already a logon domain for this computer.)"},
	{.status = 2217, .dos_err = 317, .posix_err = 0, .msg = R"(The logon server could not validate the logon.)"},
	{.status = 2219, .dos_err = 317, .posix_err = 0, .msg = R"(The security database could not be found.)"},
	{.status = 2220, .dos_err = 317, .posix_err = 0, .msg = R"(The group name could not be found.)"},
	{.status = 2221, .dos_err = 317, .posix_err = 0, .msg = R"(The user name could not be found.)"},
	{.status = 2222, .dos_err = 317, .posix_err = 0, .msg = R"(The resource name could not be found.)"},
	{.status = 2223, .dos_err = 317, .posix_err = 0, .msg = R"(The group already exists.)"},
	{.status = 2224, .dos_err = 317, .posix_err = 0, .msg = R"(The account already exists.)"},
	{.status = 2225, .dos_err = 317, .posix_err = 0, .msg = R"(The resource permission list already exists.)"},
	{.status = 2226, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is only allowed on the primary domain controller of the domain.)"},
	{.status = 2227, .dos_err = 317, .posix_err = 0, .msg = R"(The security database has not been started.)"},
	{.status = 2228, .dos_err = 317, .posix_err = 0, .msg = R"(There are too many names in the user accounts database.)"},
	{.status = 2229, .dos_err = 317, .posix_err = 0, .msg = R"(A disk I/O failure occurred.)"},
	{.status = 2230, .dos_err = 317, .posix_err = 0, .msg = R"(The limit of 64 entries per resource was exceeded.)"},
	{.status = 2231, .dos_err = 317, .posix_err = 0, .msg = R"(Deleting a user with a session is not allowed.)"},
	{.status = 2232, .dos_err = 317, .posix_err = 0, .msg = R"(The parent directory could not be located.)"},
	{.status = 2233, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to add to the security database session cache segment.)"},
	{.status = 2234, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed on this special group.)"},
	{.status = 2235, .dos_err = 317, .posix_err = 0, .msg = R"(This user is not cached in user accounts database session cache.)"},
	{.status = 2236, .dos_err = 317, .posix_err = 0, .msg = R"(The user already belongs to this group.)"},
	{.status = 2237, .dos_err = 317, .posix_err = 0, .msg = R"(The user does not belong to this group.)"},
	{.status = 2238, .dos_err = 317, .posix_err = 0, .msg = R"(This user account is undefined.)"},
	{.status = 2239, .dos_err = 317, .posix_err = 0, .msg = R"(This user account has expired.)"},
	{.status = 2240, .dos_err = 317, .posix_err = 0, .msg = R"(The user is not allowed to log on from this workstation.)"},
	{.status = 2241, .dos_err = 317, .posix_err = 0, .msg = R"(The user is not allowed to log on at this time.)"},
	{.status = 2242, .dos_err = 317, .posix_err = 0, .msg = R"(The password of this user has expired.)"},
	{.status = 2243, .dos_err = 317, .posix_err = 0, .msg = R"(The password of this user cannot change.)"},
	{.status = 2244, .dos_err = 317, .posix_err = 0, .msg = R"(This password cannot be used now.)"},
	{.status = 2245, .dos_err = 317, .posix_err = 0, .msg = R"(The password does not meet the password policy requirements. Check the minimum password length, password complexity and password history requirements.)"},
	{.status = 2246, .dos_err = 317, .posix_err = 0, .msg = R"(The password of this user is too recent to change.)"},
	{.status = 2247, .dos_err = 317, .posix_err = 0, .msg = R"(The security database is corrupted.)"},
	{.status = 2248, .dos_err = 317, .posix_err = 0, .msg = R"(No updates are necessary to this replicant network/local security database.)"},
	{.status = 2249, .dos_err = 317, .posix_err = 0, .msg = R"(This replicant database is outdated; synchronization is required.)"},
	{.status = 2250, .dos_err = 317, .posix_err = 0, .msg = R"(This network connection does not exist.)"},
	{.status = 2251, .dos_err = 317, .posix_err = 0, .msg = R"(This asg_type is invalid.)"},
	{.status = 2252, .dos_err = 317, .posix_err = 0, .msg = R"(This device is currently being shared.)"},
	{.status = 2253, .dos_err = 317, .posix_err = 0, .msg = R"(The user name may not be same as computer name.)"},
	{.status = 2270, .dos_err = 317, .posix_err = 0, .msg = R"(The computer name could not be added as a message alias.  The name may already exist on the network.)"},
	{.status = 2271, .dos_err = 317, .posix_err = 0, .msg = R"(The Messenger service is already started.)"},
	{.status = 2272, .dos_err = 317, .posix_err = 0, .msg = R"(The Messenger service failed to start.)"},
	{.status = 2273, .dos_err = 317, .posix_err = 0, .msg = R"(The message alias could not be found on the network.)"},
	{.status = 2274, .dos_err = 317, .posix_err = 0, .msg = R"(This message alias has already been forwarded.)"},
	{.status = 2275, .dos_err = 317, .posix_err = 0, .msg = R"(This message alias has been added but is still forwarded.)"},
	{.status = 2276, .dos_err = 317, .posix_err = 0, .msg = R"(This message alias already exists locally.)"},
	{.status = 2277, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of added message aliases has been exceeded.)"},
	{.status = 2278, .dos_err = 317, .posix_err = 0, .msg = R"(The computer name could not be deleted.)"},
	{.status = 2279, .dos_err = 317, .posix_err = 0, .msg = R"(Messages cannot be forwarded back to the same workstation.)"},
	{.status = 2280, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred in the domain message processor.)"},
	{.status = 2281, .dos_err = 317, .posix_err = 0, .msg = R"(The message was sent, but the recipient has paused the Messenger service.)"},
	{.status = 2282, .dos_err = 317, .posix_err = 0, .msg = R"(The message was sent but not received.)"},
	{.status = 2283, .dos_err = 317, .posix_err = 0, .msg = R"(The message alias is currently in use. Try again later.)"},
	{.status = 2284, .dos_err = 317, .posix_err = 0, .msg = R"(The Messenger service has not been started.)"},
	{.status = 2285, .dos_err = 317, .posix_err = 0, .msg = R"(The name is not on the local computer.)"},
	{.status = 2286, .dos_err = 317, .posix_err = 0, .msg = R"(The forwarded message alias could not be found on the network.)"},
	{.status = 2287, .dos_err = 317, .posix_err = 0, .msg = R"(The message alias table on the remote station is full.)"},
	{.status = 2288, .dos_err = 317, .posix_err = 0, .msg = R"(Messages for this alias are not currently being forwarded.)"},
	{.status = 2289, .dos_err = 317, .posix_err = 0, .msg = R"(The broadcast message was truncated.)"},
	{.status = 2294, .dos_err = 317, .posix_err = 0, .msg = R"(This is an invalid device name.)"},
	{.status = 2295, .dos_err = 317, .posix_err = 0, .msg = R"(A write fault occurred.)"},
	{.status = 2297, .dos_err = 317, .posix_err = 0, .msg = R"(A duplicate message alias exists on the network.)"},
	{.status = 2298, .dos_err = 317, .posix_err = 0, .msg = R"(This message alias will be deleted later.)"},
	{.status = 2299, .dos_err = 317, .posix_err = 0, .msg = R"(The message alias was not successfully deleted from all networks.)"},
	{.status = 2300, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not supported on computers with multiple networks.)"},
	{.status = 2310, .dos_err = 317, .posix_err = 0, .msg = R"(This shared resource does not exist.)"},
	{.status = 2311, .dos_err = 317, .posix_err = 0, .msg = R"(This device is not shared.)"},
	{.status = 2312, .dos_err = 317, .posix_err = 0, .msg = R"(A session does not exist with that computer name.)"},
	{.status = 2314, .dos_err = 317, .posix_err = 0, .msg = R"(There is not an open file with that identification number.)"},
	{.status = 2315, .dos_err = 317, .posix_err = 0, .msg = R"(A failure occurred when executing a remote administration command.)"},
	{.status = 2316, .dos_err = 317, .posix_err = 0, .msg = R"(A failure occurred when opening a remote temporary file.)"},
	{.status = 2317, .dos_err = 317, .posix_err = 0, .msg = R"(The data returned from a remote administration command has been truncated to 64K.)"},
	{.status = 2318, .dos_err = 317, .posix_err = 0, .msg = R"(This device cannot be shared as both a spooled and a non-spooled resource.)"},
	{.status = 2319, .dos_err = 317, .posix_err = 0, .msg = R"(The information in the list of servers may be incorrect.)"},
	{.status = 2320, .dos_err = 317, .posix_err = 0, .msg = R"(The computer is not active in this domain.)"},
	{.status = 2321, .dos_err = 317, .posix_err = 0, .msg = R"(The share must be removed from the Distributed File System before it can be deleted.)"},
	{.status = 2331, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is invalid for this device.)"},
	{.status = 2332, .dos_err = 317, .posix_err = 0, .msg = R"(This device cannot be shared.)"},
	{.status = 2333, .dos_err = 317, .posix_err = 0, .msg = R"(This device was not open.)"},
	{.status = 2334, .dos_err = 317, .posix_err = 0, .msg = R"(This device name list is invalid.)"},
	{.status = 2335, .dos_err = 317, .posix_err = 0, .msg = R"(The queue priority is invalid.)"},
	{.status = 2337, .dos_err = 317, .posix_err = 0, .msg = R"(There are no shared communication devices.)"},
	{.status = 2338, .dos_err = 317, .posix_err = 0, .msg = R"(The queue you specified does not exist.)"},
	{.status = 2340, .dos_err = 317, .posix_err = 0, .msg = R"(This list of devices is invalid.)"},
	{.status = 2341, .dos_err = 317, .posix_err = 0, .msg = R"(The requested device is invalid.)"},
	{.status = 2342, .dos_err = 317, .posix_err = 0, .msg = R"(This device is already in use by the spooler.)"},
	{.status = 2343, .dos_err = 317, .posix_err = 0, .msg = R"(This device is already in use as a communication device.)"},
	{.status = 2351, .dos_err = 317, .posix_err = 0, .msg = R"(This computer name is invalid.)"},
	{.status = 2354, .dos_err = 317, .posix_err = 0, .msg = R"(The string and prefix specified are too long.)"},
	{.status = 2356, .dos_err = 317, .posix_err = 0, .msg = R"(This path component is invalid.)"},
	{.status = 2357, .dos_err = 317, .posix_err = 0, .msg = R"(Could not determine the type of input.)"},
	{.status = 2362, .dos_err = 317, .posix_err = 0, .msg = R"(The buffer for types is not big enough.)"},
	{.status = 2370, .dos_err = 317, .posix_err = 0, .msg = R"(Profile files cannot exceed 64K.)"},
	{.status = 2371, .dos_err = 317, .posix_err = 0, .msg = R"(The start offset is out of range.)"},
	{.status = 2372, .dos_err = 317, .posix_err = 0, .msg = R"(The system cannot delete current connections to network resources.)"},
	{.status = 2373, .dos_err = 317, .posix_err = 0, .msg = R"(The system was unable to parse the command line in this file.)"},
	{.status = 2374, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while loading the profile file.)"},
	{.status = 2375, .dos_err = 317, .posix_err = 0, .msg = R"(Errors occurred while saving the profile file.  The profile was partially saved.)"},
	{.status = 2377, .dos_err = 317, .posix_err = 0, .msg = R"(Log file %1 is full.)"},
	{.status = 2378, .dos_err = 317, .posix_err = 0, .msg = R"(This log file has changed between reads.)"},
	{.status = 2379, .dos_err = 317, .posix_err = 0, .msg = R"(Log file %1 is corrupt.)"},
	{.status = 2380, .dos_err = 317, .posix_err = 0, .msg = R"(The source path cannot be a directory.)"},
	{.status = 2381, .dos_err = 317, .posix_err = 0, .msg = R"(The source path is illegal.)"},
	{.status = 2382, .dos_err = 317, .posix_err = 0, .msg = R"(The destination path is illegal.)"},
	{.status = 2383, .dos_err = 317, .posix_err = 0, .msg = R"(The source and destination paths are on different servers.)"},
	{.status = 2385, .dos_err = 317, .posix_err = 0, .msg = R"(The Run server you requested is paused.)"},
	{.status = 2389, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred when communicating with a Run server.)"},
	{.status = 2391, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred when starting a background process.)"},
	{.status = 2392, .dos_err = 317, .posix_err = 0, .msg = R"(The shared resource you are connected to could not be found.)"},
	{.status = 2400, .dos_err = 317, .posix_err = 0, .msg = R"(The LAN adapter number is invalid.)"},
	{.status = 2401, .dos_err = 317, .posix_err = 0, .msg = R"(This network connection has files open or requests pending.)"},
	{.status = 2402, .dos_err = 317, .posix_err = 0, .msg = R"(Active connections still exist.)"},
	{.status = 2403, .dos_err = 317, .posix_err = 0, .msg = R"(This share name or password is invalid.)"},
	{.status = 2404, .dos_err = 317, .posix_err = 0, .msg = R"(The device is in use by an active process and cannot be disconnected.)"},
	{.status = 2405, .dos_err = 317, .posix_err = 0, .msg = R"(The drive letter is in use locally.)"},
	{.status = 2430, .dos_err = 317, .posix_err = 0, .msg = R"(The specified client is already registered for the specified event.)"},
	{.status = 2431, .dos_err = 317, .posix_err = 0, .msg = R"(The alert table is full.)"},
	{.status = 2432, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid or nonexistent alert name was raised.)"},
	{.status = 2433, .dos_err = 317, .posix_err = 0, .msg = R"(The alert recipient is invalid.)"},
	{.status = 2434, .dos_err = 317, .posix_err = 0, .msg = R"(A user's session with this server has been deleted

because the user's logon hours are no longer valid.)"},
	{.status = 2440, .dos_err = 317, .posix_err = 0, .msg = R"(The log file does not contain the requested record number.)"},
	{.status = 2450, .dos_err = 317, .posix_err = 0, .msg = R"(The user accounts database is not configured correctly.)"},
	{.status = 2451, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not permitted when the Netlogon service is running.)"},
	{.status = 2452, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed on the last administrative account.)"},
	{.status = 2453, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find domain controller for this domain.)"},
	{.status = 2454, .dos_err = 317, .posix_err = 0, .msg = R"(Could not set logon information for this user.)"},
	{.status = 2455, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service has not been started.)"},
	{.status = 2456, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to add to the user accounts database.)"},
	{.status = 2457, .dos_err = 317, .posix_err = 0, .msg = R"(This server's clock is not synchronized with the primary domain controller's clock.)"},
	{.status = 2458, .dos_err = 317, .posix_err = 0, .msg = R"(A password mismatch has been detected.)"},
	{.status = 2460, .dos_err = 317, .posix_err = 0, .msg = R"(The server identification does not specify a valid server.)"},
	{.status = 2461, .dos_err = 317, .posix_err = 0, .msg = R"(The session identification does not specify a valid session.)"},
	{.status = 2462, .dos_err = 317, .posix_err = 0, .msg = R"(The connection identification does not specify a valid connection.)"},
	{.status = 2463, .dos_err = 317, .posix_err = 0, .msg = R"(There is no space for another entry in the table of available servers.)"},
	{.status = 2464, .dos_err = 317, .posix_err = 0, .msg = R"(The server has reached the maximum number of sessions it supports.)"},
	{.status = 2465, .dos_err = 317, .posix_err = 0, .msg = R"(The server has reached the maximum number of connections it supports.)"},
	{.status = 2466, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot open more files because it has reached its maximum number.)"},
	{.status = 2467, .dos_err = 317, .posix_err = 0, .msg = R"(There are no alternate servers registered on this server.)"},
	{.status = 2470, .dos_err = 317, .posix_err = 0, .msg = R"(Try down-level (remote admin protocol) version of API instead.)"},
	{.status = 2480, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS driver could not be accessed by the UPS service.)"},
	{.status = 2481, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service is not configured correctly.)"},
	{.status = 2482, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service could not access the specified Comm Port.)"},
	{.status = 2483, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS indicated a line fail or low battery situation. Service not started.)"},
	{.status = 2484, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service failed to perform a system shut down.)"},
	{.status = 2500, .dos_err = 317, .posix_err = 0, .msg = R"(The program below returned an MS-DOS error code:)"},
	{.status = 2501, .dos_err = 317, .posix_err = 0, .msg = R"(The program below needs more memory:)"},
	{.status = 2502, .dos_err = 317, .posix_err = 0, .msg = R"(The program below called an unsupported MS-DOS function:)"},
	{.status = 2503, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation failed to boot.)"},
	{.status = 2504, .dos_err = 317, .posix_err = 0, .msg = R"(The file below is corrupt.)"},
	{.status = 2505, .dos_err = 317, .posix_err = 0, .msg = R"(No loader is specified in the boot-block definition file.)"},
	{.status = 2506, .dos_err = 317, .posix_err = 0, .msg = R"(NetBIOS returned an error: The NCB and SMB are dumped above.)"},
	{.status = 2507, .dos_err = 317, .posix_err = 0, .msg = R"(A disk I/O error occurred.)"},
	{.status = 2508, .dos_err = 317, .posix_err = 0, .msg = R"(Image parameter substitution failed.)"},
	{.status = 2509, .dos_err = 317, .posix_err = 0, .msg = R"(Too many image parameters cross disk sector boundaries.)"},
	{.status = 2510, .dos_err = 317, .posix_err = 0, .msg = R"(The image was not generated from an MS-DOS diskette formatted with /S.)"},
	{.status = 2511, .dos_err = 317, .posix_err = 0, .msg = R"(Remote boot will be restarted later.)"},
	{.status = 2512, .dos_err = 317, .posix_err = 0, .msg = R"(The call to the Remoteboot server failed.)"},
	{.status = 2513, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot connect to the Remoteboot server.)"},
	{.status = 2514, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot open image file on the Remoteboot server.)"},
	{.status = 2515, .dos_err = 317, .posix_err = 0, .msg = R"(Connecting to the Remoteboot server...)"},
	{.status = 2516, .dos_err = 317, .posix_err = 0, .msg = R"(Connecting to the Remoteboot server...)"},
	{.status = 2517, .dos_err = 317, .posix_err = 0, .msg = R"(Remote boot service was stopped; check the error log for the cause of the problem.)"},
	{.status = 2518, .dos_err = 317, .posix_err = 0, .msg = R"(Remote boot startup failed; check the error log for the cause of the problem.)"},
	{.status = 2519, .dos_err = 317, .posix_err = 0, .msg = R"(A second connection to a Remoteboot resource is not allowed.)"},
	{.status = 2550, .dos_err = 317, .posix_err = 0, .msg = R"(The browser service was configured with MaintainServerList=No.)"},
	{.status = 2610, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to start since none of the network adapters started with this service.)"},
	{.status = 2611, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to start due to bad startup information in the registry.)"},
	{.status = 2612, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to start because its database is absent or corrupt.)"},
	{.status = 2613, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to start because RPLFILES share is absent.)"},
	{.status = 2614, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to start because RPLUSER group is absent.)"},
	{.status = 2615, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot enumerate service records.)"},
	{.status = 2616, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation record information has been corrupted.)"},
	{.status = 2617, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation record was not found.)"},
	{.status = 2618, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation name is in use by some other workstation.)"},
	{.status = 2619, .dos_err = 317, .posix_err = 0, .msg = R"(Profile record information has been corrupted.)"},
	{.status = 2620, .dos_err = 317, .posix_err = 0, .msg = R"(Profile record was not found.)"},
	{.status = 2621, .dos_err = 317, .posix_err = 0, .msg = R"(Profile name is in use by some other profile.)"},
	{.status = 2622, .dos_err = 317, .posix_err = 0, .msg = R"(There are workstations using this profile.)"},
	{.status = 2623, .dos_err = 317, .posix_err = 0, .msg = R"(Configuration record information has been corrupted.)"},
	{.status = 2624, .dos_err = 317, .posix_err = 0, .msg = R"(Configuration record was not found.)"},
	{.status = 2625, .dos_err = 317, .posix_err = 0, .msg = R"(Adapter id record information has been corrupted.)"},
	{.status = 2626, .dos_err = 317, .posix_err = 0, .msg = R"(An internal service error has occurred.)"},
	{.status = 2627, .dos_err = 317, .posix_err = 0, .msg = R"(Vendor id record information has been corrupted.)"},
	{.status = 2628, .dos_err = 317, .posix_err = 0, .msg = R"(Boot block record information has been corrupted.)"},
	{.status = 2629, .dos_err = 317, .posix_err = 0, .msg = R"(The user account for this workstation record is missing.)"},
	{.status = 2630, .dos_err = 317, .posix_err = 0, .msg = R"(The RPLUSER local group could not be found.)"},
	{.status = 2631, .dos_err = 317, .posix_err = 0, .msg = R"(Boot block record was not found.)"},
	{.status = 2632, .dos_err = 317, .posix_err = 0, .msg = R"(Chosen profile is incompatible with this workstation.)"},
	{.status = 2633, .dos_err = 317, .posix_err = 0, .msg = R"(Chosen network adapter id is in use by some other workstation.)"},
	{.status = 2634, .dos_err = 317, .posix_err = 0, .msg = R"(There are profiles using this configuration.)"},
	{.status = 2635, .dos_err = 317, .posix_err = 0, .msg = R"(There are workstations, profiles or configurations using this boot block.)"},
	{.status = 2636, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to backup Remoteboot database.)"},
	{.status = 2637, .dos_err = 317, .posix_err = 0, .msg = R"(Adapter record was not found.)"},
	{.status = 2638, .dos_err = 317, .posix_err = 0, .msg = R"(Vendor record was not found.)"},
	{.status = 2639, .dos_err = 317, .posix_err = 0, .msg = R"(Vendor name is in use by some other vendor record.)"},
	{.status = 2640, .dos_err = 317, .posix_err = 0, .msg = R"((boot name, vendor id) is in use by some other boot block record.)"},
	{.status = 2641, .dos_err = 317, .posix_err = 0, .msg = R"(Configuration name is in use by some other configuration.)"},
	{.status = 2660, .dos_err = 317, .posix_err = 0, .msg = R"(The internal database maintained by the DFS service is corrupt)"},
	{.status = 2661, .dos_err = 317, .posix_err = 0, .msg = R"(One of the records in the internal DFS database is corrupt)"},
	{.status = 2662, .dos_err = 317, .posix_err = 0, .msg = R"(There is no DFS name whose entry path matches the input Entry Path)"},
	{.status = 2663, .dos_err = 317, .posix_err = 0, .msg = R"(A root or link with the given name already exists)"},
	{.status = 2664, .dos_err = 317, .posix_err = 0, .msg = R"(The server share specified is already shared in the DFS)"},
	{.status = 2665, .dos_err = 317, .posix_err = 0, .msg = R"(The indicated server share does not support the indicated DFS namespace)"},
	{.status = 2666, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is not valid on this portion of the namespace)"},
	{.status = 2667, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is not valid on this portion of the namespace)"},
	{.status = 2668, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is ambiguous because the link has multiple servers)"},
	{.status = 2669, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to create a link)"},
	{.status = 2670, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not DFS Aware)"},
	{.status = 2671, .dos_err = 317, .posix_err = 0, .msg = R"(The specified rename target path is invalid)"},
	{.status = 2672, .dos_err = 317, .posix_err = 0, .msg = R"(The specified DFS link is offline)"},
	{.status = 2673, .dos_err = 317, .posix_err = 0, .msg = R"(The specified server is not a server for this link)"},
	{.status = 2674, .dos_err = 317, .posix_err = 0, .msg = R"(A cycle in the DFS name was detected)"},
	{.status = 2675, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is not supported on a server-based DFS)"},
	{.status = 2676, .dos_err = 317, .posix_err = 0, .msg = R"(This link is already supported by the specified server-share)"},
	{.status = 2677, .dos_err = 317, .posix_err = 0, .msg = R"(Can't remove the last server-share supporting this root or link)"},
	{.status = 2678, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is not supported for an Inter-DFS link)"},
	{.status = 2679, .dos_err = 317, .posix_err = 0, .msg = R"(The internal state of the DFS Service has become inconsistent)"},
	{.status = 2680, .dos_err = 317, .posix_err = 0, .msg = R"(The DFS Service has been installed on the specified server)"},
	{.status = 2681, .dos_err = 317, .posix_err = 0, .msg = R"(The DFS data being reconciled is identical)"},
	{.status = 2682, .dos_err = 317, .posix_err = 0, .msg = R"(The DFS root cannot be deleted - Uninstall DFS if required)"},
	{.status = 2683, .dos_err = 317, .posix_err = 0, .msg = R"(A child or parent directory of the share is already in a DFS)"},
	{.status = 2690, .dos_err = 317, .posix_err = 0, .msg = R"(DFS internal error)"},
	{.status = 2691, .dos_err = 317, .posix_err = 0, .msg = R"(This machine is already joined to a domain.)"},
	{.status = 2692, .dos_err = 317, .posix_err = 0, .msg = R"(This machine is not currently joined to a domain.)"},
	{.status = 2693, .dos_err = 317, .posix_err = 0, .msg = R"(This machine is a domain controller and cannot be unjoined from a domain.)"},
	{.status = 2694, .dos_err = 317, .posix_err = 0, .msg = R"(The destination domain controller does not support creating machine accounts in OUs.)"},
	{.status = 2695, .dos_err = 317, .posix_err = 0, .msg = R"(The specified workgroup name is invalid.)"},
	{.status = 2696, .dos_err = 317, .posix_err = 0, .msg = R"(The specified computer name is incompatible with the default language used on the domain controller.)"},
	{.status = 2697, .dos_err = 317, .posix_err = 0, .msg = R"(The specified computer account could not be found. Contact an administrator to verify the account is in the domain. If the account has been deleted unjoin, reboot, and rejoin the domain.)"},
	{.status = 2698, .dos_err = 317, .posix_err = 0, .msg = R"(This version of Windows cannot be joined to a domain.)"},
	{.status = 2699, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to resolve the DNS name of a domain controller in the domain being joined has failed.  Please verify this client is configured to reach a DNS server that can resolve DNS names in the target domain. For information about network troubleshooting, see Windows Help.)"},
	{.status = 2700, .dos_err = 317, .posix_err = 0, .msg = R"(This device is joined to Azure AD. To join an Active Directory domain, you must first go to settings and choose to disconnect your device from your work or school.)"},
	{.status = 2701, .dos_err = 317, .posix_err = 0, .msg = R"(Password must change at next logon)"},
	{.status = 2702, .dos_err = 317, .posix_err = 0, .msg = R"(Account is locked out)"},
	{.status = 2703, .dos_err = 317, .posix_err = 0, .msg = R"(Password is too long)"},
	{.status = 2704, .dos_err = 317, .posix_err = 0, .msg = R"(Password doesn't meet the complexity policy)"},
	{.status = 2705, .dos_err = 317, .posix_err = 0, .msg = R"(Password doesn't meet the requirements of the filter dll's)"},
	{.status = 2709, .dos_err = 317, .posix_err = 0, .msg = R"(Offline join completion information was not found.)"},
	{.status = 2710, .dos_err = 317, .posix_err = 0, .msg = R"(The offline join completion information was bad.)"},
	{.status = 2711, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to create offline join information. Please ensure you have access to the specified path location and permissions to modify its contents. Running as an elevated administrator may be required.)"},
	{.status = 2712, .dos_err = 317, .posix_err = 0, .msg = R"(The domain join info being saved was incomplete or bad.)"},
	{.status = 2713, .dos_err = 317, .posix_err = 0, .msg = R"(Offline join operation successfully completed but a restart is needed.)"},
	{.status = 2714, .dos_err = 317, .posix_err = 0, .msg = R"(There was no offline join operation pending.)"},
	{.status = 2715, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to set one or more requested machine or domain name values on the local computer.)"},
	{.status = 2716, .dos_err = 317, .posix_err = 0, .msg = R"(Could not verify the current machine's hostname against the saved value in the join completion information.)"},
	{.status = 2717, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to load the specified offline registry hive. Please ensure you have access to the specified path location and permissions to modify its contents. Running as an elevated administrator may be required.)"},
	{.status = 2718, .dos_err = 317, .posix_err = 0, .msg = R"(The minimum session security requirements for this operation were not met.)"},
	{.status = 2719, .dos_err = 317, .posix_err = 0, .msg = R"(Computer account provisioning blob version is not supported.)"},
	{.status = 2720, .dos_err = 317, .posix_err = 0, .msg = R"(The specified domain controller does not meet the version requirement for this operation. Please select a domain controller capable of issuing claims.)"},
	{.status = 2721, .dos_err = 317, .posix_err = 0, .msg = R"(This operation requires a domain controller which supports LDAP. Please select an LDAP-capable domain controller.)"},
	{.status = 2722, .dos_err = 317, .posix_err = 0, .msg = R"(A domain controller which meets the version requirement for this operation could not be located. Please ensure that a domain controller capable of issuing claims is available.)"},
	{.status = 2723, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows version of the specified image does not support provisioning.)"},
	{.status = 2724, .dos_err = 317, .posix_err = 0, .msg = R"(The machine name is blocked from joining the domain.)"},
	{.status = 2725, .dos_err = 317, .posix_err = 0, .msg = R"(The domain controller does not meet the version requirement for this operation. See http://go.microsoft.com/fwlink/?LinkId=294288 for more information.)"},
	{.status = 2726, .dos_err = 317, .posix_err = 0, .msg = R"(The local machine does not allow querying of LSA secrets in plain-text.)"},
	{.status = 2727, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to leave the Azure AD domain that this machine is joined to. Check the event log for detailed error information.)"},
	{.status = 2728, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to update hostname in Azure AD. Check the event log for detailed error information.)"},
	{.status = 2729, .dos_err = 317, .posix_err = 0, .msg = R"(The hostname is already taken by another device.)"},
	{.status = 2730, .dos_err = 317, .posix_err = 0, .msg = R"(The hostname is too long.)"},
	{.status = 2731, .dos_err = 317, .posix_err = 0, .msg = R"(Too many hostnames specified for the device.)"},
	{.status = 2732, .dos_err = 317, .posix_err = 0, .msg = R"(An account with the same name exists in Active Directory. Re-using the account was blocked by security policy.)"},
	{.status = 2999, .dos_err = 317, .posix_err = 0, .msg = R"(This is the last error in NERR range.)"},
	{.status = 3000, .dos_err = 317, .posix_err = 0, .msg = R"(The specified print monitor is unknown.)"},
	{.status = 3001, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer driver is currently in use.)"},
	{.status = 3002, .dos_err = 317, .posix_err = 0, .msg = R"(The spool file was not found.)"},
	{.status = 3003, .dos_err = 317, .posix_err = 0, .msg = R"(A StartDocPrinter call was not issued.)"},
	{.status = 3004, .dos_err = 317, .posix_err = 0, .msg = R"(An AddJob call was not issued.)"},
	{.status = 3005, .dos_err = 317, .posix_err = 0, .msg = R"(The specified print processor has already been installed.)"},
	{.status = 3006, .dos_err = 317, .posix_err = 0, .msg = R"(The specified print monitor has already been installed.)"},
	{.status = 3007, .dos_err = 317, .posix_err = 0, .msg = R"(The specified print monitor does not have the required functions.)"},
	{.status = 3008, .dos_err = 317, .posix_err = 0, .msg = R"(The specified print monitor is currently in use.)"},
	{.status = 3009, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is not allowed when there are jobs queued to the printer.)"},
	{.status = 3010, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is successful. Changes will not be effective until the system is rebooted.)"},
	{.status = 3011, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is successful. Changes will not be effective until the service is restarted.)"},
	{.status = 3012, .dos_err = 317, .posix_err = 0, .msg = R"(No printers were found.)"},
	{.status = 3013, .dos_err = 317, .posix_err = 0, .msg = R"(The printer driver is known to be unreliable.)"},
	{.status = 3014, .dos_err = 317, .posix_err = 0, .msg = R"(The printer driver is known to harm the system.)"},
	{.status = 3015, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer driver package is currently in use.)"},
	{.status = 3016, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to find a core driver package that is required by the printer driver package.)"},
	{.status = 3017, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation failed. A system reboot is required to roll back changes made.)"},
	{.status = 3018, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation failed. A system reboot has been initiated to roll back changes made.)"},
	{.status = 3019, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer driver was not found on the system and needs to be downloaded.)"},
	{.status = 3020, .dos_err = 317, .posix_err = 0, .msg = R"(The requested print job has failed to print. A print system update requires the job to be resubmitted.)"},
	{.status = 3021, .dos_err = 317, .posix_err = 0, .msg = R"(The printer driver does not contain a valid manifest, or contains too many manifests.)"},
	{.status = 3022, .dos_err = 317, .posix_err = 0, .msg = R"(The specified printer cannot be shared.)"},
	{.status = 3023, .dos_err = 317, .posix_err = 0, .msg = R"(There is a problem with a configuration of user specified

shut down command file.  The UPS service started anyway.)"},
	{.status = 3025, .dos_err = 317, .posix_err = 0, .msg = R"(A defective sector on drive %1 has been replaced (hotfixed).

No data was lost.  You should run CHKDSK soon to restore full

performance and replenish the volume's spare sector pool.



The hotfix occurred while processing a remote request.)"},
	{.status = 3026, .dos_err = 317, .posix_err = 0, .msg = R"(A disk error occurred on the HPFS volume in drive %1.

The error occurred while processing a remote request.)"},
	{.status = 3027, .dos_err = 317, .posix_err = 0, .msg = R"(The user accounts database (NET.ACC) is corrupted.  The local security

system is replacing the corrupted NET.ACC with the backup

made on %1 at %2.

Any updates made to the database after this time are lost.)"},
	{.status = 3028, .dos_err = 317, .posix_err = 0, .msg = R"(The user accounts database (NET.ACC) is missing. The local

security system is restoring the backup database

made on %1 at %2.

Any updates made to the database after this time are lost.)"},
	{.status = 3029, .dos_err = 317, .posix_err = 0, .msg = R"(Local security could not be started because the user accounts database

(NET.ACC) was missing or corrupted, and no usable backup

database was present.



THE SYSTEM IS NOT SECURE.)"},
	{.status = 3030, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot export directory %1, to client %2.

It is exported from another server.)"},
	{.status = 3031, .dos_err = 317, .posix_err = 0, .msg = R"(The replication server could not update directory %2 from the source

on %3 due to error %1.)"},
	{.status = 3032, .dos_err = 317, .posix_err = 0, .msg = R"(Master %1 did not send an update notice for directory %2 at the expected

time.)"},
	{.status = 3033, .dos_err = 317, .posix_err = 0, .msg = R"(User %1 has exceeded account limitation %2 on server %3.)"},
	{.status = 3034, .dos_err = 317, .posix_err = 0, .msg = R"(The primary domain controller for domain %1 failed.)"},
	{.status = 3035, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to authenticate with %2, a Windows Domain Controller for

domain %1.)"},
	{.status = 3036, .dos_err = 317, .posix_err = 0, .msg = R"(The replicator attempted to log on at %2 as %1 and failed.)"},
	{.status = 3037, .dos_err = 317, .posix_err = 0, .msg = R"(@I *LOGON HOURS)"},
	{.status = 3038, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator could not access %2

on %3 due to system error %1.)"},
	{.status = 3039, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator limit for files in a directory has been exceeded.)"},
	{.status = 3040, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator limit for tree depth has been exceeded.)"},
	{.status = 3041, .dos_err = 317, .posix_err = 0, .msg = R"(The replicator cannot update directory %1. It has tree integrity

and is the current directory for some process.)"},
	{.status = 3042, .dos_err = 317, .posix_err = 0, .msg = R"(Network error %1 occurred.)"},
	{.status = 3045, .dos_err = 317, .posix_err = 0, .msg = R"(System error %1 occurred.)"},
	{.status = 3046, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot log on. User is currently logged on and argument TRYUSER

is set to NO.)"},
	{.status = 3047, .dos_err = 317, .posix_err = 0, .msg = R"(IMPORT path %1 cannot be found.)"},
	{.status = 3048, .dos_err = 317, .posix_err = 0, .msg = R"(EXPORT path %1 cannot be found.)"},
	{.status = 3049, .dos_err = 317, .posix_err = 0, .msg = R"(Replicated data has changed in directory %1.)"},
	{.status = 3050, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was paused.)"},
	{.status = 3051, .dos_err = 317, .posix_err = 0, .msg = R"(The Registry or the information you just typed includes an illegal

value for "%1".)"},
	{.status = 3052, .dos_err = 317, .posix_err = 0, .msg = R"(The required parameter was not provided on the command

line or in the configuration file.)"},
	{.status = 3053, .dos_err = 317, .posix_err = 0, .msg = R"(LAN Manager does not recognize "%1" as a valid option.)"},
	{.status = 3054, .dos_err = 317, .posix_err = 0, .msg = R"(A request for resource could not be satisfied.)"},
	{.status = 3055, .dos_err = 317, .posix_err = 0, .msg = R"(A problem exists with the system configuration.)"},
	{.status = 3056, .dos_err = 317, .posix_err = 0, .msg = R"(A system error has occurred.)"},
	{.status = 3057, .dos_err = 317, .posix_err = 0, .msg = R"(An internal consistency error has occurred.)"},
	{.status = 3058, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration file or the command line has an ambiguous option.)"},
	{.status = 3059, .dos_err = 317, .posix_err = 0, .msg = R"(The configuration file or the command line has a duplicate parameter.)"},
	{.status = 3060, .dos_err = 317, .posix_err = 0, .msg = R"(The condition supplied for the app execution request was not satisfied, so the request was not performed.)"},
	{.status = 3061, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied handle has been invalidated and may not be used for the requested operation.)"},
	{.status = 3062, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied host generation has been invalidated and may not be used for the requested operation.)"},
	{.status = 3063, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to register a process failed because the target host was not in a valid state to receive process registrations.)"},
	{.status = 3064, .dos_err = 317, .posix_err = 0, .msg = R"(The host is not in a valid state to support the execution request.)"},
	{.status = 3065, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was not completed because a required resource donor was not found for the host.)"},
	{.status = 3066, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was not completed because an unexpected host ID was encountered.)"},
	{.status = 3067, .dos_err = 317, .posix_err = 0, .msg = R"(The operation was not completed because the specified user was not known to the service.)"},
	{.status = 3070, .dos_err = 317, .posix_err = 0, .msg = R"(memory)"},
	{.status = 3071, .dos_err = 317, .posix_err = 0, .msg = R"(disk space)"},
	{.status = 3072, .dos_err = 317, .posix_err = 0, .msg = R"(thread)"},
	{.status = 3073, .dos_err = 317, .posix_err = 0, .msg = R"(process)"},
	{.status = 3074, .dos_err = 317, .posix_err = 0, .msg = R"(Security Failure.)"},
	{.status = 3075, .dos_err = 317, .posix_err = 0, .msg = R"(Bad or missing LAN Manager root directory.)"},
	{.status = 3076, .dos_err = 317, .posix_err = 0, .msg = R"(The network software is not installed.)"},
	{.status = 3077, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not started.)"},
	{.status = 3078, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot access the user accounts database (NET.ACC).)"},
	{.status = 3079, .dos_err = 317, .posix_err = 0, .msg = R"(Incompatible files are installed in the LANMAN tree.)"},
	{.status = 3080, .dos_err = 317, .posix_err = 0, .msg = R"(The LANMAN\LOGS directory is invalid.)"},
	{.status = 3081, .dos_err = 317, .posix_err = 0, .msg = R"(The domain specified could not be used.)"},
	{.status = 3082, .dos_err = 317, .posix_err = 0, .msg = R"(The computer name is being used as a message alias on another computer.)"},
	{.status = 3083, .dos_err = 317, .posix_err = 0, .msg = R"(The announcement of the server name failed.)"},
	{.status = 3084, .dos_err = 317, .posix_err = 0, .msg = R"(The user accounts database is not configured correctly.)"},
	{.status = 3085, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not running with user-level security.)"},
	{.status = 3087, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation is not configured properly.)"},
	{.status = 3088, .dos_err = 317, .posix_err = 0, .msg = R"(View your error log for details.)"},
	{.status = 3089, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to write to this file.)"},
	{.status = 3090, .dos_err = 317, .posix_err = 0, .msg = R"(ADDPAK file is corrupted.  Delete LANMAN\NETPROG\ADDPAK.SER

and reapply all ADDPAKs.)"},
	{.status = 3091, .dos_err = 317, .posix_err = 0, .msg = R"(The LM386 server cannot be started because CACHE.EXE is not running.)"},
	{.status = 3092, .dos_err = 317, .posix_err = 0, .msg = R"(There is no account for this computer in the security database.)"},
	{.status = 3093, .dos_err = 317, .posix_err = 0, .msg = R"(This computer is not a member of the group SERVERS.)"},
	{.status = 3094, .dos_err = 317, .posix_err = 0, .msg = R"(The group SERVERS is not present in the local security database.)"},
	{.status = 3095, .dos_err = 317, .posix_err = 0, .msg = R"(This computer is configured as a member of a workgroup, not as

a member of a domain. The Netlogon service does not need to run in this

configuration.)"},
	{.status = 3096, .dos_err = 317, .posix_err = 0, .msg = R"(The primary Domain Controller for this domain could not be located.)"},
	{.status = 3097, .dos_err = 317, .posix_err = 0, .msg = R"(This computer is configured to be the primary domain controller of its domain.

However, the computer %1 is currently claiming to be the primary domain controller

of the domain.)"},
	{.status = 3098, .dos_err = 317, .posix_err = 0, .msg = R"(The service failed to authenticate with the primary domain controller.)"},
	{.status = 3099, .dos_err = 317, .posix_err = 0, .msg = R"(There is a problem with the security database creation date or serial number.)"},
	{.status = 3100, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because a network software error occurred.)"},
	{.status = 3101, .dos_err = 317, .posix_err = 0, .msg = R"(The system ran out of a resource controlled by the %1 option.)"},
	{.status = 3102, .dos_err = 317, .posix_err = 0, .msg = R"(The service failed to obtain a long-term lock on the

segment for network control blocks (NCBs). The error code is the data.)"},
	{.status = 3103, .dos_err = 317, .posix_err = 0, .msg = R"(The service failed to release the long-term lock on the

segment for network control blocks (NCBs). The error code is the data.)"},
	{.status = 3104, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error stopping service %1.

The error code from NetServiceControl is the data.)"},
	{.status = 3105, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because of a system execution failure on

path %1. The system error code is the data.)"},
	{.status = 3106, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected network control block (NCB) was received. The NCB is the data.)"},
	{.status = 3107, .dos_err = 317, .posix_err = 0, .msg = R"(The network is not started.)"},
	{.status = 3108, .dos_err = 317, .posix_err = 0, .msg = R"(A DosDevIoctl or DosFsCtl to NETWKSTA.SYS failed.

The data shown is in this format:

DWORD  approx CS:IP of call to ioctl or fsctl

WORD   error code

WORD   ioctl or fsctl number)"},
	{.status = 3109, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to create or open system semaphore %1.

The error code is the data.)"},
	{.status = 3110, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because of an open/create error on the

file %1. The system error code is the data.)"},
	{.status = 3111, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected NetBIOS error occurred.

The error code is the data.)"},
	{.status = 3112, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal server message block (SMB) was received.

The SMB is the data.)"},
	{.status = 3113, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because the requested service %1

could not be started.)"},
	{.status = 3114, .dos_err = 317, .posix_err = 0, .msg = R"(Some entries in the error log were lost because of a buffer

overflow.)"},
	{.status = 3120, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization parameters controlling resource usage other

than net buffers are sized so that too much memory is needed.)"},
	{.status = 3121, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot increase the size of a memory segment.)"},
	{.status = 3122, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because account file %1 is either incorrect

or not present.)"},
	{.status = 3123, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because network %1 was not started.)"},
	{.status = 3124, .dos_err = 317, .posix_err = 0, .msg = R"(The server failed to start. Either all three chdev

parameters must be zero or all three must be nonzero.)"},
	{.status = 3125, .dos_err = 317, .posix_err = 0, .msg = R"(A remote API request was halted due to the following

invalid description string: %1.)"},
	{.status = 3126, .dos_err = 317, .posix_err = 0, .msg = R"(The network %1 ran out of network control blocks (NCBs).  You may need to increase NCBs

for this network.  The following information includes the

number of NCBs submitted by the server when this error occurred:)"},
	{.status = 3127, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot create the %1 mailslot needed to send

the ReleaseMemory alert message.  The error received is:)"},
	{.status = 3128, .dos_err = 317, .posix_err = 0, .msg = R"(The server failed to register for the ReleaseMemory alert,

with recipient %1. The error code from

NetAlertStart is the data.)"},
	{.status = 3129, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot update the AT schedule file. The file

is corrupted.)"},
	{.status = 3130, .dos_err = 317, .posix_err = 0, .msg = R"(The server encountered an error when calling

NetIMakeLMFileName. The error code is the data.)"},
	{.status = 3131, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because of a system execution failure on

path %1. There is not enough memory to start the process.

The system error code is the data.)"},
	{.status = 3132, .dos_err = 317, .posix_err = 0, .msg = R"(Longterm lock of the server buffers failed.

Check swap disk's free space and restart the system to start the server.)"},
	{.status = 3140, .dos_err = 317, .posix_err = 0, .msg = R"(The service has stopped due to repeated consecutive

occurrences of a network control block (NCB) error.  The last bad NCB follows

in raw data.)"},
	{.status = 3141, .dos_err = 317, .posix_err = 0, .msg = R"(The Message server has stopped due to a lock on the

Message server shared data segment.)"},
	{.status = 3150, .dos_err = 317, .posix_err = 0, .msg = R"(A file system error occurred while opening or writing to the

system message log file %1. Message logging has been

switched off due to the error. The error code is the data.)"},
	{.status = 3151, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to display message POPUP due to system VIO call error.

The error code is the data.)"},
	{.status = 3152, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal server message block (SMB) was received.  The SMB is the data.)"},
	{.status = 3160, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation information segment is bigger than 64K.

The size follows, in DWORD format:)"},
	{.status = 3161, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation was unable to get the name-number of the computer.)"},
	{.status = 3162, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation could not initialize the Async NetBIOS Thread.

The error code is the data.)"},
	{.status = 3163, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation could not open the initial shared segment.

The error code is the data.)"},
	{.status = 3164, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation host table is full.)"},
	{.status = 3165, .dos_err = 317, .posix_err = 0, .msg = R"(A bad mailslot server message block (SMB) was received.  The SMB is the data.)"},
	{.status = 3166, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation encountered an error while trying to start the user accounts database.

The error code is the data.)"},
	{.status = 3167, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation encountered an error while responding to an SSI revalidation request.

The function code and the error codes are the data.)"},
	{.status = 3170, .dos_err = 317, .posix_err = 0, .msg = R"(The Alerter service had a problem creating the list of

alert recipients.  The error code is %1.)"},
	{.status = 3171, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error expanding %1 as a group name. Try

splitting the group into two or more smaller groups.)"},
	{.status = 3172, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error sending %2 the alert message -

(

%3 )

The error code is %1.)"},
	{.status = 3173, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error in creating or reading the alerter mailslot.

The error code is %1.)"},
	{.status = 3174, .dos_err = 317, .posix_err = 0, .msg = R"(The server could not read the AT schedule file.)"},
	{.status = 3175, .dos_err = 317, .posix_err = 0, .msg = R"(The server found an invalid AT schedule record.)"},
	{.status = 3176, .dos_err = 317, .posix_err = 0, .msg = R"(The server could not find an AT schedule file so it created one.)"},
	{.status = 3177, .dos_err = 317, .posix_err = 0, .msg = R"(The server could not access the %1 network with NetBiosOpen.)"},
	{.status = 3178, .dos_err = 317, .posix_err = 0, .msg = R"(The AT command processor could not run %1.)"},
	{.status = 3180, .dos_err = 317, .posix_err = 0, .msg = R"(WARNING:  Because of a lazy-write error, drive %1 now

contains some corrupted data.  The cache is stopped.)"},
	{.status = 3181, .dos_err = 317, .posix_err = 0, .msg = R"(A defective sector on drive %1 has been replaced (hotfixed).

No data was lost.  You should run CHKDSK soon to restore full

performance and replenish the volume's spare sector pool.



The hotfix occurred while processing a remote request.)"},
	{.status = 3182, .dos_err = 317, .posix_err = 0, .msg = R"(A disk error occurred on the HPFS volume in drive %1.

The error occurred while processing a remote request.)"},
	{.status = 3183, .dos_err = 317, .posix_err = 0, .msg = R"(The user accounts database (NET.ACC) is corrupted.  The local security

system is replacing the corrupted NET.ACC with the backup

made at %1.

Any updates made to the database after this time are lost.)"},
	{.status = 3184, .dos_err = 317, .posix_err = 0, .msg = R"(The user accounts database (NET.ACC) is missing.  The local

security system is restoring the backup database

made at %1.

Any updates made to the database made after this time are lost.)"},
	{.status = 3185, .dos_err = 317, .posix_err = 0, .msg = R"(Local security could not be started because the user accounts database

(NET.ACC) was missing or corrupted, and no usable backup

database was present.



THE SYSTEM IS NOT SECURE.)"},
	{.status = 3186, .dos_err = 317, .posix_err = 0, .msg = R"(Local security could not be started because an error

occurred during initialization. The error code returned is %1.



THE SYSTEM IS NOT SECURE.)"},
	{.status = 3190, .dos_err = 317, .posix_err = 0, .msg = R"(A NetWksta internal error has occurred:

%1)"},
	{.status = 3191, .dos_err = 317, .posix_err = 0, .msg = R"(The redirector is out of a resource: %1.)"},
	{.status = 3192, .dos_err = 317, .posix_err = 0, .msg = R"(A server message block (SMB) error occurred on the connection to %1.

The SMB header is the data.)"},
	{.status = 3193, .dos_err = 317, .posix_err = 0, .msg = R"(A virtual circuit error occurred on the session to %1.

The network control block (NCB) command and return code is the data.)"},
	{.status = 3194, .dos_err = 317, .posix_err = 0, .msg = R"(Hanging up a stuck session to %1.)"},
	{.status = 3195, .dos_err = 317, .posix_err = 0, .msg = R"(A network control block (NCB) error occurred (%1).

The NCB is the data.)"},
	{.status = 3196, .dos_err = 317, .posix_err = 0, .msg = R"(A write operation to %1 failed.

Data may have been lost.)"},
	{.status = 3197, .dos_err = 317, .posix_err = 0, .msg = R"(Reset of driver %1 failed to complete the network control block (NCB).

The NCB is the data.)"},
	{.status = 3198, .dos_err = 317, .posix_err = 0, .msg = R"(The amount of resource %1 requested was more

than the maximum. The maximum amount was allocated.)"},
	{.status = 3204, .dos_err = 317, .posix_err = 0, .msg = R"(The server could not create a thread.

The THREADS parameter in the CONFIG.SYS file should be increased.)"},
	{.status = 3205, .dos_err = 317, .posix_err = 0, .msg = R"(The server could not close %1.

The file is probably corrupted.)"},
	{.status = 3206, .dos_err = 317, .posix_err = 0, .msg = R"(The replicator cannot update directory %1. It has tree integrity

and is the current directory for some process.)"},
	{.status = 3207, .dos_err = 317, .posix_err = 0, .msg = R"(The server cannot export directory %1 to client %2.

It is exported from another server.)"},
	{.status = 3208, .dos_err = 317, .posix_err = 0, .msg = R"(The replication server could not update directory %2 from the source

on %3 due to error %1.)"},
	{.status = 3209, .dos_err = 317, .posix_err = 0, .msg = R"(Master %1 did not send an update notice for directory %2 at the expected

time.)"},
	{.status = 3210, .dos_err = 317, .posix_err = 0, .msg = R"(This computer could not authenticate with %2, a Windows domain controller

for domain %1, and therefore this computer might deny logon requests.

This inability to authenticate might be caused by another computer on the

same network using the same name or the password for this computer account

is not recognized. If this message appears again, contact your system

administrator.)"},
	{.status = 3211, .dos_err = 317, .posix_err = 0, .msg = R"(The replicator attempted to log on at %2 as %1 and failed.)"},
	{.status = 3212, .dos_err = 317, .posix_err = 0, .msg = R"(Network error %1 occurred.)"},
	{.status = 3213, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator limit for files in a directory has been exceeded.)"},
	{.status = 3214, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator limit for tree depth has been exceeded.)"},
	{.status = 3215, .dos_err = 317, .posix_err = 0, .msg = R"(Unrecognized message received in mailslot.)"},
	{.status = 3216, .dos_err = 317, .posix_err = 0, .msg = R"(System error %1 occurred.)"},
	{.status = 3217, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot log on. User is currently logged on and argument TRYUSER

is set to NO.)"},
	{.status = 3218, .dos_err = 317, .posix_err = 0, .msg = R"(IMPORT path %1 cannot be found.)"},
	{.status = 3219, .dos_err = 317, .posix_err = 0, .msg = R"(EXPORT path %1 cannot be found.)"},
	{.status = 3220, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator failed to update signal file in directory %2 due to

%1 system error.)"},
	{.status = 3221, .dos_err = 317, .posix_err = 0, .msg = R"(Disk Fault Tolerance Error



%1)"},
	{.status = 3222, .dos_err = 317, .posix_err = 0, .msg = R"(Replicator could not access %2

on %3 due to system error %1.)"},
	{.status = 3223, .dos_err = 317, .posix_err = 0, .msg = R"(The primary domain controller for domain %1 has apparently failed.)"},
	{.status = 3224, .dos_err = 317, .posix_err = 0, .msg = R"(Changing machine account password for account %1 failed with

the following error: 

%2)"},
	{.status = 3225, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while updating the logon or logoff information for %1.)"},
	{.status = 3226, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while synchronizing with primary domain controller %1)"},
	{.status = 3227, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup to the Windows Domain Controller %1 for the domain %2

failed because %1 does not support signing or sealing the Netlogon

session.



Either upgrade the Domain controller or set the RequireSignOrSeal

registry entry on this machine to 0.)"},
	{.status = 3230, .dos_err = 317, .posix_err = 0, .msg = R"(A power failure was detected at the server.)"},
	{.status = 3231, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service performed server shut down.)"},
	{.status = 3232, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service did not complete execution of the

user specified shut down command file.)"},
	{.status = 3233, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS driver could not be opened.  The error code is

the data.)"},
	{.status = 3234, .dos_err = 317, .posix_err = 0, .msg = R"(Power has been restored.)"},
	{.status = 3235, .dos_err = 317, .posix_err = 0, .msg = R"(There is a problem with a configuration of user specified

shut down command file.)"},
	{.status = 3236, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service failed to execute a user specified shutdown

command file %1.  The error code is the data.)"},
	{.status = 3250, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because of an invalid or missing

parameter in the configuration file %1.)"},
	{.status = 3251, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because of an invalid line in the

configuration file %1. The invalid line is the data.)"},
	{.status = 3252, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because of an error in the configuration

file %1.)"},
	{.status = 3253, .dos_err = 317, .posix_err = 0, .msg = R"(The file %1 has been changed after initialization.

The boot-block loading was temporarily terminated.)"},
	{.status = 3254, .dos_err = 317, .posix_err = 0, .msg = R"(The files do not fit to the boot-block configuration

file %1. Change the BASE and ORG definitions or the order

of the files.)"},
	{.status = 3255, .dos_err = 317, .posix_err = 0, .msg = R"(Initialization failed because the dynamic-link

library %1 returned an incorrect version number.)"},
	{.status = 3256, .dos_err = 317, .posix_err = 0, .msg = R"(There was an unrecoverable error in the dynamic-

link library of the service.)"},
	{.status = 3257, .dos_err = 317, .posix_err = 0, .msg = R"(The system returned an unexpected error code.

The error code is the data.)"},
	{.status = 3258, .dos_err = 317, .posix_err = 0, .msg = R"(The fault-tolerance error log file, LANROOT\LOGS\FT.LOG,

is more than 64K.)"},
	{.status = 3259, .dos_err = 317, .posix_err = 0, .msg = R"(The fault-tolerance error-log file, LANROOT\LOGS\FT.LOG, had the

update in progress bit set upon opening, which means that the

system crashed while working on the error log.)"},
	{.status = 3260, .dos_err = 317, .posix_err = 0, .msg = R"(This computer has been successfully joined to domain '%1'.)"},
	{.status = 3261, .dos_err = 317, .posix_err = 0, .msg = R"(This computer has been successfully joined to workgroup '%1'.)"},
	{.status = 3299, .dos_err = 317, .posix_err = 0, .msg = R"(%1 %2 %3 %4 %5 %6 %7 %8 %9.)"},
	{.status = 3301, .dos_err = 317, .posix_err = 0, .msg = R"(Remote IPC)"},
	{.status = 3302, .dos_err = 317, .posix_err = 0, .msg = R"(Remote Admin)"},
	{.status = 3303, .dos_err = 317, .posix_err = 0, .msg = R"(Logon server share)"},
	{.status = 3304, .dos_err = 317, .posix_err = 0, .msg = R"(A network error occurred.)"},
	{.status = 3400, .dos_err = 317, .posix_err = 0, .msg = R"(There is not enough memory to start the Workstation service.)"},
	{.status = 3401, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred when reading the NETWORKS entry in the LANMAN.INI file.)"},
	{.status = 3402, .dos_err = 317, .posix_err = 0, .msg = R"(This is an invalid argument: %1.)"},
	{.status = 3403, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 NETWORKS entry in the LANMAN.INI file has a

syntax error and will be ignored.)"},
	{.status = 3404, .dos_err = 317, .posix_err = 0, .msg = R"(There are too many NETWORKS entries in the LANMAN.INI file.)"},
	{.status = 3406, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred when opening network

device driver %1 = %2.)"},
	{.status = 3407, .dos_err = 317, .posix_err = 0, .msg = R"(Device driver %1 sent a bad BiosLinkage response.)"},
	{.status = 3408, .dos_err = 317, .posix_err = 0, .msg = R"(The program cannot be used with this operating system.)"},
	{.status = 3409, .dos_err = 317, .posix_err = 0, .msg = R"(The redirector is already installed.)"},
	{.status = 3410, .dos_err = 317, .posix_err = 0, .msg = R"(Installing NETWKSTA.SYS Version %1.%2.%3  (%4))"},
	{.status = 3411, .dos_err = 317, .posix_err = 0, .msg = R"(There was an error installing NETWKSTA.SYS.



Press ENTER to continue.)"},
	{.status = 3412, .dos_err = 317, .posix_err = 0, .msg = R"(Resolver linkage problem.)"},
	{.status = 3413, .dos_err = 317, .posix_err = 0, .msg = R"(Your logon time at %1 ends at %2.

Please clean up and log off.)"},
	{.status = 3414, .dos_err = 317, .posix_err = 0, .msg = R"(You will be automatically disconnected at %1.)"},
	{.status = 3415, .dos_err = 317, .posix_err = 0, .msg = R"(Your logon time at %1 has ended.)"},
	{.status = 3416, .dos_err = 317, .posix_err = 0, .msg = R"(Your logon time at %1 ended at %2.)"},
	{.status = 3417, .dos_err = 317, .posix_err = 0, .msg = R"(WARNING: You have until %1 to logoff. If you

have not logged off at this time, your session will be

disconnected, and any open files or devices you

have open may lose data.)"},
	{.status = 3418, .dos_err = 317, .posix_err = 0, .msg = R"(WARNING: You must log off at %1 now.  You have

two minutes to log off, or you will be disconnected.)"},
	{.status = 3419, .dos_err = 317, .posix_err = 0, .msg = R"(You have open files or devices, and a forced

disconnection may cause you to lose data.)"},
	{.status = 3420, .dos_err = 317, .posix_err = 0, .msg = R"(Default Share for Internal Use)"},
	{.status = 3421, .dos_err = 317, .posix_err = 0, .msg = R"(Messenger Service)"},
	{.status = 3500, .dos_err = 317, .posix_err = 0, .msg = R"(The command completed successfully.)"},
	{.status = 3501, .dos_err = 317, .posix_err = 0, .msg = R"(You used an invalid option.)"},
	{.status = 3502, .dos_err = 317, .posix_err = 0, .msg = R"(System error %1 has occurred.)"},
	{.status = 3503, .dos_err = 317, .posix_err = 0, .msg = R"(The command contains an invalid number of arguments.)"},
	{.status = 3504, .dos_err = 317, .posix_err = 0, .msg = R"(The command completed with one or more errors.)"},
	{.status = 3505, .dos_err = 317, .posix_err = 0, .msg = R"(You used an option with an invalid value.)"},
	{.status = 3506, .dos_err = 317, .posix_err = 0, .msg = R"(The option %1 is unknown.)"},
	{.status = 3507, .dos_err = 317, .posix_err = 0, .msg = R"(Option %1 is ambiguous.)"},
	{.status = 3510, .dos_err = 317, .posix_err = 0, .msg = R"(A command was used with conflicting switches.)"},
	{.status = 3511, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find subprogram %1.)"},
	{.status = 3512, .dos_err = 317, .posix_err = 0, .msg = R"(The software requires a newer version of the operating

system.)"},
	{.status = 3513, .dos_err = 317, .posix_err = 0, .msg = R"(More data is available than can be returned by Windows.)"},
	{.status = 3514, .dos_err = 317, .posix_err = 0, .msg = R"(More help is available by typing NET HELPMSG %1.)"},
	{.status = 3515, .dos_err = 317, .posix_err = 0, .msg = R"(This command can be used only on a Windows Domain Controller.)"},
	{.status = 3516, .dos_err = 317, .posix_err = 0, .msg = R"(This command cannot be used on a Windows Domain Controller.)"},
	{.status = 3520, .dos_err = 317, .posix_err = 0, .msg = R"(These Windows services are started:)"},
	{.status = 3521, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service is not started.)"},
	{.status = 3522, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service is starting)"},
	{.status = 3523, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service could not be started.)"},
	{.status = 3524, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service was started successfully.)"},
	{.status = 3525, .dos_err = 317, .posix_err = 0, .msg = R"(Stopping the Workstation service also stops the Server service.)"},
	{.status = 3526, .dos_err = 317, .posix_err = 0, .msg = R"(The workstation has open files.)"},
	{.status = 3527, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service is stopping)"},
	{.status = 3528, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service could not be stopped.)"},
	{.status = 3529, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service was stopped successfully.)"},
	{.status = 3530, .dos_err = 317, .posix_err = 0, .msg = R"(The following services are dependent on the %1 service.

Stopping the %1 service will also stop these services.)"},
	{.status = 3533, .dos_err = 317, .posix_err = 0, .msg = R"(The service is starting or stopping.  Please try again later.)"},
	{.status = 3534, .dos_err = 317, .posix_err = 0, .msg = R"(The service did not report an error.)"},
	{.status = 3535, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred controlling the device.)"},
	{.status = 3536, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service was continued successfully.)"},
	{.status = 3537, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service was paused successfully.)"},
	{.status = 3538, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service failed to resume.)"},
	{.status = 3539, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service failed to pause.)"},
	{.status = 3540, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service continue is pending)"},
	{.status = 3541, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service pause is pending)"},
	{.status = 3542, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was continued successfully.)"},
	{.status = 3543, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was paused successfully.)"},
	{.status = 3544, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 service has been started by another process and is pending.)"},
	{.status = 3547, .dos_err = 317, .posix_err = 0, .msg = R"(A service specific error occurred: %1.)"},
	{.status = 3660, .dos_err = 317, .posix_err = 0, .msg = R"(These workstations have sessions on this server:)"},
	{.status = 3661, .dos_err = 317, .posix_err = 0, .msg = R"(These workstations have sessions with open files on this server:)"},
	{.status = 3666, .dos_err = 317, .posix_err = 0, .msg = R"(The message alias is forwarded.)"},
	{.status = 3670, .dos_err = 317, .posix_err = 0, .msg = R"(You have these remote connections:)"},
	{.status = 3671, .dos_err = 317, .posix_err = 0, .msg = R"(Continuing will cancel the connections.)"},
	{.status = 3675, .dos_err = 317, .posix_err = 0, .msg = R"(The session from %1 has open files.)"},
	{.status = 3676, .dos_err = 317, .posix_err = 0, .msg = R"(New connections will be remembered.)"},
	{.status = 3677, .dos_err = 317, .posix_err = 0, .msg = R"(New connections will not be remembered.)"},
	{.status = 3678, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while saving your profile : Access Denied. The state of your remembered connections has not changed.)"},
	{.status = 3679, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while reading your profile.)"},
	{.status = 3680, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while restoring the connection to %1.)"},
	{.status = 3682, .dos_err = 317, .posix_err = 0, .msg = R"(No network services are started.)"},
	{.status = 3683, .dos_err = 317, .posix_err = 0, .msg = R"(There are no entries in the list.)"},
	{.status = 3688, .dos_err = 317, .posix_err = 0, .msg = R"(Users have open files on %1.  Continuing the operation will force the files closed.)"},
	{.status = 3689, .dos_err = 317, .posix_err = 0, .msg = R"(The Workstation service is already running. Windows will ignore command options for the workstation.)"},
	{.status = 3691, .dos_err = 317, .posix_err = 0, .msg = R"(There are open files and/or incomplete directory searches pending on the connection to %1.)"},
	{.status = 3693, .dos_err = 317, .posix_err = 0, .msg = R"(The request will be processed at a domain controller for domain %1.)"},
	{.status = 3694, .dos_err = 317, .posix_err = 0, .msg = R"(The shared queue cannot be deleted while a print job is being spooled to the queue.)"},
	{.status = 3695, .dos_err = 317, .posix_err = 0, .msg = R"(%1 has a remembered connection to %2.)"},
	{.status = 3710, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while opening the Help file.)"},
	{.status = 3711, .dos_err = 317, .posix_err = 0, .msg = R"(The Help file is empty.)"},
	{.status = 3712, .dos_err = 317, .posix_err = 0, .msg = R"(The Help file is corrupted.)"},
	{.status = 3713, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find a domain controller for domain %1.)"},
	{.status = 3714, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is privileged on systems with earlier

versions of the software.)"},
	{.status = 3716, .dos_err = 317, .posix_err = 0, .msg = R"(The device type is unknown.)"},
	{.status = 3717, .dos_err = 317, .posix_err = 0, .msg = R"(The log file has been corrupted.)"},
	{.status = 3718, .dos_err = 317, .posix_err = 0, .msg = R"(Program filenames must end with .EXE.)"},
	{.status = 3719, .dos_err = 317, .posix_err = 0, .msg = R"(A matching share could not be found so nothing was deleted.)"},
	{.status = 3720, .dos_err = 317, .posix_err = 0, .msg = R"(A bad value is in the units-per-week field of the user record.)"},
	{.status = 3721, .dos_err = 317, .posix_err = 0, .msg = R"(The password is invalid for %1.)"},
	{.status = 3722, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while sending a message to %1.)"},
	{.status = 3723, .dos_err = 317, .posix_err = 0, .msg = R"(The password or user name is invalid for %1.)"},
	{.status = 3725, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred when the share was deleted.)"},
	{.status = 3726, .dos_err = 317, .posix_err = 0, .msg = R"(The user name is invalid.)"},
	{.status = 3727, .dos_err = 317, .posix_err = 0, .msg = R"(The password is invalid.)"},
	{.status = 3728, .dos_err = 317, .posix_err = 0, .msg = R"(The passwords do not match.)"},
	{.status = 3729, .dos_err = 317, .posix_err = 0, .msg = R"(Your persistent connections were not all restored.)"},
	{.status = 3730, .dos_err = 317, .posix_err = 0, .msg = R"(This is not a valid computer name or domain name.)"},
	{.status = 3732, .dos_err = 317, .posix_err = 0, .msg = R"(Default permissions cannot be set for that resource.)"},
	{.status = 3734, .dos_err = 317, .posix_err = 0, .msg = R"(A valid password was not entered.)"},
	{.status = 3735, .dos_err = 317, .posix_err = 0, .msg = R"(A valid name was not entered.)"},
	{.status = 3736, .dos_err = 317, .posix_err = 0, .msg = R"(The resource named cannot be shared.)"},
	{.status = 3737, .dos_err = 317, .posix_err = 0, .msg = R"(The permissions string contains invalid permissions.)"},
	{.status = 3738, .dos_err = 317, .posix_err = 0, .msg = R"(You can only perform this operation on printers and communication devices.)"},
	{.status = 3742, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is an invalid user or group name.)"},
	{.status = 3743, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not configured for remote administration.)"},
	{.status = 3752, .dos_err = 317, .posix_err = 0, .msg = R"(No users have sessions with this server.)"},
	{.status = 3753, .dos_err = 317, .posix_err = 0, .msg = R"(User %1 is not a member of group %2.)"},
	{.status = 3754, .dos_err = 317, .posix_err = 0, .msg = R"(User %1 is already a member of group %2.)"},
	{.status = 3755, .dos_err = 317, .posix_err = 0, .msg = R"(There is no such user: %1.)"},
	{.status = 3756, .dos_err = 317, .posix_err = 0, .msg = R"(This is an invalid response.)"},
	{.status = 3757, .dos_err = 317, .posix_err = 0, .msg = R"(No valid response was provided.)"},
	{.status = 3758, .dos_err = 317, .posix_err = 0, .msg = R"(The destination list provided does not match the destination list of the printer queue.)"},
	{.status = 3759, .dos_err = 317, .posix_err = 0, .msg = R"(Your password cannot be changed until %1.)"},
	{.status = 3760, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a recognized day of the week.)"},
	{.status = 3761, .dos_err = 317, .posix_err = 0, .msg = R"(The time range specified ends before it starts.)"},
	{.status = 3762, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a recognized hour.)"},
	{.status = 3763, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a valid specification for minutes.)"},
	{.status = 3764, .dos_err = 317, .posix_err = 0, .msg = R"(Time supplied is not exactly on the hour.)"},
	{.status = 3765, .dos_err = 317, .posix_err = 0, .msg = R"(12 and 24 hour time formats may not be mixed.)"},
	{.status = 3766, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a valid 12-hour suffix.)"},
	{.status = 3767, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal date format has been supplied.)"},
	{.status = 3768, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal day range has been supplied.)"},
	{.status = 3769, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal time range has been supplied.)"},
	{.status = 3770, .dos_err = 317, .posix_err = 0, .msg = R"(Arguments to NET USER are invalid. Check the minimum password

length and/or arguments supplied.)"},
	{.status = 3771, .dos_err = 317, .posix_err = 0, .msg = R"(The value for ENABLESCRIPT must be YES.)"},
	{.status = 3773, .dos_err = 317, .posix_err = 0, .msg = R"(An illegal country/region code has been supplied.)"},
	{.status = 3774, .dos_err = 317, .posix_err = 0, .msg = R"(The user was successfully created but could not be added

to the USERS local group.)"},
	{.status = 3775, .dos_err = 317, .posix_err = 0, .msg = R"(The user context supplied is invalid.)"},
	{.status = 3776, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic-link library %1 could not be loaded, or an error

occurred while trying to use it.)"},
	{.status = 3777, .dos_err = 317, .posix_err = 0, .msg = R"(Sending files is no longer supported.)"},
	{.status = 3778, .dos_err = 317, .posix_err = 0, .msg = R"(You may not specify paths for ADMIN$ and IPC$ shares.)"},
	{.status = 3779, .dos_err = 317, .posix_err = 0, .msg = R"(User or group %1 is already a member of local group %2.)"},
	{.status = 3780, .dos_err = 317, .posix_err = 0, .msg = R"(There is no such user or group: %1.)"},
	{.status = 3781, .dos_err = 317, .posix_err = 0, .msg = R"(There is no such computer: %1.)"},
	{.status = 3782, .dos_err = 317, .posix_err = 0, .msg = R"(The computer %1 already exists.)"},
	{.status = 3783, .dos_err = 317, .posix_err = 0, .msg = R"(There is no such global user or group: %1.)"},
	{.status = 3784, .dos_err = 317, .posix_err = 0, .msg = R"(Only disk shares can be marked as cacheable)"},
	{.status = 3790, .dos_err = 317, .posix_err = 0, .msg = R"(The system could not find message: %1.)"},
	{.status = 3802, .dos_err = 317, .posix_err = 0, .msg = R"(This schedule date is invalid.)"},
	{.status = 3803, .dos_err = 317, .posix_err = 0, .msg = R"(The LANMAN root directory is unavailable.)"},
	{.status = 3804, .dos_err = 317, .posix_err = 0, .msg = R"(The SCHED.LOG file could not be opened.)"},
	{.status = 3805, .dos_err = 317, .posix_err = 0, .msg = R"(The Server service has not been started.)"},
	{.status = 3806, .dos_err = 317, .posix_err = 0, .msg = R"(The AT job ID does not exist.)"},
	{.status = 3807, .dos_err = 317, .posix_err = 0, .msg = R"(The AT schedule file is corrupted.)"},
	{.status = 3808, .dos_err = 317, .posix_err = 0, .msg = R"(The delete failed due to a problem with the AT schedule file.)"},
	{.status = 3809, .dos_err = 317, .posix_err = 0, .msg = R"(The command line cannot exceed 259 characters.)"},
	{.status = 3810, .dos_err = 317, .posix_err = 0, .msg = R"(The AT schedule file could not be updated because the disk is full.)"},
	{.status = 3812, .dos_err = 317, .posix_err = 0, .msg = R"(The AT schedule file is invalid.  Please delete the file and create a new one.)"},
	{.status = 3813, .dos_err = 317, .posix_err = 0, .msg = R"(The AT schedule file was deleted.)"},
	{.status = 3814, .dos_err = 317, .posix_err = 0, .msg = R"(The syntax of this command is:



AT [id] [/DELETE]

AT time [/EVERY:date | /NEXT:date] command



The AT command schedules a program command to run at a

later date and time on a server.  It also displays the

list of programs and commands scheduled to be run.



You can specify the date as M,T,W,Th,F,Sa,Su or 1-31

for the day of the month.



You can specify the time in the 24 hour HH:MM format.)"},
	{.status = 3815, .dos_err = 317, .posix_err = 0, .msg = R"(The AT command has timed-out.

Please try again later.)"},
	{.status = 3816, .dos_err = 317, .posix_err = 0, .msg = R"(The minimum password age for user accounts cannot be greater

than the maximum password age.)"},
	{.status = 3817, .dos_err = 317, .posix_err = 0, .msg = R"(You have specified a value that is incompatible

with servers with down-level software. Please specify a lower value.)"},
	{.status = 3870, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a valid computer name.)"},
	{.status = 3871, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a valid Windows network message number.)"},
	{.status = 3900, .dos_err = 317, .posix_err = 0, .msg = R"(Message from %1 to %2 on %3)"},
	{.status = 3901, .dos_err = 317, .posix_err = 0, .msg = R"(****)"},
	{.status = 3902, .dos_err = 317, .posix_err = 0, .msg = R"(**** unexpected end of message ****)"},
	{.status = 3905, .dos_err = 317, .posix_err = 0, .msg = R"(Press ESC to exit)"},
	{.status = 3906, .dos_err = 317, .posix_err = 0, .msg = R"(...)"},
	{.status = 3910, .dos_err = 317, .posix_err = 0, .msg = R"(Current time at %1 is %2)"},
	{.status = 3911, .dos_err = 317, .posix_err = 0, .msg = R"(The current local clock is %1

Do you want to set the local computer's time to match the

time at %2? %3:)"},
	{.status = 3912, .dos_err = 317, .posix_err = 0, .msg = R"(Could not locate a time-server.)"},
	{.status = 3913, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find the domain controller for domain %1.)"},
	{.status = 3914, .dos_err = 317, .posix_err = 0, .msg = R"(Local time (GMT%3) at %1 is %2)"},
	{.status = 3915, .dos_err = 317, .posix_err = 0, .msg = R"(The user's home directory could not be determined.)"},
	{.status = 3916, .dos_err = 317, .posix_err = 0, .msg = R"(The user's home directory has not been specified.)"},
	{.status = 3917, .dos_err = 317, .posix_err = 0, .msg = R"(The name specified for the user's home directory (%1) is not a universal naming convention (UNC) name.)"},
	{.status = 3918, .dos_err = 317, .posix_err = 0, .msg = R"(Drive %1 is now connected to %2. Your home directory is %3\%4.)"},
	{.status = 3919, .dos_err = 317, .posix_err = 0, .msg = R"(Drive %1 is now connected to %2.)"},
	{.status = 3920, .dos_err = 317, .posix_err = 0, .msg = R"(There are no available drive letters left.)"},
	{.status = 3932, .dos_err = 317, .posix_err = 0, .msg = R"(%1 is not a valid domain or workgroup name.)"},
	{.status = 3935, .dos_err = 317, .posix_err = 0, .msg = R"(The current SNTP value is: %1)"},
	{.status = 3936, .dos_err = 317, .posix_err = 0, .msg = R"(This computer is not currently configured to use a specific SNTP server.)"},
	{.status = 3937, .dos_err = 317, .posix_err = 0, .msg = R"(This current autoconfigured SNTP value is: %1)"},
	{.status = 3950, .dos_err = 317, .posix_err = 0, .msg = R"(Reissue the given operation as a cached IO operation)"},
	{.status = 3951, .dos_err = 317, .posix_err = 0, .msg = R"(You specified too many values for the %1 option.)"},
	{.status = 3952, .dos_err = 317, .posix_err = 0, .msg = R"(You entered an invalid value for the %1 option.)"},
	{.status = 3953, .dos_err = 317, .posix_err = 0, .msg = R"(The syntax is incorrect.)"},
	{.status = 3960, .dos_err = 317, .posix_err = 0, .msg = R"(You specified an invalid file number.)"},
	{.status = 3961, .dos_err = 317, .posix_err = 0, .msg = R"(You specified an invalid print job number.)"},
	{.status = 3963, .dos_err = 317, .posix_err = 0, .msg = R"(The user or group account specified cannot be found.)"},
	{.status = 3965, .dos_err = 317, .posix_err = 0, .msg = R"(The user was added but could not be enabled for File and Print

Services for NetWare.)"},
	{.status = 3966, .dos_err = 317, .posix_err = 0, .msg = R"(File and Print Services for NetWare is not installed.)"},
	{.status = 3967, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot set user properties for File and Print Services for NetWare.)"},
	{.status = 3968, .dos_err = 317, .posix_err = 0, .msg = R"(Password for %1 is: %2)"},
	{.status = 3969, .dos_err = 317, .posix_err = 0, .msg = R"(NetWare compatible logon)"},
	{.status = 4000, .dos_err = 317, .posix_err = 0, .msg = R"(WINS encountered an error while processing the command.)"},
	{.status = 4001, .dos_err = 317, .posix_err = 0, .msg = R"(The local WINS cannot be deleted.)"},
	{.status = 4002, .dos_err = 317, .posix_err = 0, .msg = R"(The importation from the file failed.)"},
	{.status = 4003, .dos_err = 317, .posix_err = 0, .msg = R"(The backup failed. Was a full backup done before?)"},
	{.status = 4004, .dos_err = 317, .posix_err = 0, .msg = R"(The backup failed. Check the directory to which you are backing the database.)"},
	{.status = 4005, .dos_err = 317, .posix_err = 0, .msg = R"(The name does not exist in the WINS database.)"},
	{.status = 4006, .dos_err = 317, .posix_err = 0, .msg = R"(Replication with a nonconfigured partner is not allowed.)"},
	{.status = 4050, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the supplied content information is not supported.)"},
	{.status = 4051, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied content information is malformed.)"},
	{.status = 4052, .dos_err = 317, .posix_err = 0, .msg = R"(The requested data cannot be found in local or peer caches.)"},
	{.status = 4053, .dos_err = 317, .posix_err = 0, .msg = R"(No more data is available or required.)"},
	{.status = 4054, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied object has not been initialized.)"},
	{.status = 4055, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied object has already been initialized.)"},
	{.status = 4056, .dos_err = 317, .posix_err = 0, .msg = R"(A shutdown operation is already in progress.)"},
	{.status = 4057, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied object has already been invalidated.)"},
	{.status = 4058, .dos_err = 317, .posix_err = 0, .msg = R"(An element already exists and was not replaced.)"},
	{.status = 4059, .dos_err = 317, .posix_err = 0, .msg = R"(Can not cancel the requested operation as it has already been completed.)"},
	{.status = 4060, .dos_err = 317, .posix_err = 0, .msg = R"(Can not perform the requested operation because it has already been carried out.)"},
	{.status = 4061, .dos_err = 317, .posix_err = 0, .msg = R"(An operation accessed data beyond the bounds of valid data.)"},
	{.status = 4062, .dos_err = 317, .posix_err = 0, .msg = R"(The requested version is not supported.)"},
	{.status = 4063, .dos_err = 317, .posix_err = 0, .msg = R"(A configuration value is invalid.)"},
	{.status = 4064, .dos_err = 317, .posix_err = 0, .msg = R"(The SKU is not licensed.)"},
	{.status = 4065, .dos_err = 317, .posix_err = 0, .msg = R"(PeerDist Service is still initializing and will be available shortly.)"},
	{.status = 4066, .dos_err = 317, .posix_err = 0, .msg = R"(Communication with one or more computers will be temporarily blocked due to recent errors.)"},
	{.status = 4100, .dos_err = 317, .posix_err = 0, .msg = R"(The DHCP client has obtained an IP address that is already in use on the network. The local interface will be disabled until the DHCP client can obtain a new address.)"},
	{.status = 4200, .dos_err = 317, .posix_err = 0, .msg = R"(The GUID passed was not recognized as valid by a WMI data provider.)"},
	{.status = 4201, .dos_err = 317, .posix_err = 0, .msg = R"(The instance name passed was not recognized as valid by a WMI data provider.)"},
	{.status = 4202, .dos_err = 317, .posix_err = 0, .msg = R"(The data item ID passed was not recognized as valid by a WMI data provider.)"},
	{.status = 4203, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI request could not be completed and should be retried.)"},
	{.status = 4204, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data provider could not be located.)"},
	{.status = 4205, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data provider references an instance set that has not been registered.)"},
	{.status = 4206, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data block or event notification has already been enabled.)"},
	{.status = 4207, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data block is no longer available.)"},
	{.status = 4208, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data service is not available.)"},
	{.status = 4209, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data provider failed to carry out the request.)"},
	{.status = 4210, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI MOF information is not valid.)"},
	{.status = 4211, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI registration information is not valid.)"},
	{.status = 4212, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data block or event notification has already been disabled.)"},
	{.status = 4213, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data item or data block is read only.)"},
	{.status = 4214, .dos_err = 317, .posix_err = 0, .msg = R"(The WMI data item or data block could not be changed.)"},
	{.status = 4250, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is only valid in the context of an app container.)"},
	{.status = 4251, .dos_err = 317, .posix_err = 0, .msg = R"(This application can only run in the context of an app container.)"},
	{.status = 4252, .dos_err = 317, .posix_err = 0, .msg = R"(This functionality is not supported in the context of an app container.)"},
	{.status = 4253, .dos_err = 317, .posix_err = 0, .msg = R"(The length of the SID supplied is not a valid length for app container SIDs.)"},
	{.status = 4300, .dos_err = 317, .posix_err = 0, .msg = R"(The media identifier does not represent a valid medium.)"},
	{.status = 4301, .dos_err = 317, .posix_err = 0, .msg = R"(The library identifier does not represent a valid library.)"},
	{.status = 4302, .dos_err = 317, .posix_err = 0, .msg = R"(The media pool identifier does not represent a valid media pool.)"},
	{.status = 4303, .dos_err = 317, .posix_err = 0, .msg = R"(The drive and medium are not compatible or exist in different libraries.)"},
	{.status = 4304, .dos_err = 317, .posix_err = 0, .msg = R"(The medium currently exists in an offline library and must be online to perform this operation.)"},
	{.status = 4305, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed on an offline library.)"},
	{.status = 4306, .dos_err = 317, .posix_err = 0, .msg = R"(The library, drive, or media pool is empty.)"},
	{.status = 4307, .dos_err = 317, .posix_err = 0, .msg = R"(The library, drive, or media pool must be empty to perform this operation.)"},
	{.status = 4308, .dos_err = 317, .posix_err = 0, .msg = R"(No media is currently available in this media pool or library.)"},
	{.status = 4309, .dos_err = 317, .posix_err = 0, .msg = R"(A resource required for this operation is disabled.)"},
	{.status = 4310, .dos_err = 317, .posix_err = 0, .msg = R"(The media identifier does not represent a valid cleaner.)"},
	{.status = 4311, .dos_err = 317, .posix_err = 0, .msg = R"(The drive cannot be cleaned or does not support cleaning.)"},
	{.status = 4312, .dos_err = 317, .posix_err = 0, .msg = R"(The object identifier does not represent a valid object.)"},
	{.status = 4313, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to read from or write to the database.)"},
	{.status = 4314, .dos_err = 317, .posix_err = 0, .msg = R"(The database is full.)"},
	{.status = 4315, .dos_err = 317, .posix_err = 0, .msg = R"(The medium is not compatible with the device or media pool.)"},
	{.status = 4316, .dos_err = 317, .posix_err = 0, .msg = R"(The resource required for this operation does not exist.)"},
	{.status = 4317, .dos_err = 317, .posix_err = 0, .msg = R"(The operation identifier is not valid.)"},
	{.status = 4318, .dos_err = 317, .posix_err = 0, .msg = R"(The media is not mounted or ready for use.)"},
	{.status = 4319, .dos_err = 317, .posix_err = 0, .msg = R"(The device is not ready for use.)"},
	{.status = 4320, .dos_err = 317, .posix_err = 0, .msg = R"(The operator or administrator has refused the request.)"},
	{.status = 4321, .dos_err = 317, .posix_err = 0, .msg = R"(The drive identifier does not represent a valid drive.)"},
	{.status = 4322, .dos_err = 317, .posix_err = 0, .msg = R"(Library is full. No slot is available for use.)"},
	{.status = 4323, .dos_err = 317, .posix_err = 0, .msg = R"(The transport cannot access the medium.)"},
	{.status = 4324, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to load the medium into the drive.)"},
	{.status = 4325, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to retrieve the drive status.)"},
	{.status = 4326, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to retrieve the slot status.)"},
	{.status = 4327, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to retrieve status about the transport.)"},
	{.status = 4328, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot use the transport because it is already in use.)"},
	{.status = 4329, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to open or close the inject/eject port.)"},
	{.status = 4330, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to eject the medium because it is in a drive.)"},
	{.status = 4331, .dos_err = 317, .posix_err = 0, .msg = R"(A cleaner slot is already reserved.)"},
	{.status = 4332, .dos_err = 317, .posix_err = 0, .msg = R"(A cleaner slot is not reserved.)"},
	{.status = 4333, .dos_err = 317, .posix_err = 0, .msg = R"(The cleaner cartridge has performed the maximum number of drive cleanings.)"},
	{.status = 4334, .dos_err = 317, .posix_err = 0, .msg = R"(Unexpected on-medium identifier.)"},
	{.status = 4335, .dos_err = 317, .posix_err = 0, .msg = R"(The last remaining item in this group or resource cannot be deleted.)"},
	{.status = 4336, .dos_err = 317, .posix_err = 0, .msg = R"(The message provided exceeds the maximum size allowed for this parameter.)"},
	{.status = 4337, .dos_err = 317, .posix_err = 0, .msg = R"(The volume contains system or paging files.)"},
	{.status = 4338, .dos_err = 317, .posix_err = 0, .msg = R"(The media type cannot be removed from this library since at least one drive in the library reports it can support this media type.)"},
	{.status = 4339, .dos_err = 317, .posix_err = 0, .msg = R"(This offline media cannot be mounted on this system since no enabled drives are present which can be used.)"},
	{.status = 4340, .dos_err = 317, .posix_err = 0, .msg = R"(A cleaner cartridge is present in the tape library.)"},
	{.status = 4341, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot use the inject/eject port because it is not empty.)"},
	{.status = 4342, .dos_err = 317, .posix_err = 0, .msg = R"(Error)"},
	{.status = 4343, .dos_err = 317, .posix_err = 0, .msg = R"(OK)"},
	{.status = 4344, .dos_err = 317, .posix_err = 0, .msg = R"(Y)"},
	{.status = 4345, .dos_err = 317, .posix_err = 0, .msg = R"(N)"},
	{.status = 4346, .dos_err = 317, .posix_err = 0, .msg = R"(Any)"},
	{.status = 4347, .dos_err = 317, .posix_err = 0, .msg = R"(A)"},
	{.status = 4348, .dos_err = 317, .posix_err = 0, .msg = R"(P)"},
	{.status = 4349, .dos_err = 317, .posix_err = 0, .msg = R"((not found))"},
	{.status = 4350, .dos_err = 317, .posix_err = 0, .msg = R"(This file is currently not available for use on this computer.)"},
	{.status = 4351, .dos_err = 317, .posix_err = 0, .msg = R"(The remote storage service is not operational at this time.)"},
	{.status = 4352, .dos_err = 317, .posix_err = 0, .msg = R"(The remote storage service encountered a media error.)"},
	{.status = 4353, .dos_err = 317, .posix_err = 0, .msg = R"(Read)"},
	{.status = 4354, .dos_err = 317, .posix_err = 0, .msg = R"(Change)"},
	{.status = 4355, .dos_err = 317, .posix_err = 0, .msg = R"(Full)"},
	{.status = 4356, .dos_err = 317, .posix_err = 0, .msg = R"(Please type the password:)"},
	{.status = 4357, .dos_err = 317, .posix_err = 0, .msg = R"(Type the password for %1:)"},
	{.status = 4358, .dos_err = 317, .posix_err = 0, .msg = R"(Type a password for the user:)"},
	{.status = 4359, .dos_err = 317, .posix_err = 0, .msg = R"(Type the password for the shared resource:)"},
	{.status = 4360, .dos_err = 317, .posix_err = 0, .msg = R"(Type your password:)"},
	{.status = 4361, .dos_err = 317, .posix_err = 0, .msg = R"(Retype the password to confirm:)"},
	{.status = 4362, .dos_err = 317, .posix_err = 0, .msg = R"(Type the user's old password:)"},
	{.status = 4363, .dos_err = 317, .posix_err = 0, .msg = R"(Type the user's new password:)"},
	{.status = 4364, .dos_err = 317, .posix_err = 0, .msg = R"(Type your new password:)"},
	{.status = 4365, .dos_err = 317, .posix_err = 0, .msg = R"(Type the Replicator service password:)"},
	{.status = 4366, .dos_err = 317, .posix_err = 0, .msg = R"(Type your user name, or press ENTER if it is %1:)"},
	{.status = 4367, .dos_err = 317, .posix_err = 0, .msg = R"(Type the domain or server where you want to change a password, or

press ENTER if it is for domain %1:)"},
	{.status = 4368, .dos_err = 317, .posix_err = 0, .msg = R"(Type your user name:)"},
	{.status = 4369, .dos_err = 317, .posix_err = 0, .msg = R"(Network statistics for \\%1)"},
	{.status = 4370, .dos_err = 317, .posix_err = 0, .msg = R"(Printing options for %1)"},
	{.status = 4371, .dos_err = 317, .posix_err = 0, .msg = R"(Communication-device queues accessing %1)"},
	{.status = 4372, .dos_err = 317, .posix_err = 0, .msg = R"(Print job detail)"},
	{.status = 4373, .dos_err = 317, .posix_err = 0, .msg = R"(Communication-device queues at \\%1)"},
	{.status = 4374, .dos_err = 317, .posix_err = 0, .msg = R"(Printers at %1)"},
	{.status = 4375, .dos_err = 317, .posix_err = 0, .msg = R"(Printers accessing %1)"},
	{.status = 4376, .dos_err = 317, .posix_err = 0, .msg = R"(Print jobs at %1:)"},
	{.status = 4377, .dos_err = 317, .posix_err = 0, .msg = R"(Shared resources at %1)"},
	{.status = 4378, .dos_err = 317, .posix_err = 0, .msg = R"(The following running services can be controlled:)"},
	{.status = 4379, .dos_err = 317, .posix_err = 0, .msg = R"(Statistics are available for the following running services:)"},
	{.status = 4380, .dos_err = 317, .posix_err = 0, .msg = R"(User accounts for \\%1)"},
	{.status = 4381, .dos_err = 317, .posix_err = 0, .msg = R"(The syntax of this command is:)"},
	{.status = 4382, .dos_err = 317, .posix_err = 0, .msg = R"(The options of this command are:)"},
	{.status = 4383, .dos_err = 317, .posix_err = 0, .msg = R"(Please enter the name of the Primary Domain Controller:)"},
	{.status = 4384, .dos_err = 317, .posix_err = 0, .msg = R"(The string you have entered is too long. The maximum

is %1, please reenter.)"},
	{.status = 4385, .dos_err = 317, .posix_err = 0, .msg = R"(Sunday)"},
	{.status = 4386, .dos_err = 317, .posix_err = 0, .msg = R"(Monday)"},
	{.status = 4387, .dos_err = 317, .posix_err = 0, .msg = R"(Tuesday)"},
	{.status = 4388, .dos_err = 317, .posix_err = 0, .msg = R"(Wednesday)"},
	{.status = 4389, .dos_err = 317, .posix_err = 0, .msg = R"(Thursday)"},
	{.status = 4390, .dos_err = 317, .posix_err = 0, .msg = R"(The file or directory is not a reparse point.)"},
	{.status = 4391, .dos_err = 317, .posix_err = 0, .msg = R"(The reparse point attribute cannot be set because it conflicts with an existing attribute.)"},
	{.status = 4392, .dos_err = 317, .posix_err = 0, .msg = R"(The data present in the reparse point buffer is invalid.)"},
	{.status = 4393, .dos_err = 317, .posix_err = 0, .msg = R"(The tag present in the reparse point buffer is invalid.)"},
	{.status = 4394, .dos_err = 317, .posix_err = 0, .msg = R"(There is a mismatch between the tag specified in the request and the tag present in the reparse point.)"},
	{.status = 4395, .dos_err = 317, .posix_err = 0, .msg = R"(The object manager encountered a reparse point while retrieving an object.)"},
	{.status = 4396, .dos_err = 317, .posix_err = 0, .msg = R"(Th)"},
	{.status = 4397, .dos_err = 317, .posix_err = 0, .msg = R"(F)"},
	{.status = 4398, .dos_err = 317, .posix_err = 0, .msg = R"(S)"},
	{.status = 4399, .dos_err = 317, .posix_err = 0, .msg = R"(Sa)"},
	{.status = 4400, .dos_err = 317, .posix_err = 0, .msg = R"(Fast Cache data not found.)"},
	{.status = 4401, .dos_err = 317, .posix_err = 0, .msg = R"(Fast Cache data expired.)"},
	{.status = 4402, .dos_err = 317, .posix_err = 0, .msg = R"(Fast Cache data corrupt.)"},
	{.status = 4403, .dos_err = 317, .posix_err = 0, .msg = R"(Fast Cache data has exceeded its max size and cannot be updated.)"},
	{.status = 4404, .dos_err = 317, .posix_err = 0, .msg = R"(Fast Cache has been ReArmed and requires a reboot until it can be updated.)"},
	{.status = 4405, .dos_err = 317, .posix_err = 0, .msg = R"(Aliases for \\%1)"},
	{.status = 4406, .dos_err = 317, .posix_err = 0, .msg = R"(Alias name)"},
	{.status = 4407, .dos_err = 317, .posix_err = 0, .msg = R"(Comment)"},
	{.status = 4408, .dos_err = 317, .posix_err = 0, .msg = R"(Members)"},
	{.status = 4410, .dos_err = 317, .posix_err = 0, .msg = R"(User Accounts for \\%1)"},
	{.status = 4411, .dos_err = 317, .posix_err = 0, .msg = R"(User name)"},
	{.status = 4412, .dos_err = 317, .posix_err = 0, .msg = R"(Full Name)"},
	{.status = 4413, .dos_err = 317, .posix_err = 0, .msg = R"(Comment)"},
	{.status = 4414, .dos_err = 317, .posix_err = 0, .msg = R"(User's comment)"},
	{.status = 4415, .dos_err = 317, .posix_err = 0, .msg = R"(Parameters)"},
	{.status = 4416, .dos_err = 317, .posix_err = 0, .msg = R"(Country/region code)"},
	{.status = 4417, .dos_err = 317, .posix_err = 0, .msg = R"(Privilege level)"},
	{.status = 4418, .dos_err = 317, .posix_err = 0, .msg = R"(Operator privileges)"},
	{.status = 4419, .dos_err = 317, .posix_err = 0, .msg = R"(Account active)"},
	{.status = 4420, .dos_err = 317, .posix_err = 0, .msg = R"(Secure Boot detected that rollback of protected data has been attempted.)"},
	{.status = 4421, .dos_err = 317, .posix_err = 0, .msg = R"(The value is protected by Secure Boot policy and cannot be modified or deleted.)"},
	{.status = 4422, .dos_err = 317, .posix_err = 0, .msg = R"(The Secure Boot policy is invalid.)"},
	{.status = 4423, .dos_err = 317, .posix_err = 0, .msg = R"(A new Secure Boot policy did not contain the current publisher on its update list.)"},
	{.status = 4424, .dos_err = 317, .posix_err = 0, .msg = R"(The Secure Boot policy is either not signed or is signed by a non-trusted signer.)"},
	{.status = 4425, .dos_err = 317, .posix_err = 0, .msg = R"(Secure Boot is not enabled on this machine.)"},
	{.status = 4426, .dos_err = 317, .posix_err = 0, .msg = R"(Secure Boot requires that certain files and drivers are not replaced by other files or drivers.)"},
	{.status = 4427, .dos_err = 317, .posix_err = 0, .msg = R"(The Secure Boot Supplemental Policy file was not authorized on this machine.)"},
	{.status = 4428, .dos_err = 317, .posix_err = 0, .msg = R"(The Supplemental Policy is not recognized on this device.)"},
	{.status = 4429, .dos_err = 317, .posix_err = 0, .msg = R"(The Antirollback version was not found in the Secure Boot Policy.)"},
	{.status = 4430, .dos_err = 317, .posix_err = 0, .msg = R"(The Platform ID specified in the Secure Boot policy does not match the Platform ID on this device.)"},
	{.status = 4431, .dos_err = 317, .posix_err = 0, .msg = R"(The Secure Boot policy file has an older Antirollback Version than this device.)"},
	{.status = 4432, .dos_err = 317, .posix_err = 0, .msg = R"(The Secure Boot policy file does not match the upgraded legacy policy.)"},
	{.status = 4433, .dos_err = 317, .posix_err = 0, .msg = R"(The Secure Boot policy file is required but could not be found.)"},
	{.status = 4434, .dos_err = 317, .posix_err = 0, .msg = R"(Supplemental Secure Boot policy file can not be loaded as a base Secure Boot policy.)"},
	{.status = 4435, .dos_err = 317, .posix_err = 0, .msg = R"(Base Secure Boot policy file can not be loaded as a Supplemental Secure Boot policy.)"},
	{.status = 4436, .dos_err = 317, .posix_err = 0, .msg = R"(Home directory)"},
	{.status = 4437, .dos_err = 317, .posix_err = 0, .msg = R"(Password required)"},
	{.status = 4438, .dos_err = 317, .posix_err = 0, .msg = R"(User may change password)"},
	{.status = 4439, .dos_err = 317, .posix_err = 0, .msg = R"(User profile)"},
	{.status = 4440, .dos_err = 317, .posix_err = 0, .msg = R"(The copy offload read operation is not supported by a filter.)"},
	{.status = 4441, .dos_err = 317, .posix_err = 0, .msg = R"(The copy offload write operation is not supported by a filter.)"},
	{.status = 4442, .dos_err = 317, .posix_err = 0, .msg = R"(The copy offload read operation is not supported for the file.)"},
	{.status = 4443, .dos_err = 317, .posix_err = 0, .msg = R"(The copy offload write operation is not supported for the file.)"},
	{.status = 4444, .dos_err = 317, .posix_err = 0, .msg = R"(This file is currently associated with a different stream id.)"},
	{.status = 4445, .dos_err = 317, .posix_err = 0, .msg = R"(The volume must undergo garbage collection.)"},
	{.status = 4446, .dos_err = 317, .posix_err = 0, .msg = R"(The WOF driver encountered a corruption in WIM image's Header.)"},
	{.status = 4447, .dos_err = 317, .posix_err = 0, .msg = R"(The WOF driver encountered a corruption in WIM image's Resource Table.)"},
	{.status = 4448, .dos_err = 317, .posix_err = 0, .msg = R"(The WOF driver encountered a corruption in the compressed file's Resource Table.)"},
	{.status = 4450, .dos_err = 317, .posix_err = 0, .msg = R"(Computer name)"},
	{.status = 4451, .dos_err = 317, .posix_err = 0, .msg = R"(User name)"},
	{.status = 4452, .dos_err = 317, .posix_err = 0, .msg = R"(Software version)"},
	{.status = 4453, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation active on)"},
	{.status = 4454, .dos_err = 317, .posix_err = 0, .msg = R"(Windows NT root directory)"},
	{.status = 4455, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation domain)"},
	{.status = 4456, .dos_err = 317, .posix_err = 0, .msg = R"(Logon domain)"},
	{.status = 4457, .dos_err = 317, .posix_err = 0, .msg = R"(Other domain(s))"},
	{.status = 4458, .dos_err = 317, .posix_err = 0, .msg = R"(COM Open Timeout (sec))"},
	{.status = 4459, .dos_err = 317, .posix_err = 0, .msg = R"(COM Send Count (byte))"},
	{.status = 4460, .dos_err = 317, .posix_err = 0, .msg = R"(COM Send Timeout (msec))"},
	{.status = 4461, .dos_err = 317, .posix_err = 0, .msg = R"(DOS session print time-out (sec))"},
	{.status = 4462, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum error log size (K))"},
	{.status = 4463, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum cache memory (K))"},
	{.status = 4464, .dos_err = 317, .posix_err = 0, .msg = R"(Number of network buffers)"},
	{.status = 4465, .dos_err = 317, .posix_err = 0, .msg = R"(Number of character buffers)"},
	{.status = 4466, .dos_err = 317, .posix_err = 0, .msg = R"(Size of network buffers)"},
	{.status = 4467, .dos_err = 317, .posix_err = 0, .msg = R"(Size of character buffers)"},
	{.status = 4468, .dos_err = 317, .posix_err = 0, .msg = R"(Full Computer name)"},
	{.status = 4469, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation Domain DNS Name)"},
	{.status = 4470, .dos_err = 317, .posix_err = 0, .msg = R"(Windows 2002)"},
	{.status = 4481, .dos_err = 317, .posix_err = 0, .msg = R"(Server Name)"},
	{.status = 4482, .dos_err = 317, .posix_err = 0, .msg = R"(Server Comment)"},
	{.status = 4483, .dos_err = 317, .posix_err = 0, .msg = R"(Send administrative alerts to)"},
	{.status = 4484, .dos_err = 317, .posix_err = 0, .msg = R"(Software version)"},
	{.status = 4485, .dos_err = 317, .posix_err = 0, .msg = R"(Peer Server)"},
	{.status = 4486, .dos_err = 317, .posix_err = 0, .msg = R"(Windows NT)"},
	{.status = 4487, .dos_err = 317, .posix_err = 0, .msg = R"(Server Level)"},
	{.status = 4488, .dos_err = 317, .posix_err = 0, .msg = R"(Windows NT Server)"},
	{.status = 4489, .dos_err = 317, .posix_err = 0, .msg = R"(Server is active on)"},
	{.status = 4492, .dos_err = 317, .posix_err = 0, .msg = R"(Server hidden)"},
	{.status = 4500, .dos_err = 317, .posix_err = 0, .msg = R"(Single Instance Storage is not available on this volume.)"},
	{.status = 4506, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum Logged On Users)"},
	{.status = 4507, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum concurrent administrators)"},
	{.status = 4508, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum resources shared)"},
	{.status = 4509, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum connections to resources)"},
	{.status = 4510, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum open files on server)"},
	{.status = 4511, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum open files per session)"},
	{.status = 4512, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum file locks)"},
	{.status = 4520, .dos_err = 317, .posix_err = 0, .msg = R"(Idle session time (min))"},
	{.status = 4526, .dos_err = 317, .posix_err = 0, .msg = R"(Share-level)"},
	{.status = 4527, .dos_err = 317, .posix_err = 0, .msg = R"(User-level)"},
	{.status = 4530, .dos_err = 317, .posix_err = 0, .msg = R"(Unlimited Server)"},
	{.status = 4550, .dos_err = 317, .posix_err = 0, .msg = R"(System Integrity detected that policy rollback has been attempted.)"},
	{.status = 4551, .dos_err = 317, .posix_err = 0, .msg = R"(Your organization used Device Guard to block this app. Contact your support person for more info.)"},
	{.status = 4552, .dos_err = 317, .posix_err = 0, .msg = R"(The System Integrity policy is invalid.)"},
	{.status = 4553, .dos_err = 317, .posix_err = 0, .msg = R"(The System Integrity policy is either not signed or is signed by a non-trusted signer.)"},
	{.status = 4554, .dos_err = 317, .posix_err = 0, .msg = R"(The number of System Integrity policies is out of limit.)"},
	{.status = 4555, .dos_err = 317, .posix_err = 0, .msg = R"(The Code Integrity supplemental policy is not authorized by a Code Integrity base policy.)"},
	{.status = 4560, .dos_err = 317, .posix_err = 0, .msg = R"(Virtual Secure Mode (VSM) is not initialized. The hypervisor or VSM may not be present or enabled.)"},
	{.status = 4561, .dos_err = 317, .posix_err = 0, .msg = R"(The hypervisor is not protecting DMA because an IOMMU is not present or not enabled in the BIOS.)"},
	{.status = 4570, .dos_err = 317, .posix_err = 0, .msg = R"(The Platform Manifest file was not authorized on this machine.)"},
	{.status = 4571, .dos_err = 317, .posix_err = 0, .msg = R"(The Platform Manifest file was not valid.)"},
	{.status = 4572, .dos_err = 317, .posix_err = 0, .msg = R"(The file is not authorized on this platform because an entry was not found in the Platform Manifest.)"},
	{.status = 4573, .dos_err = 317, .posix_err = 0, .msg = R"(The catalog is not authorized on this platform because an entry was not found in the Platform Manifest.)"},
	{.status = 4574, .dos_err = 317, .posix_err = 0, .msg = R"(The file is not authorized on this platform because a Binary ID was not found in the embedded signature.)"},
	{.status = 4575, .dos_err = 317, .posix_err = 0, .msg = R"(No active Platform Manifest exists on this system.)"},
	{.status = 4576, .dos_err = 317, .posix_err = 0, .msg = R"(The Platform Manifest file was not properly signed.)"},
	{.status = 4577, .dos_err = 317, .posix_err = 0, .msg = R"(Primary Domain controller for workstation domain:)"},
	{.status = 4578, .dos_err = 317, .posix_err = 0, .msg = R"(Lockout threshold:)"},
	{.status = 4579, .dos_err = 317, .posix_err = 0, .msg = R"(Lockout duration (minutes):)"},
	{.status = 4580, .dos_err = 317, .posix_err = 0, .msg = R"(Lockout observation window (minutes):)"},
	{.status = 4600, .dos_err = 317, .posix_err = 0, .msg = R"(Statistics since)"},
	{.status = 4601, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions accepted)"},
	{.status = 4602, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions timed-out)"},
	{.status = 4603, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions errored-out)"},
	{.status = 4604, .dos_err = 317, .posix_err = 0, .msg = R"(Kilobytes sent)"},
	{.status = 4605, .dos_err = 317, .posix_err = 0, .msg = R"(Kilobytes received)"},
	{.status = 4606, .dos_err = 317, .posix_err = 0, .msg = R"(Mean response time (msec))"},
	{.status = 4607, .dos_err = 317, .posix_err = 0, .msg = R"(Network errors)"},
	{.status = 4608, .dos_err = 317, .posix_err = 0, .msg = R"(Files accessed)"},
	{.status = 4609, .dos_err = 317, .posix_err = 0, .msg = R"(Print jobs spooled)"},
	{.status = 4610, .dos_err = 317, .posix_err = 0, .msg = R"(System errors)"},
	{.status = 4611, .dos_err = 317, .posix_err = 0, .msg = R"(Password violations)"},
	{.status = 4612, .dos_err = 317, .posix_err = 0, .msg = R"(Permission violations)"},
	{.status = 4613, .dos_err = 317, .posix_err = 0, .msg = R"(Communication devices accessed)"},
	{.status = 4614, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions started)"},
	{.status = 4615, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions reconnected)"},
	{.status = 4616, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions starts failed)"},
	{.status = 4617, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions disconnected)"},
	{.status = 4618, .dos_err = 317, .posix_err = 0, .msg = R"(Network I/O's performed)"},
	{.status = 4619, .dos_err = 317, .posix_err = 0, .msg = R"(Files and pipes accessed)"},
	{.status = 4620, .dos_err = 317, .posix_err = 0, .msg = R"(Times buffers exhausted)"},
	{.status = 4621, .dos_err = 317, .posix_err = 0, .msg = R"(Big buffers)"},
	{.status = 4622, .dos_err = 317, .posix_err = 0, .msg = R"(Request buffers)"},
	{.status = 4623, .dos_err = 317, .posix_err = 0, .msg = R"(Workstation Statistics for \\%1)"},
	{.status = 4624, .dos_err = 317, .posix_err = 0, .msg = R"(Server Statistics for \\%1)"},
	{.status = 4625, .dos_err = 317, .posix_err = 0, .msg = R"(Statistics since %1)"},
	{.status = 4626, .dos_err = 317, .posix_err = 0, .msg = R"(Connections made)"},
	{.status = 4627, .dos_err = 317, .posix_err = 0, .msg = R"(Connections failed)"},
	{.status = 4630, .dos_err = 317, .posix_err = 0, .msg = R"(Bytes received)"},
	{.status = 4631, .dos_err = 317, .posix_err = 0, .msg = R"(Server Message Blocks (SMBs) received)"},
	{.status = 4632, .dos_err = 317, .posix_err = 0, .msg = R"(Bytes transmitted)"},
	{.status = 4633, .dos_err = 317, .posix_err = 0, .msg = R"(Server Message Blocks (SMBs) transmitted)"},
	{.status = 4634, .dos_err = 317, .posix_err = 0, .msg = R"(Read operations)"},
	{.status = 4635, .dos_err = 317, .posix_err = 0, .msg = R"(Write operations)"},
	{.status = 4636, .dos_err = 317, .posix_err = 0, .msg = R"(Raw reads denied)"},
	{.status = 4637, .dos_err = 317, .posix_err = 0, .msg = R"(Raw writes denied)"},
	{.status = 4638, .dos_err = 317, .posix_err = 0, .msg = R"(Network errors)"},
	{.status = 4639, .dos_err = 317, .posix_err = 0, .msg = R"(Connections made)"},
	{.status = 4640, .dos_err = 317, .posix_err = 0, .msg = R"(Reconnections made)"},
	{.status = 4641, .dos_err = 317, .posix_err = 0, .msg = R"(Server disconnects)"},
	{.status = 4642, .dos_err = 317, .posix_err = 0, .msg = R"(Sessions started)"},
	{.status = 4643, .dos_err = 317, .posix_err = 0, .msg = R"(Hung sessions)"},
	{.status = 4644, .dos_err = 317, .posix_err = 0, .msg = R"(Failed sessions)"},
	{.status = 4645, .dos_err = 317, .posix_err = 0, .msg = R"(Failed operations)"},
	{.status = 4646, .dos_err = 317, .posix_err = 0, .msg = R"(Use count)"},
	{.status = 4647, .dos_err = 317, .posix_err = 0, .msg = R"(Failed use count)"},
	{.status = 4650, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was deleted successfully.)"},
	{.status = 4651, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was used successfully.)"},
	{.status = 4652, .dos_err = 317, .posix_err = 0, .msg = R"(The message was successfully sent to %1.)"},
	{.status = 4653, .dos_err = 317, .posix_err = 0, .msg = R"(The message name %1 was forwarded successfully.)"},
	{.status = 4654, .dos_err = 317, .posix_err = 0, .msg = R"(The message name %1 was added successfully.)"},
	{.status = 4655, .dos_err = 317, .posix_err = 0, .msg = R"(The message name forwarding was successfully canceled.)"},
	{.status = 4656, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was shared successfully.)"},
	{.status = 4657, .dos_err = 317, .posix_err = 0, .msg = R"(The server %1 successfully logged you on as %2.)"},
	{.status = 4658, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was logged off successfully.)"},
	{.status = 4659, .dos_err = 317, .posix_err = 0, .msg = R"(%1 was successfully removed from the list of shares the Server creates

on startup.)"},
	{.status = 4661, .dos_err = 317, .posix_err = 0, .msg = R"(The password was changed successfully.)"},
	{.status = 4662, .dos_err = 317, .posix_err = 0, .msg = R"(%1 file(s) copied.)"},
	{.status = 4663, .dos_err = 317, .posix_err = 0, .msg = R"(%1 file(s) moved.)"},
	{.status = 4664, .dos_err = 317, .posix_err = 0, .msg = R"(The message was successfully sent to all users of the network.)"},
	{.status = 4665, .dos_err = 317, .posix_err = 0, .msg = R"(The message was successfully sent to domain %1.)"},
	{.status = 4666, .dos_err = 317, .posix_err = 0, .msg = R"(The message was successfully sent to all users of this server.)"},
	{.status = 4667, .dos_err = 317, .posix_err = 0, .msg = R"(The message was successfully sent to group *%1.)"},
	{.status = 4695, .dos_err = 317, .posix_err = 0, .msg = R"(Microsoft LAN Manager Version %1)"},
	{.status = 4696, .dos_err = 317, .posix_err = 0, .msg = R"(Windows NT Server)"},
	{.status = 4697, .dos_err = 317, .posix_err = 0, .msg = R"(Windows NT Workstation)"},
	{.status = 4698, .dos_err = 317, .posix_err = 0, .msg = R"(MS-DOS Enhanced Workstation)"},
	{.status = 4699, .dos_err = 317, .posix_err = 0, .msg = R"(Created at %1)"},
	{.status = 4700, .dos_err = 317, .posix_err = 0, .msg = R"(Server Name            Remark)"},
	{.status = 4701, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot enumerate servers in non-default compartment.)"},
	{.status = 4702, .dos_err = 317, .posix_err = 0, .msg = R"((UNC))"},
	{.status = 4703, .dos_err = 317, .posix_err = 0, .msg = R"(...)"},
	{.status = 4704, .dos_err = 317, .posix_err = 0, .msg = R"(Domain)"},
	{.status = 4705, .dos_err = 317, .posix_err = 0, .msg = R"(Resources on %1)"},
	{.status = 4706, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid network provider.  Available networks are:)"},
	{.status = 4710, .dos_err = 317, .posix_err = 0, .msg = R"(Disk)"},
	{.status = 4711, .dos_err = 317, .posix_err = 0, .msg = R"(Print)"},
	{.status = 4712, .dos_err = 317, .posix_err = 0, .msg = R"(Comm)"},
	{.status = 4713, .dos_err = 317, .posix_err = 0, .msg = R"(IPC)"},
	{.status = 4714, .dos_err = 317, .posix_err = 0, .msg = R"(Status       Local     Remote                    Network)"},
	{.status = 4715, .dos_err = 317, .posix_err = 0, .msg = R"(OK)"},
	{.status = 4716, .dos_err = 317, .posix_err = 0, .msg = R"(Dormant)"},
	{.status = 4717, .dos_err = 317, .posix_err = 0, .msg = R"(Paused)"},
	{.status = 4718, .dos_err = 317, .posix_err = 0, .msg = R"(Disconnected)"},
	{.status = 4719, .dos_err = 317, .posix_err = 0, .msg = R"(Error)"},
	{.status = 4720, .dos_err = 317, .posix_err = 0, .msg = R"(Connecting)"},
	{.status = 4721, .dos_err = 317, .posix_err = 0, .msg = R"(Reconnecting)"},
	{.status = 4722, .dos_err = 317, .posix_err = 0, .msg = R"(Status)"},
	{.status = 4723, .dos_err = 317, .posix_err = 0, .msg = R"(Local name)"},
	{.status = 4724, .dos_err = 317, .posix_err = 0, .msg = R"(Remote name)"},
	{.status = 4725, .dos_err = 317, .posix_err = 0, .msg = R"(Resource type)"},
	{.status = 4726, .dos_err = 317, .posix_err = 0, .msg = R"(# Opens)"},
	{.status = 4727, .dos_err = 317, .posix_err = 0, .msg = R"(# Connections)"},
	{.status = 4728, .dos_err = 317, .posix_err = 0, .msg = R"(Unavailable)"},
	{.status = 4730, .dos_err = 317, .posix_err = 0, .msg = R"(Share name   Resource                        Remark)"},
	{.status = 4731, .dos_err = 317, .posix_err = 0, .msg = R"(Share name)"},
	{.status = 4732, .dos_err = 317, .posix_err = 0, .msg = R"(Resource)"},
	{.status = 4733, .dos_err = 317, .posix_err = 0, .msg = R"(Spooled)"},
	{.status = 4734, .dos_err = 317, .posix_err = 0, .msg = R"(Permission)"},
	{.status = 4735, .dos_err = 317, .posix_err = 0, .msg = R"(Maximum users)"},
	{.status = 4736, .dos_err = 317, .posix_err = 0, .msg = R"(No limit)"},
	{.status = 4737, .dos_err = 317, .posix_err = 0, .msg = R"(Users)"},
	{.status = 4738, .dos_err = 317, .posix_err = 0, .msg = R"(The share name entered may not be accessible from some MS-DOS workstations.

Are you sure you want to use this share name? %1:)"},
	{.status = 4739, .dos_err = 317, .posix_err = 0, .msg = R"(Caching)"},
	{.status = 4740, .dos_err = 317, .posix_err = 0, .msg = R"(ID         Path                                    User name            # Locks)"},
	{.status = 4741, .dos_err = 317, .posix_err = 0, .msg = R"(File ID)"},
	{.status = 4742, .dos_err = 317, .posix_err = 0, .msg = R"(Locks)"},
	{.status = 4743, .dos_err = 317, .posix_err = 0, .msg = R"(Permissions)"},
	{.status = 4744, .dos_err = 317, .posix_err = 0, .msg = R"(Share name)"},
	{.status = 4745, .dos_err = 317, .posix_err = 0, .msg = R"(Type)"},
	{.status = 4746, .dos_err = 317, .posix_err = 0, .msg = R"(Used as)"},
	{.status = 4747, .dos_err = 317, .posix_err = 0, .msg = R"(Comment)"},
	{.status = 4750, .dos_err = 317, .posix_err = 0, .msg = R"(Computer               User name            Client Type       Opens Idle time)"},
	{.status = 4751, .dos_err = 317, .posix_err = 0, .msg = R"(Computer)"},
	{.status = 4752, .dos_err = 317, .posix_err = 0, .msg = R"(Sess time)"},
	{.status = 4753, .dos_err = 317, .posix_err = 0, .msg = R"(Idle time)"},
	{.status = 4754, .dos_err = 317, .posix_err = 0, .msg = R"(Share name     Type     # Opens)"},
	{.status = 4755, .dos_err = 317, .posix_err = 0, .msg = R"(Client type)"},
	{.status = 4756, .dos_err = 317, .posix_err = 0, .msg = R"(Guest logon)"},
	{.status = 4770, .dos_err = 317, .posix_err = 0, .msg = R"(Manual caching of documents)"},
	{.status = 4771, .dos_err = 317, .posix_err = 0, .msg = R"(Automatic caching of documents)"},
	{.status = 4772, .dos_err = 317, .posix_err = 0, .msg = R"(Automatic caching of programs and documents)"},
	{.status = 4773, .dos_err = 317, .posix_err = 0, .msg = R"(Manual caching of documents with BranchCache enabled)"},
	{.status = 4774, .dos_err = 317, .posix_err = 0, .msg = R"(Caching disabled)"},
	{.status = 4775, .dos_err = 317, .posix_err = 0, .msg = R"(Automatic)"},
	{.status = 4776, .dos_err = 317, .posix_err = 0, .msg = R"(Manual)"},
	{.status = 4777, .dos_err = 317, .posix_err = 0, .msg = R"(Documents)"},
	{.status = 4778, .dos_err = 317, .posix_err = 0, .msg = R"(Programs)"},
	{.status = 4779, .dos_err = 317, .posix_err = 0, .msg = R"(BranchCache)"},
	{.status = 4780, .dos_err = 317, .posix_err = 0, .msg = R"(None)"},
	{.status = 4800, .dos_err = 317, .posix_err = 0, .msg = R"(Name)"},
	{.status = 4801, .dos_err = 317, .posix_err = 0, .msg = R"(Forwarded to)"},
	{.status = 4802, .dos_err = 317, .posix_err = 0, .msg = R"(Forwarded to you from)"},
	{.status = 4803, .dos_err = 317, .posix_err = 0, .msg = R"(Users of this server)"},
	{.status = 4804, .dos_err = 317, .posix_err = 0, .msg = R"(Net Send has been interrupted by a Ctrl+Break from the user.)"},
	{.status = 4810, .dos_err = 317, .posix_err = 0, .msg = R"(Name                         Job #      Size            Status)"},
	{.status = 4811, .dos_err = 317, .posix_err = 0, .msg = R"(jobs)"},
	{.status = 4812, .dos_err = 317, .posix_err = 0, .msg = R"(Print)"},
	{.status = 4813, .dos_err = 317, .posix_err = 0, .msg = R"(Name)"},
	{.status = 4814, .dos_err = 317, .posix_err = 0, .msg = R"(Job #)"},
	{.status = 4815, .dos_err = 317, .posix_err = 0, .msg = R"(Size)"},
	{.status = 4816, .dos_err = 317, .posix_err = 0, .msg = R"(Status)"},
	{.status = 4817, .dos_err = 317, .posix_err = 0, .msg = R"(Separator file)"},
	{.status = 4818, .dos_err = 317, .posix_err = 0, .msg = R"(Comment)"},
	{.status = 4819, .dos_err = 317, .posix_err = 0, .msg = R"(Priority)"},
	{.status = 4820, .dos_err = 317, .posix_err = 0, .msg = R"(Print after)"},
	{.status = 4821, .dos_err = 317, .posix_err = 0, .msg = R"(Print until)"},
	{.status = 4822, .dos_err = 317, .posix_err = 0, .msg = R"(Print processor)"},
	{.status = 4823, .dos_err = 317, .posix_err = 0, .msg = R"(Additional info)"},
	{.status = 4824, .dos_err = 317, .posix_err = 0, .msg = R"(Parameters)"},
	{.status = 4825, .dos_err = 317, .posix_err = 0, .msg = R"(Print Devices)"},
	{.status = 4826, .dos_err = 317, .posix_err = 0, .msg = R"(Printer Active)"},
	{.status = 4827, .dos_err = 317, .posix_err = 0, .msg = R"(Printer held)"},
	{.status = 4828, .dos_err = 317, .posix_err = 0, .msg = R"(Printer error)"},
	{.status = 4829, .dos_err = 317, .posix_err = 0, .msg = R"(Printer being deleted)"},
	{.status = 4830, .dos_err = 317, .posix_err = 0, .msg = R"(Printer status unknown)"},
	{.status = 4840, .dos_err = 317, .posix_err = 0, .msg = R"(Held until %1)"},
	{.status = 4841, .dos_err = 317, .posix_err = 0, .msg = R"(Job #)"},
	{.status = 4842, .dos_err = 317, .posix_err = 0, .msg = R"(Submitting user)"},
	{.status = 4843, .dos_err = 317, .posix_err = 0, .msg = R"(Notify)"},
	{.status = 4844, .dos_err = 317, .posix_err = 0, .msg = R"(Job data type)"},
	{.status = 4845, .dos_err = 317, .posix_err = 0, .msg = R"(Job parameters)"},
	{.status = 4846, .dos_err = 317, .posix_err = 0, .msg = R"(Waiting)"},
	{.status = 4847, .dos_err = 317, .posix_err = 0, .msg = R"(Held in queue)"},
	{.status = 4848, .dos_err = 317, .posix_err = 0, .msg = R"(Spooling)"},
	{.status = 4849, .dos_err = 317, .posix_err = 0, .msg = R"(Paused)"},
	{.status = 4850, .dos_err = 317, .posix_err = 0, .msg = R"(Offline)"},
	{.status = 4851, .dos_err = 317, .posix_err = 0, .msg = R"(Error)"},
	{.status = 4852, .dos_err = 317, .posix_err = 0, .msg = R"(Out of paper)"},
	{.status = 4853, .dos_err = 317, .posix_err = 0, .msg = R"(Intervention required)"},
	{.status = 4854, .dos_err = 317, .posix_err = 0, .msg = R"(Printing)"},
	{.status = 4855, .dos_err = 317, .posix_err = 0, .msg = R"(on)"},
	{.status = 4856, .dos_err = 317, .posix_err = 0, .msg = R"(Paused on %1)"},
	{.status = 4857, .dos_err = 317, .posix_err = 0, .msg = R"(Offline on %1)"},
	{.status = 4858, .dos_err = 317, .posix_err = 0, .msg = R"(Error on%1)"},
	{.status = 4859, .dos_err = 317, .posix_err = 0, .msg = R"(Out of Paper on %1)"},
	{.status = 4860, .dos_err = 317, .posix_err = 0, .msg = R"(Check printer on %1)"},
	{.status = 4861, .dos_err = 317, .posix_err = 0, .msg = R"(Printing on %1)"},
	{.status = 4862, .dos_err = 317, .posix_err = 0, .msg = R"(Driver)"},
	{.status = 4930, .dos_err = 317, .posix_err = 0, .msg = R"(User name              Type                 Date)"},
	{.status = 4931, .dos_err = 317, .posix_err = 0, .msg = R"(Lockout)"},
	{.status = 4932, .dos_err = 317, .posix_err = 0, .msg = R"(Service)"},
	{.status = 4933, .dos_err = 317, .posix_err = 0, .msg = R"(Server)"},
	{.status = 4934, .dos_err = 317, .posix_err = 0, .msg = R"(Server started)"},
	{.status = 4935, .dos_err = 317, .posix_err = 0, .msg = R"(Server paused)"},
	{.status = 4936, .dos_err = 317, .posix_err = 0, .msg = R"(Server continued)"},
	{.status = 4937, .dos_err = 317, .posix_err = 0, .msg = R"(Server stopped)"},
	{.status = 4938, .dos_err = 317, .posix_err = 0, .msg = R"(Session)"},
	{.status = 4939, .dos_err = 317, .posix_err = 0, .msg = R"(Logon Guest)"},
	{.status = 4940, .dos_err = 317, .posix_err = 0, .msg = R"(Logon User)"},
	{.status = 4941, .dos_err = 317, .posix_err = 0, .msg = R"(Logon Administrator)"},
	{.status = 4942, .dos_err = 317, .posix_err = 0, .msg = R"(Logoff normal)"},
	{.status = 4943, .dos_err = 317, .posix_err = 0, .msg = R"(Logon)"},
	{.status = 4944, .dos_err = 317, .posix_err = 0, .msg = R"(Logoff error)"},
	{.status = 4945, .dos_err = 317, .posix_err = 0, .msg = R"(Logoff auto-disconnect)"},
	{.status = 4946, .dos_err = 317, .posix_err = 0, .msg = R"(Logoff administrator-disconnect)"},
	{.status = 4947, .dos_err = 317, .posix_err = 0, .msg = R"(Logoff forced by logon restrictions)"},
	{.status = 4948, .dos_err = 317, .posix_err = 0, .msg = R"(Service)"},
	{.status = 4949, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Installed)"},
	{.status = 4950, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Install Pending)"},
	{.status = 4951, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Paused)"},
	{.status = 4952, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Pause Pending)"},
	{.status = 4953, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Continued)"},
	{.status = 4954, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Continue Pending)"},
	{.status = 4955, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Stopped)"},
	{.status = 4956, .dos_err = 317, .posix_err = 0, .msg = R"(%1 Stop Pending)"},
	{.status = 4957, .dos_err = 317, .posix_err = 0, .msg = R"(Account)"},
	{.status = 4958, .dos_err = 317, .posix_err = 0, .msg = R"(User account %1 was modified.)"},
	{.status = 4959, .dos_err = 317, .posix_err = 0, .msg = R"(Group account %1 was modified.)"},
	{.status = 4960, .dos_err = 317, .posix_err = 0, .msg = R"(User account %1 was deleted)"},
	{.status = 4961, .dos_err = 317, .posix_err = 0, .msg = R"(Group account %1 was deleted)"},
	{.status = 4962, .dos_err = 317, .posix_err = 0, .msg = R"(User account %1 was added)"},
	{.status = 4963, .dos_err = 317, .posix_err = 0, .msg = R"(Group account %1 was added)"},
	{.status = 4964, .dos_err = 317, .posix_err = 0, .msg = R"(Account system settings were modified)"},
	{.status = 4965, .dos_err = 317, .posix_err = 0, .msg = R"(Logon restriction)"},
	{.status = 4966, .dos_err = 317, .posix_err = 0, .msg = R"(Limit exceeded:  UNKNOWN)"},
	{.status = 4967, .dos_err = 317, .posix_err = 0, .msg = R"(Limit exceeded:  Logon hours)"},
	{.status = 4968, .dos_err = 317, .posix_err = 0, .msg = R"(Limit exceeded:  Account expired)"},
	{.status = 4969, .dos_err = 317, .posix_err = 0, .msg = R"(Limit exceeded:  Workstation ID invalid)"},
	{.status = 4970, .dos_err = 317, .posix_err = 0, .msg = R"(Limit exceeded:  Account disabled)"},
	{.status = 4971, .dos_err = 317, .posix_err = 0, .msg = R"(Limit exceeded:  Account deleted)"},
	{.status = 4972, .dos_err = 317, .posix_err = 0, .msg = R"(Share)"},
	{.status = 4973, .dos_err = 317, .posix_err = 0, .msg = R"(Use %1)"},
	{.status = 4974, .dos_err = 317, .posix_err = 0, .msg = R"(Unuse %1)"},
	{.status = 4975, .dos_err = 317, .posix_err = 0, .msg = R"(User's session disconnected %1)"},
	{.status = 4976, .dos_err = 317, .posix_err = 0, .msg = R"(Administrator stopped sharing resource %1)"},
	{.status = 4977, .dos_err = 317, .posix_err = 0, .msg = R"(User reached limit for %1)"},
	{.status = 4978, .dos_err = 317, .posix_err = 0, .msg = R"(Bad password)"},
	{.status = 4979, .dos_err = 317, .posix_err = 0, .msg = R"(Administrator privilege required)"},
	{.status = 4980, .dos_err = 317, .posix_err = 0, .msg = R"(Access)"},
	{.status = 4981, .dos_err = 317, .posix_err = 0, .msg = R"(%1 permissions added)"},
	{.status = 4982, .dos_err = 317, .posix_err = 0, .msg = R"(%1 permissions modified)"},
	{.status = 4983, .dos_err = 317, .posix_err = 0, .msg = R"(%1 permissions deleted)"},
	{.status = 4984, .dos_err = 317, .posix_err = 0, .msg = R"(Access denied)"},
	{.status = 4985, .dos_err = 317, .posix_err = 0, .msg = R"(Unknown)"},
	{.status = 4986, .dos_err = 317, .posix_err = 0, .msg = R"(Other)"},
	{.status = 4987, .dos_err = 317, .posix_err = 0, .msg = R"(Duration:)"},
	{.status = 4988, .dos_err = 317, .posix_err = 0, .msg = R"(Duration: Not available)"},
	{.status = 4989, .dos_err = 317, .posix_err = 0, .msg = R"(Duration: Less than one second)"},
	{.status = 4990, .dos_err = 317, .posix_err = 0, .msg = R"((none))"},
	{.status = 4991, .dos_err = 317, .posix_err = 0, .msg = R"(Closed %1)"},
	{.status = 4992, .dos_err = 317, .posix_err = 0, .msg = R"(Closed %1 (disconnected))"},
	{.status = 4993, .dos_err = 317, .posix_err = 0, .msg = R"(Administrator closed %1)"},
	{.status = 4994, .dos_err = 317, .posix_err = 0, .msg = R"(Access ended)"},
	{.status = 4995, .dos_err = 317, .posix_err = 0, .msg = R"(Log on to network)"},
	{.status = 4996, .dos_err = 317, .posix_err = 0, .msg = R"(Logon denied)"},
	{.status = 4997, .dos_err = 317, .posix_err = 0, .msg = R"(Program             Message             Time)"},
	{.status = 4998, .dos_err = 317, .posix_err = 0, .msg = R"(Account locked due to %1 bad passwords)"},
	{.status = 4999, .dos_err = 317, .posix_err = 0, .msg = R"(Account unlocked by administrator)"},
	{.status = 5000, .dos_err = 317, .posix_err = 0, .msg = R"(Log off network)"},
	{.status = 5001, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be completed because other resources are dependent on this resource.)"},
	{.status = 5002, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource dependency cannot be found.)"},
	{.status = 5003, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource cannot be made dependent on the specified resource because it is already dependent.)"},
	{.status = 5004, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource is not online.)"},
	{.status = 5005, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster node is not available for this operation.)"},
	{.status = 5006, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource is not available.)"},
	{.status = 5007, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource could not be found.)"},
	{.status = 5008, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster is being shut down.)"},
	{.status = 5009, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster node cannot be evicted from the cluster unless the node is down or it is the last node.)"},
	{.status = 5010, .dos_err = 317, .posix_err = 0, .msg = R"(The object already exists.)"},
	{.status = 5011, .dos_err = 317, .posix_err = 0, .msg = R"(The object is already in the list.)"},
	{.status = 5012, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster group is not available for any new requests.)"},
	{.status = 5013, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster group could not be found.)"},
	{.status = 5014, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed because the cluster group is not online.)"},
	{.status = 5015, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because either the specified cluster node is not the owner of the resource, or the node is not a possible owner of the resource.)"},
	{.status = 5016, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because either the specified cluster node is not the owner of the group, or the node is not a possible owner of the group.)"},
	{.status = 5017, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource could not be created in the specified resource monitor.)"},
	{.status = 5018, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource could not be brought online by the resource monitor.)"},
	{.status = 5019, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed because the cluster resource is online.)"},
	{.status = 5020, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource could not be deleted or brought offline because it is the quorum resource.)"},
	{.status = 5021, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster could not make the specified resource a quorum resource because it is not capable of being a quorum resource.)"},
	{.status = 5022, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster software is shutting down.)"},
	{.status = 5023, .dos_err = 317, .posix_err = 0, .msg = R"(The group or resource is not in the correct state to perform the requested operation.)"},
	{.status = 5024, .dos_err = 317, .posix_err = 0, .msg = R"(The properties were stored but not all changes will take effect until the next time the resource is brought online.)"},
	{.status = 5025, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster could not make the specified resource a quorum resource because it does not belong to a shared storage class.)"},
	{.status = 5026, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource could not be deleted since it is a core resource.)"},
	{.status = 5027, .dos_err = 317, .posix_err = 0, .msg = R"(The quorum resource failed to come online.)"},
	{.status = 5028, .dos_err = 317, .posix_err = 0, .msg = R"(The quorum log could not be created or mounted successfully.)"},
	{.status = 5029, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster log is corrupt.)"},
	{.status = 5030, .dos_err = 317, .posix_err = 0, .msg = R"(The record could not be written to the cluster log since it exceeds the maximum size.)"},
	{.status = 5031, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster log exceeds its maximum size.)"},
	{.status = 5032, .dos_err = 317, .posix_err = 0, .msg = R"(No checkpoint record was found in the cluster log.)"},
	{.status = 5033, .dos_err = 317, .posix_err = 0, .msg = R"(The minimum required disk space needed for logging is not available.)"},
	{.status = 5034, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node failed to take control of the quorum resource because the resource is owned by another active node.)"},
	{.status = 5035, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster network is not available for this operation.)"},
	{.status = 5036, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster node is not available for this operation.)"},
	{.status = 5037, .dos_err = 317, .posix_err = 0, .msg = R"(All cluster nodes must be running to perform this operation.)"},
	{.status = 5038, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster resource failed.)"},
	{.status = 5039, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is not valid.)"},
	{.status = 5040, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node already exists.)"},
	{.status = 5041, .dos_err = 317, .posix_err = 0, .msg = R"(A node is in the process of joining the cluster.)"},
	{.status = 5042, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node was not found.)"},
	{.status = 5043, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster local node information was not found.)"},
	{.status = 5044, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network already exists.)"},
	{.status = 5045, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network was not found.)"},
	{.status = 5046, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network interface already exists.)"},
	{.status = 5047, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network interface was not found.)"},
	{.status = 5048, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster request is not valid for this object.)"},
	{.status = 5049, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network provider is not valid.)"},
	{.status = 5050, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is down.)"},
	{.status = 5051, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is not reachable.)"},
	{.status = 5052, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is not a member of the cluster.)"},
	{.status = 5053, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster join operation is not in progress.)"},
	{.status = 5054, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is not valid.)"},
	{.status = 5055, .dos_err = 317, .posix_err = 0, .msg = R"(Mar)"},
	{.status = 5056, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is up.)"},
	{.status = 5057, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster IP address is already in use.)"},
	{.status = 5058, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is not paused.)"},
	{.status = 5059, .dos_err = 317, .posix_err = 0, .msg = R"(No cluster security context is available.)"},
	{.status = 5060, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is not configured for internal cluster communication.)"},
	{.status = 5061, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is already up.)"},
	{.status = 5062, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is already down.)"},
	{.status = 5063, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is already online.)"},
	{.status = 5064, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is already offline.)"},
	{.status = 5065, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is already a member of the cluster.)"},
	{.status = 5066, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is the only one configured for internal cluster communication between two or more active cluster nodes. The internal communication capability cannot be removed from the network.)"},
	{.status = 5067, .dos_err = 317, .posix_err = 0, .msg = R"(One or more cluster resources depend on the network to provide service to clients. The client access capability cannot be removed from the network.)"},
	{.status = 5068, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot currently be performed on the cluster group containing the quorum resource.)"},
	{.status = 5069, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster quorum resource is not allowed to have any dependencies.)"},
	{.status = 5070, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is paused.)"},
	{.status = 5071, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster resource cannot be brought online. The owner node cannot run this resource.)"},
	{.status = 5072, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is not ready to perform the requested operation.)"},
	{.status = 5073, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node is shutting down.)"},
	{.status = 5074, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster join operation was aborted.)"},
	{.status = 5075, .dos_err = 317, .posix_err = 0, .msg = R"(The node failed to join the cluster because the joining node and other nodes in the cluster have incompatible operating system versions. To get more information about operating system versions of the cluster, run the Validate a Configuration Wizard or the Test-Cluster Windows PowerShell cmdlet.)"},
	{.status = 5076, .dos_err = 317, .posix_err = 0, .msg = R"(This resource cannot be created because the cluster has reached the limit on the number of resources it can monitor.)"},
	{.status = 5077, .dos_err = 317, .posix_err = 0, .msg = R"(The system configuration changed during the cluster join or form operation. The join or form operation was aborted.)"},
	{.status = 5078, .dos_err = 317, .posix_err = 0, .msg = R"(The specified resource type was not found.)"},
	{.status = 5079, .dos_err = 317, .posix_err = 0, .msg = R"(The specified node does not support a resource of this type. This may be due to version inconsistencies or due to the absence of the resource DLL on this node.)"},
	{.status = 5080, .dos_err = 317, .posix_err = 0, .msg = R"(The specified resource name is not supported by this resource DLL. This may be due to a bad (or changed) name supplied to the resource DLL.)"},
	{.status = 5081, .dos_err = 317, .posix_err = 0, .msg = R"(No authentication package could be registered with the RPC server.)"},
	{.status = 5082, .dos_err = 317, .posix_err = 0, .msg = R"(You cannot bring the group online because the owner of the group is not in the preferred list for the group. To change the owner node for the group, move the group.)"},
	{.status = 5083, .dos_err = 317, .posix_err = 0, .msg = R"(The join operation failed because the cluster database sequence number has changed or is incompatible with the locker node. This may happen during a join operation if the cluster database was changing during the join.)"},
	{.status = 5084, .dos_err = 317, .posix_err = 0, .msg = R"(The resource monitor will not allow the fail operation to be performed while the resource is in its current state. This may happen if the resource is in a pending state.)"},
	{.status = 5085, .dos_err = 317, .posix_err = 0, .msg = R"(A non locker code got a request to reserve the lock for making global updates.)"},
	{.status = 5086, .dos_err = 317, .posix_err = 0, .msg = R"(The quorum disk could not be located by the cluster service.)"},
	{.status = 5087, .dos_err = 317, .posix_err = 0, .msg = R"(The backed up cluster database is possibly corrupt.)"},
	{.status = 5088, .dos_err = 317, .posix_err = 0, .msg = R"(A DFS root already exists in this cluster node.)"},
	{.status = 5089, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to modify a resource property failed because it conflicts with another existing property.)"},
	{.status = 5090, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not supported on a cluster without an Administrator Access Point.)"},
	{.status = 5091, .dos_err = 317, .posix_err = 0, .msg = R"(Denmark)"},
	{.status = 5092, .dos_err = 317, .posix_err = 0, .msg = R"(Sweden)"},
	{.status = 5093, .dos_err = 317, .posix_err = 0, .msg = R"(Norway)"},
	{.status = 5094, .dos_err = 317, .posix_err = 0, .msg = R"(Germany)"},
	{.status = 5095, .dos_err = 317, .posix_err = 0, .msg = R"(Australia)"},
	{.status = 5096, .dos_err = 317, .posix_err = 0, .msg = R"(Japan)"},
	{.status = 5097, .dos_err = 317, .posix_err = 0, .msg = R"(Korea)"},
	{.status = 5098, .dos_err = 317, .posix_err = 0, .msg = R"(China (PRC))"},
	{.status = 5099, .dos_err = 317, .posix_err = 0, .msg = R"(Taiwan)"},
	{.status = 5100, .dos_err = 317, .posix_err = 0, .msg = R"(Asia)"},
	{.status = 5101, .dos_err = 317, .posix_err = 0, .msg = R"(Portugal)"},
	{.status = 5102, .dos_err = 317, .posix_err = 0, .msg = R"(Finland)"},
	{.status = 5103, .dos_err = 317, .posix_err = 0, .msg = R"(Arabic)"},
	{.status = 5104, .dos_err = 317, .posix_err = 0, .msg = R"(Hebrew)"},
	{.status = 5150, .dos_err = 317, .posix_err = 0, .msg = R"(A power failure has occurred at %1.  Please terminate all activity with this server.)"},
	{.status = 5151, .dos_err = 317, .posix_err = 0, .msg = R"(Power has been restored at %1.  Normal operations have resumed.)"},
	{.status = 5152, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service is starting shut down at %1.)"},
	{.status = 5153, .dos_err = 317, .posix_err = 0, .msg = R"(The UPS service is about to perform final shut down.)"},
	{.status = 5170, .dos_err = 317, .posix_err = 0, .msg = R"(The Workstation must be started with the NET START command.)"},
	{.status = 5175, .dos_err = 317, .posix_err = 0, .msg = R"(Remote IPC)"},
	{.status = 5176, .dos_err = 317, .posix_err = 0, .msg = R"(Remote Admin)"},
	{.status = 5177, .dos_err = 317, .posix_err = 0, .msg = R"(Default share)"},
	{.status = 5178, .dos_err = 317, .posix_err = 0, .msg = R"(User Profiles)"},
	{.status = 5280, .dos_err = 317, .posix_err = 0, .msg = R"(The password entered is longer than 14 characters.  Computers

with Windows prior to Windows 2000 will not be able to use

this account. Do you want to continue this operation? %1:)"},
	{.status = 5281, .dos_err = 317, .posix_err = 0, .msg = R"(%1 has a remembered connection to %2. Do you

want to overwrite the remembered connection? %3:)"},
	{.status = 5282, .dos_err = 317, .posix_err = 0, .msg = R"(Do you want to resume loading the profile?  The command which

caused the error will be ignored. %1:)"},
	{.status = 5284, .dos_err = 317, .posix_err = 0, .msg = R"(Do you want to continue this operation? %1:)"},
	{.status = 5285, .dos_err = 317, .posix_err = 0, .msg = R"(Do you want to add this? %1:)"},
	{.status = 5286, .dos_err = 317, .posix_err = 0, .msg = R"(Do you want to continue this operation? %1:)"},
	{.status = 5287, .dos_err = 317, .posix_err = 0, .msg = R"(Is it OK to start it? %1:)"},
	{.status = 5288, .dos_err = 317, .posix_err = 0, .msg = R"(Do you want to start the Workstation service? %1:)"},
	{.status = 5289, .dos_err = 317, .posix_err = 0, .msg = R"(Is it OK to continue disconnecting and force them closed? %1:)"},
	{.status = 5290, .dos_err = 317, .posix_err = 0, .msg = R"(The printer does not exist.  Do you want to create it? %1:)"},
	{.status = 5291, .dos_err = 317, .posix_err = 0, .msg = R"(Never)"},
	{.status = 5292, .dos_err = 317, .posix_err = 0, .msg = R"(Never)"},
	{.status = 5293, .dos_err = 317, .posix_err = 0, .msg = R"(Never)"},
	{.status = 5295, .dos_err = 317, .posix_err = 0, .msg = R"(NET.HLP)"},
	{.status = 5296, .dos_err = 317, .posix_err = 0, .msg = R"(NET.HLP)"},
	{.status = 5297, .dos_err = 317, .posix_err = 0, .msg = R"(Deny)"},
	{.status = 5300, .dos_err = 317, .posix_err = 0, .msg = R"(The network control block (NCB) request completed successfully.

The NCB is the data.)"},
	{.status = 5301, .dos_err = 317, .posix_err = 0, .msg = R"(Illegal network control block (NCB) buffer length on SEND DATAGRAM,

SEND BROADCAST, ADAPTER STATUS, or SESSION STATUS.

The NCB is the data.)"},
	{.status = 5302, .dos_err = 317, .posix_err = 0, .msg = R"(The data descriptor array specified in the network control block (NCB) is

invalid.  The NCB is the data.)"},
	{.status = 5303, .dos_err = 317, .posix_err = 0, .msg = R"(The command specified in the network control block (NCB) is illegal.

The NCB is the data.)"},
	{.status = 5304, .dos_err = 317, .posix_err = 0, .msg = R"(The message correlator specified in the network control block (NCB) is

invalid.  The NCB is the data.)"},
	{.status = 5305, .dos_err = 317, .posix_err = 0, .msg = R"(A network control block (NCB) command timed-out.  The session may have

terminated abnormally.  The NCB is the data.)"},
	{.status = 5306, .dos_err = 317, .posix_err = 0, .msg = R"(An incomplete network control block (NCB) message was received.

The NCB is the data.)"},
	{.status = 5307, .dos_err = 317, .posix_err = 0, .msg = R"(The buffer address specified in the network control block (NCB) is illegal.

The NCB is the data.)"},
	{.status = 5308, .dos_err = 317, .posix_err = 0, .msg = R"(The session number specified in the network control block (NCB) is not active.

The NCB is the data.)"},
	{.status = 5309, .dos_err = 317, .posix_err = 0, .msg = R"(No resource was available in the network adapter.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5310, .dos_err = 317, .posix_err = 0, .msg = R"(The session specified in the network control block (NCB) was closed.

The NCB is the data.)"},
	{.status = 5311, .dos_err = 317, .posix_err = 0, .msg = R"(The network control block (NCB) command was canceled.

The NCB is the data.)"},
	{.status = 5312, .dos_err = 317, .posix_err = 0, .msg = R"(The message segment specified in the network control block (NCB) is

illogical.  The NCB is the data.)"},
	{.status = 5313, .dos_err = 317, .posix_err = 0, .msg = R"(The name already exists in the local adapter name table.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5314, .dos_err = 317, .posix_err = 0, .msg = R"(The network adapter name table is full.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5315, .dos_err = 317, .posix_err = 0, .msg = R"(The network name has active sessions and is now de-registered.

The network control block (NCB) command completed.  The NCB is the data.)"},
	{.status = 5316, .dos_err = 317, .posix_err = 0, .msg = R"(A previously issued Receive Lookahead command is active

for this session.  The network control block (NCB) command was rejected.

The NCB is the data.)"},
	{.status = 5317, .dos_err = 317, .posix_err = 0, .msg = R"(The local session table is full. The network control block (NCB) request was refused.

The NCB is the data.)"},
	{.status = 5318, .dos_err = 317, .posix_err = 0, .msg = R"(A network control block (NCB) session open was rejected.  No LISTEN is outstanding

on the remote computer.  The NCB is the data.)"},
	{.status = 5319, .dos_err = 317, .posix_err = 0, .msg = R"(The name number specified in the network control block (NCB) is illegal.

The NCB is the data.)"},
	{.status = 5320, .dos_err = 317, .posix_err = 0, .msg = R"(The call name specified in the network control block (NCB) cannot be found or

did not answer.  The NCB is the data.)"},
	{.status = 5321, .dos_err = 317, .posix_err = 0, .msg = R"(The name specified in the network control block (NCB) was not found.  Cannot put '*' or

00h in the NCB name.  The NCB is the data.)"},
	{.status = 5322, .dos_err = 317, .posix_err = 0, .msg = R"(The name specified in the network control block (NCB) is in use on a remote adapter.

The NCB is the data.)"},
	{.status = 5323, .dos_err = 317, .posix_err = 0, .msg = R"(The name specified in the network control block (NCB) has been deleted.

The NCB is the data.)"},
	{.status = 5324, .dos_err = 317, .posix_err = 0, .msg = R"(The session specified in the network control block (NCB) ended abnormally.

The NCB is the data.)"},
	{.status = 5325, .dos_err = 317, .posix_err = 0, .msg = R"(The network protocol has detected two or more identical

names on the network.	The network control block (NCB) is the data.)"},
	{.status = 5326, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected protocol packet was received.  There may be an

incompatible remote device.  The network control block (NCB) is the data.)"},
	{.status = 5333, .dos_err = 317, .posix_err = 0, .msg = R"(The NetBIOS interface is busy.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5334, .dos_err = 317, .posix_err = 0, .msg = R"(There are too many network control block (NCB) commands outstanding.

The NCB request was refused.  The NCB is the data.)"},
	{.status = 5335, .dos_err = 317, .posix_err = 0, .msg = R"(The adapter number specified in the network control block (NCB) is illegal.

The NCB is the data.)"},
	{.status = 5336, .dos_err = 317, .posix_err = 0, .msg = R"(The network control block (NCB) command completed while a cancel was occurring.

The NCB is the data.)"},
	{.status = 5337, .dos_err = 317, .posix_err = 0, .msg = R"(The name specified in the network control block (NCB) is reserved.

The NCB is the data.)"},
	{.status = 5338, .dos_err = 317, .posix_err = 0, .msg = R"(The network control block (NCB) command is not valid to cancel.

The NCB is the data.)"},
	{.status = 5351, .dos_err = 317, .posix_err = 0, .msg = R"(There are multiple network control block (NCB) requests for the same session.

The NCB request was refused.  The NCB is the data.)"},
	{.status = 5352, .dos_err = 317, .posix_err = 0, .msg = R"(There has been a network adapter error. The only NetBIOS

command that may be issued is an NCB RESET. The network control block (NCB) is

the data.)"},
	{.status = 5354, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of applications was exceeded.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5356, .dos_err = 317, .posix_err = 0, .msg = R"(The requested resources are not available.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5364, .dos_err = 317, .posix_err = 0, .msg = R"(A system error has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5365, .dos_err = 317, .posix_err = 0, .msg = R"(A ROM checksum failure has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5366, .dos_err = 317, .posix_err = 0, .msg = R"(A RAM test failure has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5367, .dos_err = 317, .posix_err = 0, .msg = R"(A digital loopback failure has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5368, .dos_err = 317, .posix_err = 0, .msg = R"(An analog loopback failure has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5369, .dos_err = 317, .posix_err = 0, .msg = R"(An interface failure has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5370, .dos_err = 317, .posix_err = 0, .msg = R"(An unrecognized network control block (NCB) return code was received.

The NCB is the data.)"},
	{.status = 5380, .dos_err = 317, .posix_err = 0, .msg = R"(A network adapter malfunction has occurred.

The network control block (NCB) request was refused.  The NCB is the data.)"},
	{.status = 5381, .dos_err = 317, .posix_err = 0, .msg = R"(The network control block (NCB) command is still pending.

The NCB is the data.)"},
	{.status = 5500, .dos_err = 317, .posix_err = 0, .msg = R"(The update log on %1 is over 80%% capacity. The primary

domain controller %2 is not retrieving the updates.)"},
	{.status = 5501, .dos_err = 317, .posix_err = 0, .msg = R"(The update log on %1 is full, and no further updates

can be added until the primary domain controller %2

retrieves the updates.)"},
	{.status = 5502, .dos_err = 317, .posix_err = 0, .msg = R"(The time difference with the primary domain controller %1

exceeds the maximum allowed skew of %2 seconds.)"},
	{.status = 5503, .dos_err = 317, .posix_err = 0, .msg = R"(The account of user %1 has been locked out on %2

due to %3 bad password attempts.)"},
	{.status = 5504, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 log file cannot be opened.)"},
	{.status = 5505, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 log file is corrupted and will be cleared.)"},
	{.status = 5506, .dos_err = 317, .posix_err = 0, .msg = R"(The Application log file could not be opened.  %1 will be used as the

default log file.)"},
	{.status = 5507, .dos_err = 317, .posix_err = 0, .msg = R"(The %1 Log is full.  If this is the first time you have seen this

message, take the following steps:



1. Click Start, click Run, type "eventvwr", and then click OK.



2. Click %1, click the Action menu, click Clear All Events, and then click No.





If this dialog reappears, contact your helpdesk or system administrator.)"},
	{.status = 5508, .dos_err = 317, .posix_err = 0, .msg = R"(The security database full synchronization has been initiated by the server %1.)"},
	{.status = 5509, .dos_err = 317, .posix_err = 0, .msg = R"(Windows could not be started as configured.

A previous working configuration was used instead.)"},
	{.status = 5510, .dos_err = 317, .posix_err = 0, .msg = R"(The exception 0x%1 occurred in the application %2 at location 0x%3.)"},
	{.status = 5511, .dos_err = 317, .posix_err = 0, .msg = R"(The servers %1 and  %3 both claim to be an NT Domain Controller for

the %2 domain. One of the servers should be removed from the

domain because the servers have different security identifiers

(SID).)"},
	{.status = 5512, .dos_err = 317, .posix_err = 0, .msg = R"(The server %1 and %2 both claim to be the primary domain

controller for the %3 domain. One of the servers should be

demoted or removed from the domain.)"},
	{.status = 5513, .dos_err = 317, .posix_err = 0, .msg = R"(The computer %1 tried to connect to the server %2 using

the trust relationship established by the %3 domain. However, the

computer lost the correct security identifier (SID)

when the domain was reconfigured. Reestablish the trust

relationship.)"},
	{.status = 5514, .dos_err = 317, .posix_err = 0, .msg = R"(The computer has rebooted from a bugcheck.  The bugcheck was:

%1.

%2

A full dump was not saved.)"},
	{.status = 5515, .dos_err = 317, .posix_err = 0, .msg = R"(The computer has rebooted from a bugcheck.  The bugcheck was:

%1.

%2

A dump was saved in: %3.)"},
	{.status = 5516, .dos_err = 317, .posix_err = 0, .msg = R"(The computer or domain %1 trusts domain %2.  (This may be an indirect

trust.)  However, %1 and %2 have the same machine security identifier

(SID).  NT should be re-installed on either %1 or %2.)"},
	{.status = 5517, .dos_err = 317, .posix_err = 0, .msg = R"(The computer or domain %1 trusts domain %2.  (This may be an indirect

trust.)  However, %2 is not a valid name for a trusted domain.

The name of the trusted domain should be changed to a valid name.)"},
	{.status = 5600, .dos_err = 317, .posix_err = 0, .msg = R"(Could not share the User or Script path.)"},
	{.status = 5601, .dos_err = 317, .posix_err = 0, .msg = R"(The password for this computer is not found in the local security

database.)"},
	{.status = 5602, .dos_err = 317, .posix_err = 0, .msg = R"(An internal error occurred while accessing the computer's

local or network security database.)"},
	{.status = 5700, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service could not initialize the replication data

structures successfully. The service was terminated.  The following

error occurred: 

%1)"},
	{.status = 5701, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service failed to update the domain trust list.  The

following error occurred: 

%1)"},
	{.status = 5702, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service could not add the RPC interface.  The

service was terminated. The following error occurred: 

%1)"},
	{.status = 5703, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service could not read a mailslot message from %1 due

to the following error: 

%2)"},
	{.status = 5704, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service failed to register the service with the

service controller. The service was terminated. The following

error occurred: 

%1)"},
	{.status = 5705, .dos_err = 317, .posix_err = 0, .msg = R"(The change log cache maintained by the Netlogon service for %1

database changes is inconsistent. The Netlogon service is resetting

the change log.)"},
	{.status = 5706, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service could not create server share %1.  The following

error occurred: 

%2)"},
	{.status = 5707, .dos_err = 317, .posix_err = 0, .msg = R"(The down-level logon request for the user %1 from %2 failed.)"},
	{.status = 5708, .dos_err = 317, .posix_err = 0, .msg = R"(The down-level logoff request for the user %1 from %2 failed.)"},
	{.status = 5709, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows NT or Windows 2000 %1 logon request for the user %2\%3 from %4 (via %5)

failed.)"},
	{.status = 5710, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows NT or Windows 2000 %1 logoff request for the user %2\%3 from %4

failed.)"},
	{.status = 5711, .dos_err = 317, .posix_err = 0, .msg = R"(The partial synchronization request from the server %1 completed

successfully. %2 changes(s) has(have) been returned to the

caller.)"},
	{.status = 5712, .dos_err = 317, .posix_err = 0, .msg = R"(The partial synchronization request from the server %1 failed with

the following error: 

%2)"},
	{.status = 5713, .dos_err = 317, .posix_err = 0, .msg = R"(The full synchronization request from the server %1 completed

successfully. %2 object(s) has(have) been returned to

the caller.)"},
	{.status = 5714, .dos_err = 317, .posix_err = 0, .msg = R"(The full synchronization request from the server %1 failed with

the following error: 

%2)"},
	{.status = 5715, .dos_err = 317, .posix_err = 0, .msg = R"(The partial synchronization replication of the %1 database from the

primary domain controller %2 completed successfully. %3 change(s) is(are)

applied to the database.)"},
	{.status = 5716, .dos_err = 317, .posix_err = 0, .msg = R"(The partial synchronization replication of the %1 database from the

primary domain controller %2 failed with the following error: 

%3)"},
	{.status = 5717, .dos_err = 317, .posix_err = 0, .msg = R"(The full synchronization replication of the %1 database from the

primary domain controller %2 completed successfully.)"},
	{.status = 5718, .dos_err = 317, .posix_err = 0, .msg = R"(The full synchronization replication of the %1 database from the

primary domain controller %2 failed with the following error: 

%3)"},
	{.status = 5719, .dos_err = 317, .posix_err = 0, .msg = R"(This computer was not able to set up a secure session with a domain

controller in domain %1 due to the following: 

%2



This may lead to authentication problems. Make sure that this

computer is connected to the network. If the problem persists,

please contact your domain administrator.







ADDITIONAL INFO



If this computer is a domain controller for the specified domain, it

sets up the secure session to the primary domain controller emulator in the specified

domain. Otherwise, this computer sets up the secure session to any domain controller

in the specified domain.)"},
	{.status = 5720, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup to the Windows Domain Controller %1 for the domain %2

failed because the computer %3 does not have a local security database account.)"},
	{.status = 5721, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup to the Windows Domain Controller %1 for the domain %2

failed because the Domain Controller did not have an account %4

needed to set up the session by this computer %3.







ADDITIONAL DATA



If this computer is a member of or a Domain Controller in the specified domain, the

aforementioned account is a computer account for this computer in the specified domain.

Otherwise, the account is an interdomain trust account with the specified domain.)"},
	{.status = 5722, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup from the computer %1 failed to authenticate.

The name(s) of the account(s) referenced in the security database is

%2.  The following error occurred: 

%3)"},
	{.status = 5723, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup from computer '%1' failed because the security database

does not contain a trust account '%2' referenced by the specified computer.







USER ACTION





If this is the first occurrence of this event for the specified computer

and account, this may be a transient issue that doesn't require any action

at this time.



If this is a Read-Only Domain Controller and '%2' is a legitimate machine

account for the computer '%1' then '%1' should be marked cacheable for this

location if appropriate or otherwise ensure connectivity to a domain controller 

capable of servicing the request (for example a writable domain controller).



Otherwise, the following steps may be taken to resolve this problem:







If '%2' is a legitimate machine account for the computer '%1', then '%1'

should be rejoined to the domain.







If '%2' is a legitimate interdomain trust account, then the trust should

be recreated.







Otherwise, assuming that '%2' is not a legitimate account, the following

action should be taken on '%1':







If '%1' is a Domain Controller, then the trust associated with '%2' should be deleted.







If '%1' is not a Domain Controller, it should be disjoined from the domain.)"},
	{.status = 5724, .dos_err = 317, .posix_err = 0, .msg = R"(Could not register control handler with service controller %1.)"},
	{.status = 5725, .dos_err = 317, .posix_err = 0, .msg = R"(Could not set service status with service controller %1.)"},
	{.status = 5726, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find the computer name %1.)"},
	{.status = 5727, .dos_err = 317, .posix_err = 0, .msg = R"(Could not load %1 device driver.)"},
	{.status = 5728, .dos_err = 317, .posix_err = 0, .msg = R"(Could not load any transport.)"},
	{.status = 5729, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Domain Object "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5730, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Global Group "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5731, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Local Group "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5732, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 User "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5733, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Policy Object "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5734, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Trusted Domain Object "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5735, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Account Object "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5736, .dos_err = 317, .posix_err = 0, .msg = R"(Replication of the %1 Secret "%2" from primary domain controller

%3 failed with the following error: 

%4)"},
	{.status = 5737, .dos_err = 317, .posix_err = 0, .msg = R"(The system returned the following unexpected error code: 

%1)"},
	{.status = 5738, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon has detected two machine accounts for server "%1".

The server can be either a Windows 2000 Server that is a member of the

domain or the server can be a LAN Manager server with an account in the

SERVERS global group.  It cannot be both.)"},
	{.status = 5739, .dos_err = 317, .posix_err = 0, .msg = R"(This domain has more global groups than can be replicated to a LanMan

BDC.  Either delete some of your global groups or remove the LanMan

BDCs from the domain.)"},
	{.status = 5740, .dos_err = 317, .posix_err = 0, .msg = R"(The Browser driver returned the following error to Netlogon: 

%1)"},
	{.status = 5741, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon could not register the %1<1B> name for the following reason: 

%2)"},
	{.status = 5742, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to retrieve messages needed to boot remote boot clients.)"},
	{.status = 5743, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced a severe error and can no longer provide remote boot

for 3Com 3Start remote boot clients.)"},
	{.status = 5744, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced a severe system error and will shut itself down.)"},
	{.status = 5745, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 failed to acknowledge receipt of the

boot data.  Remote boot of this client was not completed.)"},
	{.status = 5746, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to an error in opening

file %2.)"},
	{.status = 5747, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to an error in reading

file %2.)"},
	{.status = 5748, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to insufficient memory

at the remote boot server.)"},
	{.status = 5749, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 will be booted without using checksums

because checksum for file %2 could not be calculated.)"},
	{.status = 5750, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to too many lines in

file %2.)"},
	{.status = 5751, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted because the boot block

configuration file %2 for this client does not contain boot block

line and/or loader line.)"},
	{.status = 5752, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to a bad size of

file %2.)"},
	{.status = 5753, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to remote boot

service internal error.)"},
	{.status = 5754, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted because file %2 has an

invalid boot header.)"},
	{.status = 5755, .dos_err = 317, .posix_err = 0, .msg = R"(Client with computer name %1 was not booted due to network error.)"},
	{.status = 5756, .dos_err = 317, .posix_err = 0, .msg = R"(Client with adapter id %1 was not booted due to lack of resources.)"},
	{.status = 5757, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error copying file or directory %1.)"},
	{.status = 5758, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error deleting file or directory %1.)"},
	{.status = 5759, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error setting permissions on file or directory %1.)"},
	{.status = 5760, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error evaluating RPL configurations.)"},
	{.status = 5761, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error creating RPL profiles for all configurations.)"},
	{.status = 5762, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error accessing registry.)"},
	{.status = 5763, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error replacing possibly outdated RPLDISK.SYS.)"},
	{.status = 5764, .dos_err = 317, .posix_err = 0, .msg = R"(Service experienced error adding security accounts or setting

file permissions.  These accounts are the RPLUSER local group

and the user accounts for the individual RPL workstations.)"},
	{.status = 5765, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to back up its database.)"},
	{.status = 5766, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to initialize from its database.  The database may be

missing or corrupted.  Service will attempt restoring the database

from the backup.)"},
	{.status = 5767, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to restore its database from the backup.  Service

will not start.)"},
	{.status = 5768, .dos_err = 317, .posix_err = 0, .msg = R"(Service successfully restored its database from the backup.)"},
	{.status = 5769, .dos_err = 317, .posix_err = 0, .msg = R"(Service failed to initialize from its restored database.  Service

will not start.)"},
	{.status = 5770, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup to the Windows Domain Controller %1 from computer

%2 using account %4 failed.  %2 is declared to be a BDC in domain %3.

However, %2 tried to connect as either a DC in a trusted domain,

a member workstation in domain %3, or as a server in domain %3.

Use the Active Directory Users and Computers tool or Server Manager to remove the BDC account for %2.)"},
	{.status = 5771, .dos_err = 317, .posix_err = 0, .msg = R"(The Remoteboot database was in NT 3.5 / NT 3.51 format and NT is

attempting to convert it to NT 4.0 format. The JETCONV converter

will write to the Application event log when it is finished.)"},
	{.status = 5772, .dos_err = 317, .posix_err = 0, .msg = R"(Global group SERVERS exists in domain %1 and has members.

This group defines Lan Manager BDCs in the domain.

Lan Manager BDCs are not permitted in NT domains.)"},
	{.status = 5773, .dos_err = 317, .posix_err = 0, .msg = R"(The following DNS server that is authoritative for the DNS domain controller

locator records of this domain controller does not support dynamic DNS updates:







DNS server IP address: %1



Returned Response Code (RCODE): %2



Returned Status Code: %3







USER ACTION





Configure the DNS server to allow dynamic DNS updates or manually add the DNS

records from the file '%SystemRoot%\System32\Config\Netlogon.dns' to the DNS database.)"},
	{.status = 5774, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic registration of the DNS record '%1' failed on the following DNS server:







DNS server IP address: %3



Returned Response Code (RCODE): %4



Returned Status Code: %5







For computers and users to locate this domain controller, this record must be

registered in DNS.







USER ACTION





Determine what might have caused this failure, resolve the problem, and initiate

registration of the DNS records by the domain controller. To determine what might

have caused this failure, run DCDiag.exe. To learn more about DCDiag.exe, see Help

and Support Center. To initiate registration of the DNS records by this domain 

controller, run 'nltest.exe /dsregdns' from the command prompt on the domain controller

or restart Net Logon service. 

  Or, you can manually add this record to DNS, but it

is not recommended.







ADDITIONAL DATA



Error Value: %2)"},
	{.status = 5775, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic deletion of the DNS record '%1' failed on the following DNS server:







DNS server IP address: %3



Returned Response Code (RCODE): %4



Returned Status Code: %5







USER ACTION





To prevent remote computers from connecting unnecessarily to the domain controller,

delete the record manually or troubleshoot the failure to dynamically delete the

record. To learn more about debugging DNS, see Help and Support Center.







ADDITIONAL DATA



Error Value: %2)"},
	{.status = 5776, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to create/open file %1 with the following error: 

%2)"},
	{.status = 5777, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon got the following error while trying to get the subnet to site

mapping information from the DS: 

%1)"},
	{.status = 5778, .dos_err = 317, .posix_err = 0, .msg = R"('%1' tried to determine its site by looking up its IP address ('%2')

in the Configuration\Sites\Subnets container in the DS.  No subnet matched

the IP address.  Consider adding a subnet object for this IP address.)"},
	{.status = 5779, .dos_err = 317, .posix_err = 0, .msg = R"(The site name for this computer is '%1'.  That site name is not a valid

site name.  A site name must be a valid DNS label.

Rename the site to be a valid name.)"},
	{.status = 5780, .dos_err = 317, .posix_err = 0, .msg = R"(The subnet object '%1' appears in the Configuration\Sites\Subnets

container in the DS.  The name is not syntactically valid.  The valid

syntax is xx.xx.xx.xx/yy where xx.xx.xx.xx is a valid IP subnet number

and yy is the number of bits in the subnet mask.



Correct the name of the subnet object.)"},
	{.status = 5781, .dos_err = 317, .posix_err = 0, .msg = R"(Dynamic registration or deletion of one or more DNS records associated with DNS

domain '%1' failed.  These records are used by other computers to locate this

server as a domain controller (if the specified domain is an Active Directory

domain) or as an LDAP server (if the specified domain is an application partition).







Possible causes of failure include:





- TCP/IP properties of the network connections of this computer contain wrong IP address(es) of the preferred and alternate DNS servers



- Specified preferred and alternate DNS servers are not running



- DNS server(s) primary for the records to be registered is not running



- Preferred or alternate DNS servers are configured with wrong root hints



- Parent DNS zone contains incorrect delegation to the child zone authoritative for the DNS records that failed registration







USER ACTION





Fix possible misconfiguration(s) specified above and initiate registration or deletion of

the DNS records by running 'nltest.exe /dsregdns' from the command prompt on the domain

controller or by restarting Net Logon service on the domain controller.)"},
	{.status = 5782, .dos_err = 317, .posix_err = 0, .msg = R"(Dynamic registration or deregistration of one or more DNS records failed with the following error: 

%1)"},
	{.status = 5783, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup to the Windows Domain Controller %1 for the domain %2

is not responsive.  The current RPC call from Netlogon on \\%3 to %1 has been cancelled.)"},
	{.status = 5784, .dos_err = 317, .posix_err = 0, .msg = R"(Site '%2' does not have any Domain Controllers for domain '%3'.

Domain Controllers in site '%1' have been automatically

selected to cover site '%2' for domain '%3' based on configured

Directory Server replication costs.)"},
	{.status = 5785, .dos_err = 317, .posix_err = 0, .msg = R"(This Domain Controller no longer automatically covers site '%1' for domain '%2'.)"},
	{.status = 5786, .dos_err = 317, .posix_err = 0, .msg = R"(Site '%2' does not have any Global Catalog servers for forest '%3'.

Global Catalog servers in site '%1' have been automatically

selected to cover site '%2' for forest '%3' based on configured

Directory Server replication costs.)"},
	{.status = 5787, .dos_err = 317, .posix_err = 0, .msg = R"(This Global Catalog server no longer automatically covers site '%1' for forest '%2'.)"},
	{.status = 5788, .dos_err = 317, .posix_err = 0, .msg = R"(Attempt to update HOST Service Principal Names (SPNs) of the computer

object in Active Directory failed. The updated values were '%1' and '%2'.

The following error occurred: 

%3)"},
	{.status = 5789, .dos_err = 317, .posix_err = 0, .msg = R"(Attempt to update DNS Host Name of the computer object

in Active Directory failed. The updated value was '%1'.

The following error occurred: 

%2)"},
	{.status = 5790, .dos_err = 317, .posix_err = 0, .msg = R"(No suitable Domain Controller is available for domain %1.

An NT4 or older domain controller is available but it cannot

be used for authentication purposes in the Windows 2000 or newer

domain that this computer is a member of.

The following error occurred:

%2)"},
	{.status = 5791, .dos_err = 317, .posix_err = 0, .msg = R"(The domain of this computer, %1 has been downgraded from Windows 2000

or newer to Windows NT4 or older. The computer cannot function properly

in this case for authentication purposes. This computer needs to rejoin

the domain.

The following error occurred:

%2)"},
	{.status = 5792, .dos_err = 317, .posix_err = 0, .msg = R"(Site '%2' does not have any LDAP servers for non-domain NC '%3'.

LDAP servers in site '%1' have been automatically selected to

cover site '%2' for non-domain NC '%3' based on configured

Directory Server replication costs.)"},
	{.status = 5793, .dos_err = 317, .posix_err = 0, .msg = R"(This LDAP server no longer automatically covers site '%1' for non-domain NC '%2'.)"},
	{.status = 5794, .dos_err = 317, .posix_err = 0, .msg = R"(Site '%2' is no longer manually configured in the registry as

covered by this Domain Controller for domain '%3'. As a result,

site '%2' does not have any Domain Controllers for domain '%3'.

Domain Controllers in site '%1' have been automatically

selected to cover site '%2' for domain '%3' based on configured

Directory Server replication costs.)"},
	{.status = 5795, .dos_err = 317, .posix_err = 0, .msg = R"(This Domain Controller no longer automatically covers site '%1' for domain '%2'.

However, site '%1' is still (manually) covered by this Domain Controller for

domain '%2' since this site has been manually configured in the registry.)"},
	{.status = 5796, .dos_err = 317, .posix_err = 0, .msg = R"(Site '%2' is no longer manually configured in the registry as

covered by this Global Catalog server for forest '%3'. As a result,

site '%2' does not have any Global Catalog servers for forest '%3'.

Global Catalog servers in site '%1' have been automatically

selected to cover site '%2' for forest '%3' based on configured

Directory Server replication costs.)"},
	{.status = 5797, .dos_err = 317, .posix_err = 0, .msg = R"(This Global Catalog server no longer automatically covers site '%1' for forest '%2'.

However, site '%1' is still (manually) covered by this Global catalog for

forest '%2' since this site has been manually configured in the registry.)"},
	{.status = 5798, .dos_err = 317, .posix_err = 0, .msg = R"(Site '%2' is no longer manually configured in the registry as

covered by this LDAP server for non-domain NC '%3'. As a result,

site '%2' does not have any LDAP servers for non-domain NC '%3'.

LDAP servers in site '%1' have been automatically

selected to cover site '%2' for non-domain NC '%3' based on

configured Directory Server replication costs.)"},
	{.status = 5799, .dos_err = 317, .posix_err = 0, .msg = R"(This LDAP server no longer automatically covers site '%1' for non-domain NC '%2'.

However, site '%1' is still (manually) covered by this LDAP server for

non-domain NC '%2' since this site has been manually configured in the registry.)"},
	{.status = 5800, .dos_err = 317, .posix_err = 0, .msg = R"(Attempt to update DnsHostName and HOST Service Principal Name (SPN) attributes

of the computer object in Active Directory failed because the Domain Controller

'%1' had more than one account with the name '%2' corresponding to this computer.

Not having SPNs registered may result in authentication failures for this computer.

Contact your domain administrator who may need to manually resolve the account name

collision.)"},
	{.status = 5801, .dos_err = 317, .posix_err = 0, .msg = R"(Attempt to update DnsHostName and HOST Service Principal Name (SPN) attributes

of the computer object in Active Directory failed because this computer account

name, '%2' could not be mapped to the computer object on Domain Controller '%1'.

Not having SPNs registered may result in authentication failures for this computer.

Contact your domain administrator. The following technical information may be

useful for the resolution of this failure:



DsCrackNames status = 0x%3, crack error = 0x%4.)"},
	{.status = 5802, .dos_err = 317, .posix_err = 0, .msg = R"(None of the IP addresses (%2) of this Domain Controller map to the configured site '%1'.

While this may be a temporary situation due to IP address changes, it is generally

recommended that the IP address of the Domain Controller (accessible to machines in

its domain) maps to the Site which it services. If the above list of IP addresses is

stable, consider moving this server to a site (or create one if it does not already

exist) such that the above IP address maps to the selected site. This may require the

creation of a new subnet object (whose range includes the above IP address) which maps

to the selected site object.)"},
	{.status = 5803, .dos_err = 317, .posix_err = 0, .msg = R"(The following error occurred while reading a parameter '%2' in the

Netlogon %1 registry section:

%3)"},
	{.status = 5804, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon %1 registry key contains an invalid value 0x%2 for parameter '%3'.

The minimum and maximum values allowed for this parameter are 0x%4 and 0x%5, respectively.

The value of 0x%6 has been assigned to this parameter.)"},
	{.status = 5805, .dos_err = 317, .posix_err = 0, .msg = R"(The session setup from the computer %1 failed to authenticate.

The following error occurred: 

%2)"},
	{.status = 5806, .dos_err = 317, .posix_err = 0, .msg = R"(Dynamic DNS updates have been manually disabled on this domain controller.







USER ACTION





Reconfigure this domain controller to use dynamic DNS updates or manually add the DNS

records from the file '%SystemRoot%\System32\Config\Netlogon.dns' to the DNS database.)"},
	{.status = 5807, .dos_err = 317, .posix_err = 0, .msg = R"(During the past %1 hours there have been %2 connections to this Domain

Controller from client machines whose IP addresses don't map to any of

the existing sites in the enterprise. Those clients, therefore, have

undefined sites and may connect to any Domain Controller including

those that are in far distant locations from the clients. A client's site

is determined by the mapping of its subnet to one of the existing sites.

To move the above clients to one of the sites, please consider creating

subnet object(s) covering the above IP addresses with mapping to one of the

existing sites.  The names and IP addresses of the clients in question have

been logged on this computer in the following log file

'%SystemRoot%\debug\netlogon.log' and, potentially, in the log file

'%SystemRoot%\debug\netlogon.bak' created if the former log becomes full.

The log(s) may contain additional unrelated debugging information. To filter

out the needed information, please search for lines which contain text

'NO_CLIENT_SITE:'. The first word after this string is the client name and

the second word is the client IP address. The maximum size of the log(s) is

controlled by the following registry DWORD value

'HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Netlogon\Parameters\LogFileMaxSize';

the default is %3 bytes.  The current maximum size is %4 bytes.  To set a

different maximum size, create the above registry value and set the desired

maximum size in bytes.)"},
	{.status = 5808, .dos_err = 317, .posix_err = 0, .msg = R"(The deregistration of some DNS domain controller locator records was aborted

at the time of this domain controller demotion because the DNS deregistrations

took too long.







USER ACTION





Manually delete the DNS records listed in the file

'%SystemRoot%\System32\Config\Netlogon.dns' from the DNS database.)"},
	{.status = 5809, .dos_err = 317, .posix_err = 0, .msg = R"(The NetLogon service on this domain controller has been configured to use port %1

for incoming RPC connections over TCP/IP from remote machines. However, the

following error occurred when Netlogon attempted to register this port with the RPC

endpoint mapper service: 

%2 

This will prevent the NetLogon service on remote

machines from connecting to this domain controller over TCP/IP that may result in

authentication problems.







USER ACTION





The specified port is configured via the Group Policy or via a registry value 'DcTcpipPort'

under the 'HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Netlogon\Parameters'

registry key; the value configured through the Group Policy takes precedence. If the

port specified is in error, reset it to a correct value. You can also remove this

configuration for the port in which case the port will be assigned dynamically by

the endpoint mapper at the time the NetLogon service on remote machines makes RPC connections

to this domain controller. After the misconfiguration is corrected, restart the NetLogon

service on this machine and verify that this event log no longer appears.)"},
	{.status = 5810, .dos_err = 317, .posix_err = 0, .msg = R"(During the past %1 hours, this domain controller has received %2 connections

from dual-stack IPv4/IPv6 clients with partial subnet-site mappings. A client

has a partial subnet-site mapping if its IPv4 address is mapped to a site but

its global IPv6 address is not mapped to a site, or vice versa. To ensure correct

behavior for applications running on member computers and servers that rely on

subnet-site mappings, dual-stack IPv4/IPv6 clients must have both IPv4 and global

IPv6 addresses mapped to the same site. If a partially mapped client attempts

to connect to this domain controller using its unmapped IP address, its mapped

address is used for the client's site mapping.







The log files %SystemRoot%\debug\netlogon.log or %SystemRoot%\debug\netlogon.bak

contain the name, unmapped IP address and mapped IP address for each partially

mapped client. The log files may also contain unrelated debugging information.

To locate the information pertaining to partial-subnet mappings, search for

lines that contain the text 'PARTIAL_CLIENT_SITE_MAPPING:'. The first word after

this text is the client name. Following the client name is the client's unmapped

IP address (the IP address that does not have a subnet-site mapping) and the

client's mapped IP address, which was used to return site information.







USER ACTION





Use the Active Directory Sites and Services management console (MMC) snap-in

to add the subnet mapping for the unmapped IP addresses to the same site being

used by the mapped IP addresses. When adding site mappings for IPv6 addresses,

you should use global IPv6 addresses and not for instance temporary, link-local

or site-local IPv6 addresses.







The default maximum size of the log files is %3 bytes. The current maximum

size is %4 bytes. To set a different maximum size, create the following registry

DWORD value to specify the maximum size in bytes:





HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Netlogon\Parameters\LogFileMaxSize)"},
	{.status = 5811, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic registration of the DNS record '%1' for the remote domain controller '%3'

failed on the following DNS server:







DNS server IP address: %4



Returned Response Code (RCODE): %5



Returned Status Code: %6







For computers and users to locate the domain controller '%3', this record must be

registered in DNS.







USER ACTION





Determine what might have caused this failure, resolve the problem, and initiate

registration of the DNS records by the domain controller '%3'. For help with determining

and resolving the problem, see Help and Support for information about troubleshooting

DNS. To initiate registration of the DNS records by the domain controller '%3', run

'nltest.exe /dsregdns' from the command prompt on the domain controller '%3' or restart

the Net Logon service on the domain controller '%3'. Nltest.exe is a command line tool

that is built into Windows Server.



 As a workaround, you can manually add this record to DNS, but it is not recommended

because you then must manually update any changes it requires hereafter.







ADDITIONAL DATA



Error Value: %2)"},
	{.status = 5812, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic deregistration of the DNS record '%1' for the remote domain controller

'%3' failed on the following DNS server:







DNS server IP address: %4



Returned Response Code (RCODE): %5



Returned Status Code: %6







USER ACTION





To prevent remote computers from attempting to connect to the domain controller '%3'

using an invalid record, delete the record '%1' manually or troubleshoot the root cause

behind the dynamic deregistration failure. To learn more about troubleshooting DNS, see

Help and Support.







ADDITIONAL DATA



Error Value: %2)"},
	{.status = 5813, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic registration request for the DNS record '%1' has been rejected by the

remote domain controller '%2'. Error: '%3'







For computers and users to locate this domain controller, this record must be

registered in DNS. If the problem persists, please contact your domain administrator.)"},
	{.status = 5814, .dos_err = 317, .posix_err = 0, .msg = R"(The dynamic deregistration request of the DNS record '%1' has been rejected by the

remote domain controller '%2'. Error: '%3'







To prevent remote computers from connecting unnecessarily to this domain controller,

an administrator with sufficient privileges must manually delete the record on the DNS

server that hosts it.)"},
	{.status = 5815, .dos_err = 317, .posix_err = 0, .msg = R"(The remoting of the dynamic update request for the local domain controller's DNS records

through a secure session has failed with error '%1'.







For other computers and member servers to locate this domain controller, the appropriate

records must be registered in DNS. On this domain controller, look for events related to

failure to set up a secure session to determine why the request is failing. If the problem

persists, please contact your domain administrator.)"},
	{.status = 5816, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon has failed an authentication request of account %1 in domain %2. The request timed out before it 

could be sent to domain controller %3 in domain %4. This is the first failure. If the problem continues, 

consolidated events will be logged about every %5 minutes.

Please see http://support.microsoft.com/kb/2654097 for more information.)"},
	{.status = 5817, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon has failed an additional %1 authentication requests in the last %2 minutes.

The requests timed out before they could be sent to domain controller %3 in domain %4.

Please see http://support.microsoft.com/kb/2654097 for more information.)"},
	{.status = 5818, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon took more than %1 seconds for an authentication request of account %2 in domain %3, through 

domain controller %4 in domain %5. This is the first warning. If the problem persists, a recurring event will be logged 

every %6 minutes.

Please see http://support.microsoft.com/kb/2654097 for more information on this error.)"},
	{.status = 5819, .dos_err = 317, .posix_err = 0, .msg = R"(Netlogon took more than %1 seconds for %2 authentication requests through domain controller %3 in domain %4 in the last %5 minutes.

Please see http://support.microsoft.com/kb/2654097 for more information.)"},
	{.status = 5820, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service could not add the AuthZ RPC interface.  The

service was terminated. The following error occurred: '%1')"},
	{.status = 5821, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service failed to initialize the AuthZ resource manager.

The service was terminated. The following error occurred: '%1'.)"},
	{.status = 5822, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service failed to initialize the security descriptor 

for the Netlogon RPC interface.   The service was terminated. The 

following error occurred: '%1'.)"},
	{.status = 5823, .dos_err = 317, .posix_err = 0, .msg = R"(

The system successfully changed its password on the domain controller %1.



This event is logged when the password for the computer account is

changed by the system. It is logged on the computer that changed the

password.)"},
	{.status = 5824, .dos_err = 317, .posix_err = 0, .msg = R"(

The system successfully changed the password for managed service account %1  

on the domain controller %2.



This event is logged when the password for a standalone managed service

account is changed by the system. It is logged on the computer that

changed the password.)"},
	{.status = 5826, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service detected a non-windows account using secure RPC.







 Machine SamAccountName: %1



 Domain: %2



 Account Type: %3



 Machine Os: %4



 Machine Os Build Version: %5



 Machine Os Service Pack: %6)"},
	{.status = 5827, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service denied a vulnerable Netlogon secure channel connection from a machine account.







 Machine SamAccountName: %1



 Domain: %2



 Account Type: %3



 Machine Operating System: %4



 Machine Operating System Build: %5



 Machine Operating System Service Pack: %6







For more information about why this was denied, please visit  https://go.microsoft.com/fwlink/?linkid=2133485.)"},
	{.status = 5828, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service denied a vulnerable Netlogon secure channel connection using a trust account.







 Account Type: %1



 Trust Name: %2



 Trust Target: %3



 Client IP Address: %4







For more information about why this was denied, please visit  https://go.microsoft.com/fwlink/?linkid=2133485.)"},
	{.status = 5829, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service allowed a vulnerable Netlogon secure channel connection.







Warning: This connection will be denied once the enforcement phase is released. To better understand the enforcement phase, 

please visit  https://go.microsoft.com/fwlink/?linkid=2133485.







 Machine SamAccountName: %1



 Domain: %2



 Account Type: %3



 Machine Operating System: %4



 Machine Operating System Build: %5



 Machine Operating System Service Pack: %6)"},
	{.status = 5830, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service allowed a vulnerable Netlogon secure channel connection because the machine account is allowed in the 

"Domain controller: Allow vulnerable Netlogon secure channel connections" group policy.







Warning: Using vulnerable Netlogon secure channels will expose the domain-joined devices to attack. To protect your device from attack, 

remove a machine account from "Domain controller: Allow vulnerable Netlogon secure channel connections" group policy after the third-party 

Netlogon client has been updated. To better understand the risk of configuring machine accounts to be allowed to use vulnerable Netlogon 

secure channel connections, please visit  https://go.microsoft.com/fwlink/?linkid=2133485.







 Machine SamAccountName: %1



 Domain: %2



 Account Type: %3



 Machine Os: %4



 Machine Os Build Version: %5



 Machine Os Service Pack: %6)"},
	{.status = 5831, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service allowed a vulnerable Netlogon secure channel connection because the trust account is allowed in the 

"Domain controller: Allow vulnerable Netlogon secure channel connections" group policy.







Warning: Using vulnerable Netlogon secure channels will expose Active Directory forests to attack. To protect your 

Active Directory forests from attack, all trusts must use secure RPC with Netlogon secure channel. Remove a trust account from 

"Domain controller: Allow vulnerable Netlogon secure channel connections" group policy after the third-party Netlogon client on the domain controllers 

have been updated. To better understand the risk of configuring trust accounts to be allowed to use vulnerable Netlogon secure channel connections, 

please visit  https://go.microsoft.com/fwlink/?linkid=2133485.







 Account Type: %1



 Trust Name: %2



 Trust Target: %3



 Client IP Address: %4)"},
	{.status = 5832, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service allowed one or more unsecure pass-through NTLM authentication requests from trusted domains and/or forests

during the most recent event throttling window. These unsecure requests would normally be blocked but were allowed to proceed

due to the current trust configuration.











Warning: Allowing unsecure pass-through authentication requests will expose your Active Directory forest to attack.

For more information about this issue please visit https://go.microsoft.com/fwlink/?linkid=276811&.











Count of unsecure requests allowed due to administrative override: %1)"},
	{.status = 5833, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service blocked one or more unsecure pass-through NTLM authentication requests from trusted clients, domains,

and/or forests during the most recent event throttling window. For more information about this issue, including how to enable

more verbose logging, please visit https://go.microsoft.com/fwlink/?linkid=276811&.











Count of unsecure requests blocked: %1)"},
	{.status = 5834, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service allowed an unsecure pass-through NTLM authentication request from a trusted client, domain,

or forest. This unsecure request would normally be blocked but was allowed to proceed due to the current trust

configuration.











Warning: Allowing unsecure pass-through authentication requests will expose your Active Directory forest to attack.

For more information about this issue please visit https://go.microsoft.com/fwlink/?linkid=276811&.











Account name: %1



Trust name: %2



Trust type: %3



Client IP Address: %4



Block reason: %5



Resource server Netbios name: %6



Resource server DNS name: %7



Resource domain Netbios name: %8



Resource domain DNS name: %9)"},
	{.status = 5835, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service blocked an unsecure pass-through NTLM authentication requests from a trusted client, domain,

or forest. For more information, please visit https://go.microsoft.com/fwlink/?linkid=276811&.











Account name: %1



Trust name: %2



Trust type: %3



Client IP Address: %4



Block reason: %5



Resource server Netbios name: %6



Resource server DNS name: %7



Resource domain Netbios name: %8



Resource domain DNS name: %9)"},
	{.status = 5836, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service was able to bind to a TCP/IP port with the configured backlog size of %1.)"},
	{.status = 5837, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service tried to bind to a TCP/IP port with the configured backlog size of %1 but failed. 







More information can be found in the following log file '%SystemRoot%\debug\netlogon.log' and, potentially, in the log file

'%SystemRoot%\debug\netlogon.bak' created if the former log becomes full. For steps in enabling the log, please visit

https://go.microsoft.com/fwlink/?linkid=2163327)"},
	{.status = 5838, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service encountered a client using RPC signing instead of RPC sealing.







 Machine SamAccountName: %1



 Domain: %2



 Account Type: %3



 Machine Operating System: %4



 Machine Operating System Build: %5



 Machine Operating System Service Pack: %6



 Client IP Address: %7









For more information about the impact of this, please visit  https://go.microsoft.com/fwlink/?linkid=2209514.)"},
	{.status = 5839, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service encountered a trust using RPC signing instead of RPC sealing.







 Account Type: %1



 Trust Name: %2



 Trust Target: %3



 Client IP Address: %4







For more information about the impact of this, please visit  https://go.microsoft.com/fwlink/?linkid=2209514.)"},
	{.status = 5840, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service created a secure channel with a client with RC4.







 Account Name: %1



 Domain: %2



 Account Type: %3



 Client IP Address: %4



 Negotiated Flags: %5







For more information about why this was logged, please visit  https://go.microsoft.com/fwlink/?linkid=2209514.)"},
	{.status = 5841, .dos_err = 317, .posix_err = 0, .msg = R"(The Netlogon service denied a client using RC4 due to the 'RejectMd5Clients' setting.







 Account Name: %1



 Domain: %2



 Account Type: %3



 Client IP Address: %4



 Negotiated Flags: %5







For more information about why this was denied, please visit  https://go.microsoft.com/fwlink/?linkid=2209514.)"},
	{.status = 5890, .dos_err = 317, .posix_err = 0, .msg = R"(An operation was attempted that is incompatible with the current membership state of the node.)"},
	{.status = 5891, .dos_err = 317, .posix_err = 0, .msg = R"(The quorum resource does not contain the quorum log.)"},
	{.status = 5892, .dos_err = 317, .posix_err = 0, .msg = R"(The membership engine requested shutdown of the cluster service on this node.)"},
	{.status = 5893, .dos_err = 317, .posix_err = 0, .msg = R"(The join operation failed because the cluster instance ID of the joining node does not match the cluster instance ID of the sponsor node.)"},
	{.status = 5894, .dos_err = 317, .posix_err = 0, .msg = R"(A matching cluster network for the specified IP address could not be found.)"},
	{.status = 5895, .dos_err = 317, .posix_err = 0, .msg = R"(The actual data type of the property did not match the expected data type of the property.)"},
	{.status = 5896, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node was evicted from the cluster successfully, but the node was not cleaned up. To determine what cleanup steps failed and how to recover, see the Failover Clustering application event log using Event Viewer.)"},
	{.status = 5897, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more parameter values specified for a resource's properties are in conflict.)"},
	{.status = 5898, .dos_err = 317, .posix_err = 0, .msg = R"(This computer cannot be made a member of a cluster.)"},
	{.status = 5899, .dos_err = 317, .posix_err = 0, .msg = R"(This computer cannot be made a member of a cluster because it does not have the correct version of Windows installed.)"},
	{.status = 5900, .dos_err = 317, .posix_err = 0, .msg = R"(A cluster cannot be created with the specified cluster name because that cluster name is already in use. Specify a different name for the cluster.)"},
	{.status = 5901, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster configuration action has already been committed.)"},
	{.status = 5902, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster configuration action could not be rolled back.)"},
	{.status = 5903, .dos_err = 317, .posix_err = 0, .msg = R"(The drive letter assigned to a system disk on one node conflicted with the drive letter assigned to a disk on another node.)"},
	{.status = 5904, .dos_err = 317, .posix_err = 0, .msg = R"(One or more nodes in the cluster are running a version of Windows that does not support this operation.)"},
	{.status = 5905, .dos_err = 317, .posix_err = 0, .msg = R"(The name of the corresponding computer account doesn't match the Network Name for this resource.)"},
	{.status = 5906, .dos_err = 317, .posix_err = 0, .msg = R"(No network adapters are available.)"},
	{.status = 5907, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster node has been poisoned.)"},
	{.status = 5908, .dos_err = 317, .posix_err = 0, .msg = R"(The group is unable to accept the request since it is moving to another node.)"},
	{.status = 5909, .dos_err = 317, .posix_err = 0, .msg = R"(The resource type cannot accept the request since is too busy performing another operation.)"},
	{.status = 5910, .dos_err = 317, .posix_err = 0, .msg = R"(The call to the cluster resource DLL timed out.)"},
	{.status = 5911, .dos_err = 317, .posix_err = 0, .msg = R"(The address is not valid for an IPv6 Address resource. A global IPv6 address is required, and it must match a cluster network. Compatibility addresses are not permitted.)"},
	{.status = 5912, .dos_err = 317, .posix_err = 0, .msg = R"(An internal cluster error occurred. A call to an invalid function was attempted.)"},
	{.status = 5913, .dos_err = 317, .posix_err = 0, .msg = R"(A parameter value is out of acceptable range.)"},
	{.status = 5914, .dos_err = 317, .posix_err = 0, .msg = R"(A network error occurred while sending data to another node in the cluster. The number of bytes transmitted was less than required.)"},
	{.status = 5915, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid cluster registry operation was attempted.)"},
	{.status = 5916, .dos_err = 317, .posix_err = 0, .msg = R"(An input string of characters is not properly terminated.)"},
	{.status = 5917, .dos_err = 317, .posix_err = 0, .msg = R"(An input string of characters is not in a valid format for the data it represents.)"},
	{.status = 5918, .dos_err = 317, .posix_err = 0, .msg = R"(An internal cluster error occurred. A cluster database transaction was attempted while a transaction was already in progress.)"},
	{.status = 5919, .dos_err = 317, .posix_err = 0, .msg = R"(An internal cluster error occurred. There was an attempt to commit a cluster database transaction while no transaction was in progress.)"},
	{.status = 5920, .dos_err = 317, .posix_err = 0, .msg = R"(An internal cluster error occurred. Data was not properly initialized.)"},
	{.status = 5921, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while reading from a stream of data. An unexpected number of bytes was returned.)"},
	{.status = 5922, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while writing to a stream of data. The required number of bytes could not be written.)"},
	{.status = 5923, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while deserializing a stream of cluster data.)"},
	{.status = 5924, .dos_err = 317, .posix_err = 0, .msg = R"(One or more property values for this resource are in conflict with one or more property values associated with its dependent resource(s).)"},
	{.status = 5925, .dos_err = 317, .posix_err = 0, .msg = R"(A quorum of cluster nodes was not present to form a cluster.)"},
	{.status = 5926, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is not valid for an IPv6 Address resource, or it does not match the configured address.)"},
	{.status = 5927, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster network is not valid for an IPv6 Tunnel resource. Check the configuration of the IP Address resource on which the IPv6 Tunnel resource depends.)"},
	{.status = 5928, .dos_err = 317, .posix_err = 0, .msg = R"(Quorum resource cannot reside in the Available Storage group.)"},
	{.status = 5929, .dos_err = 317, .posix_err = 0, .msg = R"(The dependencies for this resource are nested too deeply.)"},
	{.status = 5930, .dos_err = 317, .posix_err = 0, .msg = R"(The call into the resource DLL raised an unhandled exception.)"},
	{.status = 5931, .dos_err = 317, .posix_err = 0, .msg = R"(The RHS process failed to initialize.)"},
	{.status = 5932, .dos_err = 317, .posix_err = 0, .msg = R"(The Failover Clustering feature is not installed on this node.)"},
	{.status = 5933, .dos_err = 317, .posix_err = 0, .msg = R"(The resources must be online on the same node for this operation)"},
	{.status = 5934, .dos_err = 317, .posix_err = 0, .msg = R"(A new node can not be added since this cluster is already at its maximum number of nodes.)"},
	{.status = 5935, .dos_err = 317, .posix_err = 0, .msg = R"(This cluster can not be created since the specified number of nodes exceeds the maximum allowed limit.)"},
	{.status = 5936, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to use the specified cluster name failed because an enabled computer object with the given name already exists in the domain.)"},
	{.status = 5937, .dos_err = 317, .posix_err = 0, .msg = R"(This cluster cannot be destroyed. It has non-core application groups which must be deleted before the cluster can be destroyed.)"},
	{.status = 5938, .dos_err = 317, .posix_err = 0, .msg = R"(File share associated with file share witness resource cannot be hosted by this cluster or any of its nodes.)"},
	{.status = 5939, .dos_err = 317, .posix_err = 0, .msg = R"(Eviction of this node is invalid at this time. Due to quorum requirements node eviction will result in cluster shutdown.

If it is the last node in the cluster, destroy cluster command should be used.)"},
	{.status = 5940, .dos_err = 317, .posix_err = 0, .msg = R"(Only one instance of this resource type is allowed in the cluster.)"},
	{.status = 5941, .dos_err = 317, .posix_err = 0, .msg = R"(Only one instance of this resource type is allowed per resource group.)"},
	{.status = 5942, .dos_err = 317, .posix_err = 0, .msg = R"(The resource failed to come online due to the failure of one or more provider resources.)"},
	{.status = 5943, .dos_err = 317, .posix_err = 0, .msg = R"(The resource has indicated that it cannot come online on any node.)"},
	{.status = 5944, .dos_err = 317, .posix_err = 0, .msg = R"(The current operation cannot be performed on this group at this time.)"},
	{.status = 5945, .dos_err = 317, .posix_err = 0, .msg = R"(The directory or file is not located on a cluster shared volume.)"},
	{.status = 5946, .dos_err = 317, .posix_err = 0, .msg = R"(The Security Descriptor does not meet the requirements for a cluster.)"},
	{.status = 5947, .dos_err = 317, .posix_err = 0, .msg = R"(There is one or more shared volumes resources configured in the cluster.

Those resources must be moved to available storage in order for operation to succeed.)"},
	{.status = 5948, .dos_err = 317, .posix_err = 0, .msg = R"(This group or resource cannot be directly manipulated.

Use shared volume APIs to perform desired operation.)"},
	{.status = 5949, .dos_err = 317, .posix_err = 0, .msg = R"(Back up is in progress. Please wait for backup completion before trying this operation again.)"},
	{.status = 5950, .dos_err = 317, .posix_err = 0, .msg = R"(The path does not belong to a cluster shared volume.)"},
	{.status = 5951, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster shared volume is not locally mounted on this node.)"},
	{.status = 5952, .dos_err = 317, .posix_err = 0, .msg = R"(The cluster watchdog is terminating.)"},
	{.status = 5953, .dos_err = 317, .posix_err = 0, .msg = R"(A resource vetoed a move between two nodes because they are incompatible.)"},
	{.status = 5954, .dos_err = 317, .posix_err = 0, .msg = R"(The request is invalid either because node weight cannot be changed while the cluster is in disk-only quorum mode, or because changing the node weight would violate the minimum cluster quorum requirements.)"},
	{.status = 5955, .dos_err = 317, .posix_err = 0, .msg = R"(The resource vetoed the call.)"},
	{.status = 5956, .dos_err = 317, .posix_err = 0, .msg = R"(Resource could not start or run because it could not reserve sufficient system resources.)"},
	{.status = 5957, .dos_err = 317, .posix_err = 0, .msg = R"(A resource vetoed a move between two nodes because the destination currently does not have enough resources to complete the operation.)"},
	{.status = 5958, .dos_err = 317, .posix_err = 0, .msg = R"(

A resource vetoed a move between two nodes because the source currently does not have enough resources to complete the operation.)"},
	{.status = 5959, .dos_err = 317, .posix_err = 0, .msg = R"(

The requested operation can not be completed because the group is queued for an operation.)"},
	{.status = 5960, .dos_err = 317, .posix_err = 0, .msg = R"(

The requested operation can not be completed because a resource has locked status.)"},
	{.status = 5961, .dos_err = 317, .posix_err = 0, .msg = R"(

The resource cannot move to another node because a cluster shared volume vetoed the operation.)"},
	{.status = 5962, .dos_err = 317, .posix_err = 0, .msg = R"(

A node drain is already in progress.)"},
	{.status = 5963, .dos_err = 317, .posix_err = 0, .msg = R"(

Clustered storage is not connected to the node.)"},
	{.status = 5964, .dos_err = 317, .posix_err = 0, .msg = R"(

The disk is not configured in a way to be used with CSV. CSV disks must have at least one partition that is formatted with NTFS or REFS.)"},
	{.status = 5965, .dos_err = 317, .posix_err = 0, .msg = R"(

The resource must be part of the Available Storage group to complete this action.)"},
	{.status = 5966, .dos_err = 317, .posix_err = 0, .msg = R"(

CSVFS failed operation as volume is in redirected mode.)"},
	{.status = 5967, .dos_err = 317, .posix_err = 0, .msg = R"(

CSVFS failed operation as volume is not in redirected mode.)"},
	{.status = 5968, .dos_err = 317, .posix_err = 0, .msg = R"(

Cluster properties cannot be returned at this time.)"},
	{.status = 5969, .dos_err = 317, .posix_err = 0, .msg = R"(

The clustered disk resource contains software snapshot diff area that are not supported for Cluster Shared Volumes.)"},
	{.status = 5970, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation cannot be completed because the resource is in maintenance mode.)"},
	{.status = 5971, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation cannot be completed because of cluster affinity conflicts)"},
	{.status = 5972, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation cannot be completed because the resource is a replica virtual machine.)"},
	{.status = 5973, .dos_err = 317, .posix_err = 0, .msg = R"(

The Cluster Functional Level could not be increased because not all nodes in the cluster support the updated version.)"},
	{.status = 5974, .dos_err = 317, .posix_err = 0, .msg = R"(

Updating the cluster functional level failed because the cluster is running in fix quorum mode.

Start additional nodes which are members of the cluster until the cluster reaches quorum and the cluster will automatically

switch out of fix quorum mode, or stop and restart the cluster without the FixQuorum switch. Once the cluster is out

of fix quorum mode retry the Update-ClusterFunctionalLevel PowerShell cmdlet to update the cluster functional level.)"},
	{.status = 5975, .dos_err = 317, .posix_err = 0, .msg = R"(

The cluster functional level has been successfully updated but not all features are available yet. Restart the cluster by

using the Stop-Cluster PowerShell cmdlet followed by the Start-Cluster PowerShell cmdlet and all cluster features will

be available.)"},
	{.status = 5976, .dos_err = 317, .posix_err = 0, .msg = R"(

The cluster is currently performing a version upgrade.)"},
	{.status = 5977, .dos_err = 317, .posix_err = 0, .msg = R"(

The cluster did not successfully complete the version upgrade.)"},
	{.status = 5978, .dos_err = 317, .posix_err = 0, .msg = R"(

The cluster node is in grace period.)"},
	{.status = 5979, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation has failed because CSV volume was not able to recover in time specified on this file object.)"},
	{.status = 5980, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because the requested node is not currently part of active cluster membership.)"},
	{.status = 5981, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because the requested cluster resource is currently unmonitored.)"},
	{.status = 5982, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because a resource does not support running in an unmonitored state.)"},
	{.status = 5983, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation cannot be completed because a resource participates in replication.)"},
	{.status = 5984, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because the requested cluster node has been isolated)"},
	{.status = 5985, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because the requested cluster node has been quarantined)"},
	{.status = 5986, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because the specified database update condition was not met)"},
	{.status = 5987, .dos_err = 317, .posix_err = 0, .msg = R"(

A clustered space is in a degraded condition and the requested action cannot be completed at this time.)"},
	{.status = 5988, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation failed because token delegation for this control is not supported.)"},
	{.status = 5989, .dos_err = 317, .posix_err = 0, .msg = R"(

The operation has failed because CSV has invalidated this file object.)"},
	{.status = 5990, .dos_err = 317, .posix_err = 0, .msg = R"(

This operation is supported only on the CSV coordinator node.)"},
	{.status = 5991, .dos_err = 317, .posix_err = 0, .msg = R"(

The cluster group set is not available for any further requests.)"},
	{.status = 5992, .dos_err = 317, .posix_err = 0, .msg = R"(

The cluster group set could not be found.)"},
	{.status = 5993, .dos_err = 317, .posix_err = 0, .msg = R"(

The action cannot be completed at this time because the cluster group set would fall below quorum and not be able to act as a provider.)"},
	{.status = 5994, .dos_err = 317, .posix_err = 0, .msg = R"(

The specified parent fault domain is not found.)"},
	{.status = 5995, .dos_err = 317, .posix_err = 0, .msg = R"(

The fault domain cannot be a child of the parent specified.)"},
	{.status = 5996, .dos_err = 317, .posix_err = 0, .msg = R"(

Storage Spaces Direct has rejected the proposed fault domain changes because it impacts the fault tolerance of the storage.)"},
	{.status = 5997, .dos_err = 317, .posix_err = 0, .msg = R"(

Storage Spaces Direct has rejected the proposed fault domain changes because it reduces the storage connected to the system.)"},
	{.status = 5998, .dos_err = 317, .posix_err = 0, .msg = R"(

Cluster infrastructure file server creation failed because a valid non-empty file server name was not provided.)"},
	{.status = 5999, .dos_err = 317, .posix_err = 0, .msg = R"(

The action cannot be completed because the cluster set management cluster is unreachable.)"},
	{.status = 6000, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file could not be encrypted.)"},
	{.status = 6001, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file could not be decrypted.)"},
	{.status = 6002, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file is encrypted and the user does not have the ability to decrypt it.)"},
	{.status = 6003, .dos_err = 317, .posix_err = 0, .msg = R"(There is no valid encryption recovery policy configured for this system.)"},
	{.status = 6004, .dos_err = 317, .posix_err = 0, .msg = R"(The required encryption driver is not loaded for this system.)"},
	{.status = 6005, .dos_err = 317, .posix_err = 0, .msg = R"(The file was encrypted with a different encryption driver than is currently loaded.)"},
	{.status = 6006, .dos_err = 317, .posix_err = 0, .msg = R"(There are no EFS keys defined for the user.)"},
	{.status = 6007, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file is not encrypted.)"},
	{.status = 6008, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file is not in the defined EFS export format.)"},
	{.status = 6009, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file is read only.)"},
	{.status = 6010, .dos_err = 317, .posix_err = 0, .msg = R"(The directory has been disabled for encryption.)"},
	{.status = 6011, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not trusted for remote encryption operation.)"},
	{.status = 6012, .dos_err = 317, .posix_err = 0, .msg = R"(Recovery policy configured for this system contains invalid recovery certificate.)"},
	{.status = 6013, .dos_err = 317, .posix_err = 0, .msg = R"(The encryption algorithm used on the source file needs a bigger key buffer than the one on the destination file.)"},
	{.status = 6014, .dos_err = 317, .posix_err = 0, .msg = R"(The disk partition does not support file encryption.)"},
	{.status = 6015, .dos_err = 317, .posix_err = 0, .msg = R"(This machine is disabled for file encryption.)"},
	{.status = 6016, .dos_err = 317, .posix_err = 0, .msg = R"(A newer system is required to decrypt this encrypted file.)"},
	{.status = 6017, .dos_err = 317, .posix_err = 0, .msg = R"(The remote server sent an invalid response for a file being opened with Client Side Encryption.)"},
	{.status = 6018, .dos_err = 317, .posix_err = 0, .msg = R"(Client Side Encryption is not supported by the remote server even though it claims to support it.)"},
	{.status = 6019, .dos_err = 317, .posix_err = 0, .msg = R"(File is encrypted and should be opened in Client Side Encryption mode.)"},
	{.status = 6020, .dos_err = 317, .posix_err = 0, .msg = R"(A new encrypted file is being created and a $EFS needs to be provided.)"},
	{.status = 6021, .dos_err = 317, .posix_err = 0, .msg = R"(The SMB client requested a CSE FSCTL on a non-CSE file.)"},
	{.status = 6022, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation was blocked by policy. For more information, contact your system administrator.)"},
	{.status = 6023, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file could not be encrypted with Windows Information Protection.)"},
	{.status = 6118, .dos_err = 317, .posix_err = 0, .msg = R"(The list of servers for this workgroup is not currently available)"},
	{.status = 6200, .dos_err = 317, .posix_err = 0, .msg = R"(The Task Scheduler service must be configured to run in the System account to function properly. Individual tasks may be configured to run in other accounts.)"},
	{.status = 6250, .dos_err = 317, .posix_err = 0, .msg = R"(

The object cannot be deleted from the local cluster because it is registered with the cluster set.)"},
	{.status = 6600, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an invalid log sector.)"},
	{.status = 6601, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a log sector with invalid block parity.)"},
	{.status = 6602, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a remapped log sector.)"},
	{.status = 6603, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a partial or incomplete log block.)"},
	{.status = 6604, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an attempt access data outside the active log range.)"},
	{.status = 6605, .dos_err = 317, .posix_err = 0, .msg = R"(Log service user marshalling buffers are exhausted.)"},
	{.status = 6606, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an attempt read from a marshalling area with an invalid read context.)"},
	{.status = 6607, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an invalid log restart area.)"},
	{.status = 6608, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an invalid log block version.)"},
	{.status = 6609, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an invalid log block.)"},
	{.status = 6610, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an attempt to read the log with an invalid read mode.)"},
	{.status = 6611, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a log stream with no restart area.)"},
	{.status = 6612, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a corrupted metadata file.)"},
	{.status = 6613, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a metadata file that could not be created by the log file system.)"},
	{.status = 6614, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered a metadata file with inconsistent data.)"},
	{.status = 6615, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an attempt to erroneous allocate or dispose reservation space.)"},
	{.status = 6616, .dos_err = 317, .posix_err = 0, .msg = R"(Log service cannot delete log file or file system container.)"},
	{.status = 6617, .dos_err = 317, .posix_err = 0, .msg = R"(Log service has reached the maximum allowable containers allocated to a log file.)"},
	{.status = 6618, .dos_err = 317, .posix_err = 0, .msg = R"(Log service has attempted to read or write backward past the start of the log.)"},
	{.status = 6619, .dos_err = 317, .posix_err = 0, .msg = R"(Log policy could not be installed because a policy of the same type is already present.)"},
	{.status = 6620, .dos_err = 317, .posix_err = 0, .msg = R"(Log policy in question was not installed at the time of the request.)"},
	{.status = 6621, .dos_err = 317, .posix_err = 0, .msg = R"(The installed set of policies on the log is invalid.)"},
	{.status = 6622, .dos_err = 317, .posix_err = 0, .msg = R"(A policy on the log in question prevented the operation from completing.)"},
	{.status = 6623, .dos_err = 317, .posix_err = 0, .msg = R"(Log space cannot be reclaimed because the log is pinned by the archive tail.)"},
	{.status = 6624, .dos_err = 317, .posix_err = 0, .msg = R"(Log record is not a record in the log file.)"},
	{.status = 6625, .dos_err = 317, .posix_err = 0, .msg = R"(Number of reserved log records or the adjustment of the number of reserved log records is invalid.)"},
	{.status = 6626, .dos_err = 317, .posix_err = 0, .msg = R"(Reserved log space or the adjustment of the log space is invalid.)"},
	{.status = 6627, .dos_err = 317, .posix_err = 0, .msg = R"(An new or existing archive tail or base of the active log is invalid.)"},
	{.status = 6628, .dos_err = 317, .posix_err = 0, .msg = R"(Log space is exhausted.)"},
	{.status = 6629, .dos_err = 317, .posix_err = 0, .msg = R"(The log could not be set to the requested size.)"},
	{.status = 6630, .dos_err = 317, .posix_err = 0, .msg = R"(Log is multiplexed, no direct writes to the physical log is allowed.)"},
	{.status = 6631, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because the log is a dedicated log.)"},
	{.status = 6632, .dos_err = 317, .posix_err = 0, .msg = R"(The operation requires an archive context.)"},
	{.status = 6633, .dos_err = 317, .posix_err = 0, .msg = R"(Log archival is in progress.)"},
	{.status = 6634, .dos_err = 317, .posix_err = 0, .msg = R"(The operation requires a non-ephemeral log, but the log is ephemeral.)"},
	{.status = 6635, .dos_err = 317, .posix_err = 0, .msg = R"(The log must have at least two containers before it can be read from or written to.)"},
	{.status = 6636, .dos_err = 317, .posix_err = 0, .msg = R"(A log client has already registered on the stream.)"},
	{.status = 6637, .dos_err = 317, .posix_err = 0, .msg = R"(A log client has not been registered on the stream.)"},
	{.status = 6638, .dos_err = 317, .posix_err = 0, .msg = R"(A request has already been made to handle the log full condition.)"},
	{.status = 6639, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an error when attempting to read from a log container.)"},
	{.status = 6640, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an error when attempting to write to a log container.)"},
	{.status = 6641, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an error when attempting open a log container.)"},
	{.status = 6642, .dos_err = 317, .posix_err = 0, .msg = R"(Log service encountered an invalid container state when attempting a requested action.)"},
	{.status = 6643, .dos_err = 317, .posix_err = 0, .msg = R"(Log service is not in the correct state to perform a requested action.)"},
	{.status = 6644, .dos_err = 317, .posix_err = 0, .msg = R"(Log space cannot be reclaimed because the log is pinned.)"},
	{.status = 6645, .dos_err = 317, .posix_err = 0, .msg = R"(Log metadata flush failed.)"},
	{.status = 6646, .dos_err = 317, .posix_err = 0, .msg = R"(Security on the log and its containers is inconsistent.)"},
	{.status = 6647, .dos_err = 317, .posix_err = 0, .msg = R"(Records were appended to the log or reservation changes were made, but the log could not be flushed.)"},
	{.status = 6648, .dos_err = 317, .posix_err = 0, .msg = R"(The log is pinned due to reservation consuming most of the log space. Free some reserved records to make space available.)"},
	{.status = 6700, .dos_err = 317, .posix_err = 0, .msg = R"(The transaction handle associated with this operation is not valid.)"},
	{.status = 6701, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation was made in the context of a transaction that is no longer active.)"},
	{.status = 6702, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is not valid on the Transaction object in its current state.)"},
	{.status = 6703, .dos_err = 317, .posix_err = 0, .msg = R"(The caller has called a response API, but the response is not expected because the TM did not issue the corresponding request to the caller.)"},
	{.status = 6704, .dos_err = 317, .posix_err = 0, .msg = R"(It is too late to perform the requested operation, since the Transaction has already been aborted.)"},
	{.status = 6705, .dos_err = 317, .posix_err = 0, .msg = R"(It is too late to perform the requested operation, since the Transaction has already been committed.)"},
	{.status = 6706, .dos_err = 317, .posix_err = 0, .msg = R"(The Transaction Manager was unable to be successfully initialized. Transacted operations are not supported.)"},
	{.status = 6707, .dos_err = 317, .posix_err = 0, .msg = R"(The specified ResourceManager made no changes or updates to the resource under this transaction.)"},
	{.status = 6708, .dos_err = 317, .posix_err = 0, .msg = R"(The resource manager has attempted to prepare a transaction that it has not successfully joined.)"},
	{.status = 6709, .dos_err = 317, .posix_err = 0, .msg = R"(The Transaction object already has a superior enlistment, and the caller attempted an operation that would have created a new superior. Only a single superior enlistment is allow.)"},
	{.status = 6710, .dos_err = 317, .posix_err = 0, .msg = R"(The RM tried to register a protocol that already exists.)"},
	{.status = 6711, .dos_err = 317, .posix_err = 0, .msg = R"(The attempt to propagate the Transaction failed.)"},
	{.status = 6712, .dos_err = 317, .posix_err = 0, .msg = R"(The requested propagation protocol was not registered as a CRM.)"},
	{.status = 6713, .dos_err = 317, .posix_err = 0, .msg = R"(The buffer passed in to PushTransaction or PullTransaction is not in a valid format.)"},
	{.status = 6714, .dos_err = 317, .posix_err = 0, .msg = R"(The current transaction context associated with the thread is not a valid handle to a transaction object.)"},
	{.status = 6715, .dos_err = 317, .posix_err = 0, .msg = R"(The specified Transaction object could not be opened, because it was not found.)"},
	{.status = 6716, .dos_err = 317, .posix_err = 0, .msg = R"(The specified ResourceManager object could not be opened, because it was not found.)"},
	{.status = 6717, .dos_err = 317, .posix_err = 0, .msg = R"(The specified Enlistment object could not be opened, because it was not found.)"},
	{.status = 6718, .dos_err = 317, .posix_err = 0, .msg = R"(The specified TransactionManager object could not be opened, because it was not found.)"},
	{.status = 6719, .dos_err = 317, .posix_err = 0, .msg = R"(The object specified could not be created or opened, because its associated TransactionManager is not online.  The TransactionManager must be brought fully Online by calling RecoverTransactionManager to recover to the end of its LogFile before objects in its Transaction or ResourceManager namespaces can be opened.  In addition, errors in writing records to its LogFile can cause a TransactionManager to go offline.)"},
	{.status = 6720, .dos_err = 317, .posix_err = 0, .msg = R"(The specified TransactionManager was unable to create the objects contained in its logfile in the Ob namespace. Therefore, the TransactionManager was unable to recover.)"},
	{.status = 6721, .dos_err = 317, .posix_err = 0, .msg = R"(The call to create a superior Enlistment on this Transaction object could not be completed, because the Transaction object specified for the enlistment is a subordinate branch of the Transaction. Only the root of the Transaction can be enlisted on as a superior.)"},
	{.status = 6722, .dos_err = 317, .posix_err = 0, .msg = R"(Because the associated transaction manager or resource manager has been closed, the handle is no longer valid.)"},
	{.status = 6723, .dos_err = 317, .posix_err = 0, .msg = R"(The specified operation could not be performed on this Superior enlistment, because the enlistment was not created with the corresponding completion response in the NotificationMask.)"},
	{.status = 6724, .dos_err = 317, .posix_err = 0, .msg = R"(The specified operation could not be performed, because the record that would be logged was too long. This can occur because of two conditions: either there are too many Enlistments on this Transaction, or the combined RecoveryInformation being logged on behalf of those Enlistments is too long.)"},
	{.status = 6725, .dos_err = 317, .posix_err = 0, .msg = R"(Implicit transaction are not supported.)"},
	{.status = 6726, .dos_err = 317, .posix_err = 0, .msg = R"(The kernel transaction manager had to abort or forget the transaction because it blocked forward progress.)"},
	{.status = 6727, .dos_err = 317, .posix_err = 0, .msg = R"(The TransactionManager identity that was supplied did not match the one recorded in the TransactionManager's log file.)"},
	{.status = 6728, .dos_err = 317, .posix_err = 0, .msg = R"(This snapshot operation cannot continue because a transactional resource manager cannot be frozen in its current state.  Please try again.)"},
	{.status = 6729, .dos_err = 317, .posix_err = 0, .msg = R"(The transaction cannot be enlisted on with the specified EnlistmentMask, because the transaction has already completed the PrePrepare phase.  In order to ensure correctness, the ResourceManager must switch to a write-through mode and cease caching data within this transaction.  Enlisting for only subsequent transaction phases may still succeed.)"},
	{.status = 6730, .dos_err = 317, .posix_err = 0, .msg = R"(The transaction does not have a superior enlistment.)"},
	{.status = 6731, .dos_err = 317, .posix_err = 0, .msg = R"(The attempt to commit the Transaction completed, but it is possible that some portion of the transaction tree did not commit successfully due to heuristics.  Therefore it is possible that some data modified in the transaction may not have committed, resulting in transactional inconsistency.  If possible, check the consistency of the associated data.)"},
	{.status = 6800, .dos_err = 317, .posix_err = 0, .msg = R"(The function attempted to use a name that is reserved for use by another transaction.)"},
	{.status = 6801, .dos_err = 317, .posix_err = 0, .msg = R"(Transaction support within the specified resource manager is not started or was shut down due to an error.)"},
	{.status = 6802, .dos_err = 317, .posix_err = 0, .msg = R"(The metadata of the RM has been corrupted. The RM will not function.)"},
	{.status = 6803, .dos_err = 317, .posix_err = 0, .msg = R"(The specified directory does not contain a resource manager.)"},
	{.status = 6805, .dos_err = 317, .posix_err = 0, .msg = R"(The remote server or share does not support transacted file operations.)"},
	{.status = 6806, .dos_err = 317, .posix_err = 0, .msg = R"(The requested log size is invalid.)"},
	{.status = 6807, .dos_err = 317, .posix_err = 0, .msg = R"(The object (file, stream, link) corresponding to the handle has been deleted by a Transaction Savepoint Rollback.)"},
	{.status = 6808, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file miniversion was not found for this transacted file open.)"},
	{.status = 6809, .dos_err = 317, .posix_err = 0, .msg = R"(The specified file miniversion was found but has been invalidated. Most likely cause is a transaction savepoint rollback.)"},
	{.status = 6810, .dos_err = 317, .posix_err = 0, .msg = R"(A miniversion may only be opened in the context of the transaction that created it.)"},
	{.status = 6811, .dos_err = 317, .posix_err = 0, .msg = R"(It is not possible to open a miniversion with modify access.)"},
	{.status = 6812, .dos_err = 317, .posix_err = 0, .msg = R"(It is not possible to create any more miniversions for this stream.)"},
	{.status = 6814, .dos_err = 317, .posix_err = 0, .msg = R"(The remote server sent mismatching version number or Fid for a file opened with transactions.)"},
	{.status = 6815, .dos_err = 317, .posix_err = 0, .msg = R"(The handle has been invalidated by a transaction. The most likely cause is the presence of memory mapping on a file or an open handle when the transaction ended or rolled back to savepoint.)"},
	{.status = 6816, .dos_err = 317, .posix_err = 0, .msg = R"(There is no transaction metadata on the file.)"},
	{.status = 6817, .dos_err = 317, .posix_err = 0, .msg = R"(The log data is corrupt.)"},
	{.status = 6818, .dos_err = 317, .posix_err = 0, .msg = R"(The file can't be recovered because there is a handle still open on it.)"},
	{.status = 6819, .dos_err = 317, .posix_err = 0, .msg = R"(The transaction outcome is unavailable because the resource manager responsible for it has disconnected.)"},
	{.status = 6820, .dos_err = 317, .posix_err = 0, .msg = R"(The request was rejected because the enlistment in question is not a superior enlistment.)"},
	{.status = 6821, .dos_err = 317, .posix_err = 0, .msg = R"(The transactional resource manager is already consistent. Recovery is not needed.)"},
	{.status = 6822, .dos_err = 317, .posix_err = 0, .msg = R"(The transactional resource manager has already been started.)"},
	{.status = 6823, .dos_err = 317, .posix_err = 0, .msg = R"(The file cannot be opened transactionally, because its identity depends on the outcome of an unresolved transaction.)"},
	{.status = 6824, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed because another transaction is depending on the fact that this property will not change.)"},
	{.status = 6825, .dos_err = 317, .posix_err = 0, .msg = R"(The operation would involve a single file with two transactional resource managers and is therefore not allowed.)"},
	{.status = 6826, .dos_err = 317, .posix_err = 0, .msg = R"(The $Txf directory must be empty for this operation to succeed.)"},
	{.status = 6827, .dos_err = 317, .posix_err = 0, .msg = R"(The operation would leave a transactional resource manager in an inconsistent state and is therefore not allowed.)"},
	{.status = 6828, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed because the transaction manager does not have a log.)"},
	{.status = 6829, .dos_err = 317, .posix_err = 0, .msg = R"(A rollback could not be scheduled because a previously scheduled rollback has already executed or been queued for execution.)"},
	{.status = 6830, .dos_err = 317, .posix_err = 0, .msg = R"(The transactional metadata attribute on the file or directory is corrupt and unreadable.)"},
	{.status = 6831, .dos_err = 317, .posix_err = 0, .msg = R"(The encryption operation could not be completed because a transaction is active.)"},
	{.status = 6832, .dos_err = 317, .posix_err = 0, .msg = R"(This object is not allowed to be opened in a transaction.)"},
	{.status = 6833, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to create space in the transactional resource manager's log failed. The failure status has been recorded in the event log.)"},
	{.status = 6834, .dos_err = 317, .posix_err = 0, .msg = R"(Memory mapping (creating a mapped section) a remote file under a transaction is not supported.)"},
	{.status = 6835, .dos_err = 317, .posix_err = 0, .msg = R"(Transaction metadata is already present on this file and cannot be superseded.)"},
	{.status = 6836, .dos_err = 317, .posix_err = 0, .msg = R"(A transaction scope could not be entered because the scope handler has not been initialized.)"},
	{.status = 6837, .dos_err = 317, .posix_err = 0, .msg = R"(Promotion was required in order to allow the resource manager to enlist, but the transaction was set to disallow it.)"},
	{.status = 6838, .dos_err = 317, .posix_err = 0, .msg = R"(This file is open for modification in an unresolved transaction and may be opened for execute only by a transacted reader.)"},
	{.status = 6839, .dos_err = 317, .posix_err = 0, .msg = R"(The request to thaw frozen transactions was ignored because transactions had not previously been frozen.)"},
	{.status = 6840, .dos_err = 317, .posix_err = 0, .msg = R"(Transactions cannot be frozen because a freeze is already in progress.)"},
	{.status = 6841, .dos_err = 317, .posix_err = 0, .msg = R"(The target volume is not a snapshot volume. This operation is only valid on a volume mounted as a snapshot.)"},
	{.status = 6842, .dos_err = 317, .posix_err = 0, .msg = R"(The savepoint operation failed because files are open on the transaction. This is not permitted.)"},
	{.status = 6843, .dos_err = 317, .posix_err = 0, .msg = R"(Windows has discovered corruption in a file, and that file has since been repaired. Data loss may have occurred.)"},
	{.status = 6844, .dos_err = 317, .posix_err = 0, .msg = R"(The sparse operation could not be completed because a transaction is active on the file.)"},
	{.status = 6845, .dos_err = 317, .posix_err = 0, .msg = R"(The call to create a TransactionManager object failed because the Tm Identity stored in the logfile does not match the Tm Identity that was passed in as an argument.)"},
	{.status = 6846, .dos_err = 317, .posix_err = 0, .msg = R"(I/O was attempted on a section object that has been floated as a result of a transaction ending. There is no valid data.)"},
	{.status = 6847, .dos_err = 317, .posix_err = 0, .msg = R"(The transactional resource manager cannot currently accept transacted work due to a transient condition such as low resources.)"},
	{.status = 6848, .dos_err = 317, .posix_err = 0, .msg = R"(The transactional resource manager had too many transactions outstanding that could not be aborted. The transactional resource manger has been shut down.)"},
	{.status = 6849, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed due to bad clusters on disk.)"},
	{.status = 6850, .dos_err = 317, .posix_err = 0, .msg = R"(The compression operation could not be completed because a transaction is active on the file.)"},
	{.status = 6851, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be completed because the volume is dirty. Please run chkdsk and try again.)"},
	{.status = 6852, .dos_err = 317, .posix_err = 0, .msg = R"(The link tracking operation could not be completed because a transaction is active.)"},
	{.status = 6853, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed in a transaction.)"},
	{.status = 6854, .dos_err = 317, .posix_err = 0, .msg = R"(The handle is no longer properly associated with its transaction.  It may have been opened in a transactional resource manager that was subsequently forced to restart.  Please close the handle and open a new one.)"},
	{.status = 6855, .dos_err = 317, .posix_err = 0, .msg = R"(The specified operation could not be performed because the resource manager is not enlisted in the transaction.)"},
	{.status = 7001, .dos_err = 317, .posix_err = 0, .msg = R"(The specified session name is invalid.)"},
	{.status = 7002, .dos_err = 317, .posix_err = 0, .msg = R"(The specified protocol driver is invalid.)"},
	{.status = 7003, .dos_err = 317, .posix_err = 0, .msg = R"(The specified protocol driver was not found in the system path.)"},
	{.status = 7004, .dos_err = 317, .posix_err = 0, .msg = R"(The specified terminal connection driver was not found in the system path.)"},
	{.status = 7005, .dos_err = 317, .posix_err = 0, .msg = R"(A registry key for event logging could not be created for this session.)"},
	{.status = 7006, .dos_err = 317, .posix_err = 0, .msg = R"(A service with the same name already exists on the system.)"},
	{.status = 7007, .dos_err = 317, .posix_err = 0, .msg = R"(A close operation is pending on the session.)"},
	{.status = 7008, .dos_err = 317, .posix_err = 0, .msg = R"(There are no free output buffers available.)"},
	{.status = 7009, .dos_err = 317, .posix_err = 0, .msg = R"(The MODEM.INF file was not found.)"},
	{.status = 7010, .dos_err = 317, .posix_err = 0, .msg = R"(The modem name was not found in MODEM.INF.)"},
	{.status = 7011, .dos_err = 317, .posix_err = 0, .msg = R"(The modem did not accept the command sent to it. Verify that the configured modem name matches the attached modem.)"},
	{.status = 7012, .dos_err = 317, .posix_err = 0, .msg = R"(The modem did not respond to the command sent to it. Verify that the modem is properly cabled and powered on.)"},
	{.status = 7013, .dos_err = 317, .posix_err = 0, .msg = R"(Carrier detect has failed or carrier has been dropped due to disconnect.)"},
	{.status = 7014, .dos_err = 317, .posix_err = 0, .msg = R"(Dial tone not detected within the required time. Verify that the phone cable is properly attached and functional.)"},
	{.status = 7015, .dos_err = 317, .posix_err = 0, .msg = R"(Busy signal detected at remote site on callback.)"},
	{.status = 7016, .dos_err = 317, .posix_err = 0, .msg = R"(Voice detected at remote site on callback.)"},
	{.status = 7017, .dos_err = 317, .posix_err = 0, .msg = R"(Transport driver error)"},
	{.status = 7022, .dos_err = 317, .posix_err = 0, .msg = R"(The specified session cannot be found.)"},
	{.status = 7023, .dos_err = 317, .posix_err = 0, .msg = R"(The specified session name is already in use.)"},
	{.status = 7024, .dos_err = 317, .posix_err = 0, .msg = R"(The task you are trying to do can't be completed because Remote Desktop Services is currently busy. Please try again in a few minutes. Other users should still be able to log on.)"},
	{.status = 7025, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt has been made to connect to a session whose video mode is not supported by the current client.)"},
	{.status = 7035, .dos_err = 317, .posix_err = 0, .msg = R"(The application attempted to enable DOS graphics mode. DOS graphics mode is not supported.)"},
	{.status = 7037, .dos_err = 317, .posix_err = 0, .msg = R"(Your interactive logon privilege has been disabled. Please contact your administrator.)"},
	{.status = 7038, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation can be performed only on the system console. This is most often the result of a driver or system DLL requiring direct console access.)"},
	{.status = 7040, .dos_err = 317, .posix_err = 0, .msg = R"(The client failed to respond to the server connect message.)"},
	{.status = 7041, .dos_err = 317, .posix_err = 0, .msg = R"(Disconnecting the console session is not supported.)"},
	{.status = 7042, .dos_err = 317, .posix_err = 0, .msg = R"(Reconnecting a disconnected session to the console is not supported.)"},
	{.status = 7044, .dos_err = 317, .posix_err = 0, .msg = R"(The request to control another session remotely was denied.)"},
	{.status = 7045, .dos_err = 317, .posix_err = 0, .msg = R"(The requested session access is denied.)"},
	{.status = 7049, .dos_err = 317, .posix_err = 0, .msg = R"(The specified terminal connection driver is invalid.)"},
	{.status = 7050, .dos_err = 317, .posix_err = 0, .msg = R"(The requested session cannot be controlled remotely.

This may be because the session is disconnected or does not currently have a user logged on.)"},
	{.status = 7051, .dos_err = 317, .posix_err = 0, .msg = R"(The requested session is not configured to allow remote control.)"},
	{.status = 7052, .dos_err = 317, .posix_err = 0, .msg = R"(Your request to connect to this Terminal Server has been rejected. Your Terminal Server client license number is currently being used by another user. Please call your system administrator to obtain a unique license number.)"},
	{.status = 7053, .dos_err = 317, .posix_err = 0, .msg = R"(Your request to connect to this Terminal Server has been rejected. Your Terminal Server client license number has not been entered for this copy of the Terminal Server client. Please contact your system administrator.)"},
	{.status = 7054, .dos_err = 317, .posix_err = 0, .msg = R"(The number of connections to this computer is limited and all connections are in use right now. Try connecting later or contact your system administrator.)"},
	{.status = 7055, .dos_err = 317, .posix_err = 0, .msg = R"(The client you are using is not licensed to use this system. Your logon request is denied.)"},
	{.status = 7056, .dos_err = 317, .posix_err = 0, .msg = R"(The system license has expired. Your logon request is denied.)"},
	{.status = 7057, .dos_err = 317, .posix_err = 0, .msg = R"(Remote control could not be terminated because the specified session is not currently being remotely controlled.)"},
	{.status = 7058, .dos_err = 317, .posix_err = 0, .msg = R"(The remote control of the console was terminated because the display mode was changed. Changing the display mode in a remote control session is not supported.)"},
	{.status = 7059, .dos_err = 317, .posix_err = 0, .msg = R"(Activation has already been reset the maximum number of times for this installation. Your activation timer will not be cleared.)"},
	{.status = 7060, .dos_err = 317, .posix_err = 0, .msg = R"(Remote logins are currently disabled.)"},
	{.status = 7061, .dos_err = 317, .posix_err = 0, .msg = R"(You do not have the proper encryption level to access this Session.)"},
	{.status = 7062, .dos_err = 317, .posix_err = 0, .msg = R"(The user %s\\%s is currently logged on to this computer. Only the current user or an administrator can log on to this computer.)"},
	{.status = 7063, .dos_err = 317, .posix_err = 0, .msg = R"(The user %s\\%s is already logged on to the console of this computer. You do not have permission to log in at this time. To resolve this issue, contact %s\\%s and have them log off.)"},
	{.status = 7064, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to log you on because of an account restriction.)"},
	{.status = 7065, .dos_err = 317, .posix_err = 0, .msg = R"(The RDP protocol component %2 detected an error in the protocol stream and has disconnected the client.)"},
	{.status = 7066, .dos_err = 317, .posix_err = 0, .msg = R"(The Client Drive Mapping Service Has Connected on Terminal Connection.)"},
	{.status = 7067, .dos_err = 317, .posix_err = 0, .msg = R"(The Client Drive Mapping Service Has Disconnected on Terminal Connection.)"},
	{.status = 7068, .dos_err = 317, .posix_err = 0, .msg = R"(The Terminal Server security layer detected an error in the protocol stream and has disconnected the client.)"},
	{.status = 7069, .dos_err = 317, .posix_err = 0, .msg = R"(The target session is incompatible with the current session.)"},
	{.status = 7070, .dos_err = 317, .posix_err = 0, .msg = R"(Windows can't connect to your session because a problem occurred in the Windows video subsystem. Try connecting again later, or contact the server administrator for assistance.)"},
	{.status = 8001, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service API was called incorrectly.)"},
	{.status = 8002, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot be started.)"},
	{.status = 8003, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot be stopped.)"},
	{.status = 8004, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service API terminated the request. The event log may have more information.)"},
	{.status = 8005, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service terminated the request. The event log may have more information.)"},
	{.status = 8006, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot be contacted. The event log may have more information.)"},
	{.status = 8007, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot satisfy the request because the user has insufficient privileges. The event log may have more information.)"},
	{.status = 8008, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot satisfy the request because authenticated RPC is not available. The event log may have more information.)"},
	{.status = 8009, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot satisfy the request because the user has insufficient privileges on the domain controller. The event log may have more information.)"},
	{.status = 8010, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot satisfy the request because authenticated RPC is not available on the domain controller. The event log may have more information.)"},
	{.status = 8011, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot communicate with the file replication service on the domain controller. The event log may have more information.)"},
	{.status = 8012, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service on the domain controller cannot communicate with the file replication service on this computer. The event log may have more information.)"},
	{.status = 8013, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot populate the system volume because of an internal error. The event log may have more information.)"},
	{.status = 8014, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot populate the system volume because of an internal timeout. The event log may have more information.)"},
	{.status = 8015, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot process the request. The system volume is busy with a previous request.)"},
	{.status = 8016, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service cannot stop replicating the system volume because of an internal error. The event log may have more information.)"},
	{.status = 8017, .dos_err = 317, .posix_err = 0, .msg = R"(The file replication service detected an invalid parameter.)"},
	{.status = 8200, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while installing the directory service. For more information, see the event log.)"},
	{.status = 8201, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service evaluated group memberships locally.)"},
	{.status = 8202, .dos_err = 317, .posix_err = 0, .msg = R"(The specified directory service attribute or value does not exist.)"},
	{.status = 8203, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute syntax specified to the directory service is invalid.)"},
	{.status = 8204, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute type specified to the directory service is not defined.)"},
	{.status = 8205, .dos_err = 317, .posix_err = 0, .msg = R"(The specified directory service attribute or value already exists.)"},
	{.status = 8206, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service is busy.)"},
	{.status = 8207, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service is unavailable.)"},
	{.status = 8208, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service was unable to allocate a relative identifier.)"},
	{.status = 8209, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service has exhausted the pool of relative identifiers.)"},
	{.status = 8210, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation could not be performed because the directory service is not the master for that type of operation.)"},
	{.status = 8211, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service was unable to initialize the subsystem that allocates relative identifiers.)"},
	{.status = 8212, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation did not satisfy one or more constraints associated with the class of the object.)"},
	{.status = 8213, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service can perform the requested operation only on a leaf object.)"},
	{.status = 8214, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service cannot perform the requested operation on the RDN attribute of an object.)"},
	{.status = 8215, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service detected an attempt to modify the object class of an object.)"},
	{.status = 8216, .dos_err = 317, .posix_err = 0, .msg = R"(The requested cross-domain move operation could not be performed.)"},
	{.status = 8217, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to contact the global catalog server.)"},
	{.status = 8218, .dos_err = 317, .posix_err = 0, .msg = R"(The policy object is shared and can only be modified at the root.)"},
	{.status = 8219, .dos_err = 317, .posix_err = 0, .msg = R"(The policy object does not exist.)"},
	{.status = 8220, .dos_err = 317, .posix_err = 0, .msg = R"(The requested policy information is only in the directory service.)"},
	{.status = 8221, .dos_err = 317, .posix_err = 0, .msg = R"(A domain controller promotion is currently active.)"},
	{.status = 8222, .dos_err = 317, .posix_err = 0, .msg = R"(A domain controller promotion is not currently active)"},
	{.status = 8224, .dos_err = 317, .posix_err = 0, .msg = R"(An operations error occurred.)"},
	{.status = 8225, .dos_err = 317, .posix_err = 0, .msg = R"(A protocol error occurred.)"},
	{.status = 8226, .dos_err = 317, .posix_err = 0, .msg = R"(The time limit for this request was exceeded.)"},
	{.status = 8227, .dos_err = 317, .posix_err = 0, .msg = R"(The size limit for this request was exceeded.)"},
	{.status = 8228, .dos_err = 317, .posix_err = 0, .msg = R"(The administrative limit for this request was exceeded.)"},
	{.status = 8229, .dos_err = 317, .posix_err = 0, .msg = R"(The compare response was false.)"},
	{.status = 8230, .dos_err = 317, .posix_err = 0, .msg = R"(The compare response was true.)"},
	{.status = 8231, .dos_err = 317, .posix_err = 0, .msg = R"(The requested authentication method is not supported by the server.)"},
	{.status = 8232, .dos_err = 317, .posix_err = 0, .msg = R"(A more secure authentication method is required for this server.)"},
	{.status = 8233, .dos_err = 317, .posix_err = 0, .msg = R"(Inappropriate authentication.)"},
	{.status = 8234, .dos_err = 317, .posix_err = 0, .msg = R"(The authentication mechanism is unknown.)"},
	{.status = 8235, .dos_err = 317, .posix_err = 0, .msg = R"(A referral was returned from the server.)"},
	{.status = 8236, .dos_err = 317, .posix_err = 0, .msg = R"(The server does not support the requested critical extension.)"},
	{.status = 8237, .dos_err = 317, .posix_err = 0, .msg = R"(This request requires a secure connection.)"},
	{.status = 8238, .dos_err = 317, .posix_err = 0, .msg = R"(Inappropriate matching.)"},
	{.status = 8239, .dos_err = 317, .posix_err = 0, .msg = R"(A constraint violation occurred.)"},
	{.status = 8240, .dos_err = 317, .posix_err = 0, .msg = R"(There is no such object on the server.)"},
	{.status = 8241, .dos_err = 317, .posix_err = 0, .msg = R"(There is an alias problem.)"},
	{.status = 8242, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid dn syntax has been specified.)"},
	{.status = 8243, .dos_err = 317, .posix_err = 0, .msg = R"(The object is a leaf object.)"},
	{.status = 8244, .dos_err = 317, .posix_err = 0, .msg = R"(There is an alias dereferencing problem.)"},
	{.status = 8245, .dos_err = 317, .posix_err = 0, .msg = R"(The server is unwilling to process the request.)"},
	{.status = 8246, .dos_err = 317, .posix_err = 0, .msg = R"(A loop has been detected.)"},
	{.status = 8247, .dos_err = 317, .posix_err = 0, .msg = R"(There is a naming violation.)"},
	{.status = 8248, .dos_err = 317, .posix_err = 0, .msg = R"(The result set is too large.)"},
	{.status = 8249, .dos_err = 317, .posix_err = 0, .msg = R"(The operation affects multiple DSAs)"},
	{.status = 8250, .dos_err = 317, .posix_err = 0, .msg = R"(The server is not operational.)"},
	{.status = 8251, .dos_err = 317, .posix_err = 0, .msg = R"(A local error has occurred.)"},
	{.status = 8252, .dos_err = 317, .posix_err = 0, .msg = R"(An encoding error has occurred.)"},
	{.status = 8253, .dos_err = 317, .posix_err = 0, .msg = R"(A decoding error has occurred.)"},
	{.status = 8254, .dos_err = 317, .posix_err = 0, .msg = R"(The search filter cannot be recognized.)"},
	{.status = 8255, .dos_err = 317, .posix_err = 0, .msg = R"(One or more parameters are illegal.)"},
	{.status = 8256, .dos_err = 317, .posix_err = 0, .msg = R"(The specified method is not supported.)"},
	{.status = 8257, .dos_err = 317, .posix_err = 0, .msg = R"(No results were returned.)"},
	{.status = 8258, .dos_err = 317, .posix_err = 0, .msg = R"(The specified control is not supported by the server.)"},
	{.status = 8259, .dos_err = 317, .posix_err = 0, .msg = R"(A referral loop was detected by the client.)"},
	{.status = 8260, .dos_err = 317, .posix_err = 0, .msg = R"(The preset referral limit was exceeded.)"},
	{.status = 8261, .dos_err = 317, .posix_err = 0, .msg = R"(The search requires a SORT control.)"},
	{.status = 8262, .dos_err = 317, .posix_err = 0, .msg = R"(The search results exceed the offset range specified.)"},
	{.status = 8263, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service detected the subsystem that allocates relative identifiers is disabled. This can occur as a protective mechanism when the system determines a significant portion of relative identifiers (RIDs) have been exhausted. Please see http://go.microsoft.com/fwlink/?LinkId=228610 for recommended diagnostic steps and the procedure to re-enable account creation.)"},
	{.status = 8301, .dos_err = 317, .posix_err = 0, .msg = R"(The root object must be the head of a naming context. The root object cannot have an instantiated parent.)"},
	{.status = 8302, .dos_err = 317, .posix_err = 0, .msg = R"(The add replica operation cannot be performed. The naming context must be writeable in order to create the replica.)"},
	{.status = 8303, .dos_err = 317, .posix_err = 0, .msg = R"(A reference to an attribute that is not defined in the schema occurred.)"},
	{.status = 8304, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum size of an object has been exceeded.)"},
	{.status = 8305, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to add an object to the directory with a name that is already in use.)"},
	{.status = 8306, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to add an object of a class that does not have an RDN defined in the schema.)"},
	{.status = 8307, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to add an object using an RDN that is not the RDN defined in the schema.)"},
	{.status = 8308, .dos_err = 317, .posix_err = 0, .msg = R"(None of the requested attributes were found on the objects.)"},
	{.status = 8309, .dos_err = 317, .posix_err = 0, .msg = R"(The user buffer is too small.)"},
	{.status = 8310, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute specified in the operation is not present on the object.)"},
	{.status = 8311, .dos_err = 317, .posix_err = 0, .msg = R"(Illegal modify operation. Some aspect of the modification is not permitted.)"},
	{.status = 8312, .dos_err = 317, .posix_err = 0, .msg = R"(The specified object is too large.)"},
	{.status = 8313, .dos_err = 317, .posix_err = 0, .msg = R"(The specified instance type is not valid.)"},
	{.status = 8314, .dos_err = 317, .posix_err = 0, .msg = R"(The operation must be performed at a master DSA.)"},
	{.status = 8315, .dos_err = 317, .posix_err = 0, .msg = R"(The object class attribute must be specified.)"},
	{.status = 8316, .dos_err = 317, .posix_err = 0, .msg = R"(A required attribute is missing.)"},
	{.status = 8317, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to modify an object to include an attribute that is not legal for its class.)"},
	{.status = 8318, .dos_err = 317, .posix_err = 0, .msg = R"(The specified attribute is already present on the object.)"},
	{.status = 8320, .dos_err = 317, .posix_err = 0, .msg = R"(The specified attribute is not present, or has no values.)"},
	{.status = 8321, .dos_err = 317, .posix_err = 0, .msg = R"(Multiple values were specified for an attribute that can have only one value.)"},
	{.status = 8322, .dos_err = 317, .posix_err = 0, .msg = R"(A value for the attribute was not in the acceptable range of values.)"},
	{.status = 8323, .dos_err = 317, .posix_err = 0, .msg = R"(The specified value already exists.)"},
	{.status = 8324, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute cannot be removed because it is not present on the object.)"},
	{.status = 8325, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute value cannot be removed because it is not present on the object.)"},
	{.status = 8326, .dos_err = 317, .posix_err = 0, .msg = R"(The specified root object cannot be a subref.)"},
	{.status = 8327, .dos_err = 317, .posix_err = 0, .msg = R"(Chaining is not permitted.)"},
	{.status = 8328, .dos_err = 317, .posix_err = 0, .msg = R"(Chained evaluation is not permitted.)"},
	{.status = 8329, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be performed because the object's parent is either uninstantiated or deleted.)"},
	{.status = 8330, .dos_err = 317, .posix_err = 0, .msg = R"(Having a parent that is an alias is not permitted. Aliases are leaf objects.)"},
	{.status = 8331, .dos_err = 317, .posix_err = 0, .msg = R"(The object and parent must be of the same type, either both masters or both replicas.)"},
	{.status = 8332, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed because child objects exist. This operation can only be performed on a leaf object.)"},
	{.status = 8333, .dos_err = 317, .posix_err = 0, .msg = R"(Directory object not found.)"},
	{.status = 8334, .dos_err = 317, .posix_err = 0, .msg = R"(The aliased object is missing.)"},
	{.status = 8335, .dos_err = 317, .posix_err = 0, .msg = R"(The object name has bad syntax.)"},
	{.status = 8336, .dos_err = 317, .posix_err = 0, .msg = R"(It is not permitted for an alias to refer to another alias.)"},
	{.status = 8337, .dos_err = 317, .posix_err = 0, .msg = R"(The alias cannot be dereferenced.)"},
	{.status = 8338, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is out of scope.)"},
	{.status = 8339, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot continue because the object is in the process of being removed.)"},
	{.status = 8340, .dos_err = 317, .posix_err = 0, .msg = R"(The DSA object cannot be deleted.)"},
	{.status = 8341, .dos_err = 317, .posix_err = 0, .msg = R"(A directory service error has occurred.)"},
	{.status = 8342, .dos_err = 317, .posix_err = 0, .msg = R"(The operation can only be performed on an internal master DSA object.)"},
	{.status = 8343, .dos_err = 317, .posix_err = 0, .msg = R"(The object must be of class DSA.)"},
	{.status = 8344, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient access rights to perform the operation.)"},
	{.status = 8345, .dos_err = 317, .posix_err = 0, .msg = R"(The object cannot be added because the parent is not on the list of possible superiors.)"},
	{.status = 8346, .dos_err = 317, .posix_err = 0, .msg = R"(Access to the attribute is not permitted because the attribute is owned by the Security Accounts Manager (SAM).)"},
	{.status = 8347, .dos_err = 317, .posix_err = 0, .msg = R"(The name has too many parts.)"},
	{.status = 8348, .dos_err = 317, .posix_err = 0, .msg = R"(The name is too long.)"},
	{.status = 8349, .dos_err = 317, .posix_err = 0, .msg = R"(The name value is too long.)"},
	{.status = 8350, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service encountered an error parsing a name.)"},
	{.status = 8351, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service cannot get the attribute type for a name.)"},
	{.status = 8352, .dos_err = 317, .posix_err = 0, .msg = R"(The name does not identify an object; the name identifies a phantom.)"},
	{.status = 8353, .dos_err = 317, .posix_err = 0, .msg = R"(The security descriptor is too short.)"},
	{.status = 8354, .dos_err = 317, .posix_err = 0, .msg = R"(The security descriptor is invalid.)"},
	{.status = 8355, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to create name for deleted object.)"},
	{.status = 8356, .dos_err = 317, .posix_err = 0, .msg = R"(The parent of a new subref must exist.)"},
	{.status = 8357, .dos_err = 317, .posix_err = 0, .msg = R"(The object must be a naming context.)"},
	{.status = 8358, .dos_err = 317, .posix_err = 0, .msg = R"(It is not permitted to add an attribute which is owned by the system.)"},
	{.status = 8359, .dos_err = 317, .posix_err = 0, .msg = R"(The class of the object must be structural; you cannot instantiate an abstract class.)"},
	{.status = 8360, .dos_err = 317, .posix_err = 0, .msg = R"(The schema object could not be found.)"},
	{.status = 8361, .dos_err = 317, .posix_err = 0, .msg = R"(A local object with this GUID (dead or alive) already exists.)"},
	{.status = 8362, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed on a back link.)"},
	{.status = 8363, .dos_err = 317, .posix_err = 0, .msg = R"(The cross reference for the specified naming context could not be found.)"},
	{.status = 8364, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be performed because the directory service is shutting down.)"},
	{.status = 8365, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service request is invalid.)"},
	{.status = 8366, .dos_err = 317, .posix_err = 0, .msg = R"(The role owner attribute could not be read.)"},
	{.status = 8367, .dos_err = 317, .posix_err = 0, .msg = R"(The requested FSMO operation failed. The current FSMO holder could not be contacted.)"},
	{.status = 8368, .dos_err = 317, .posix_err = 0, .msg = R"(Modification of a DN across a naming context is not permitted.)"},
	{.status = 8369, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute cannot be modified because it is owned by the system.)"},
	{.status = 8370, .dos_err = 317, .posix_err = 0, .msg = R"(Only the replicator can perform this function.)"},
	{.status = 8371, .dos_err = 317, .posix_err = 0, .msg = R"(The specified class is not defined.)"},
	{.status = 8372, .dos_err = 317, .posix_err = 0, .msg = R"(The specified class is not a subclass.)"},
	{.status = 8373, .dos_err = 317, .posix_err = 0, .msg = R"(The name reference is invalid.)"},
	{.status = 8374, .dos_err = 317, .posix_err = 0, .msg = R"(A cross reference already exists.)"},
	{.status = 8375, .dos_err = 317, .posix_err = 0, .msg = R"(It is not permitted to delete a master cross reference.)"},
	{.status = 8376, .dos_err = 317, .posix_err = 0, .msg = R"(Subtree notifications are only supported on NC heads.)"},
	{.status = 8377, .dos_err = 317, .posix_err = 0, .msg = R"(Notification filter is too complex.)"},
	{.status = 8378, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: duplicate RDN.)"},
	{.status = 8379, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: duplicate OID.)"},
	{.status = 8380, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: duplicate MAPI identifier.)"},
	{.status = 8381, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: duplicate schema-id GUID.)"},
	{.status = 8382, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: duplicate LDAP display name.)"},
	{.status = 8383, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: range-lower less than range upper.)"},
	{.status = 8384, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: syntax mismatch.)"},
	{.status = 8385, .dos_err = 317, .posix_err = 0, .msg = R"(Schema deletion failed: attribute is used in must-contain.)"},
	{.status = 8386, .dos_err = 317, .posix_err = 0, .msg = R"(Schema deletion failed: attribute is used in may-contain.)"},
	{.status = 8387, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: attribute in may-contain does not exist.)"},
	{.status = 8388, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: attribute in must-contain does not exist.)"},
	{.status = 8389, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: class in aux-class list does not exist or is not an auxiliary class.)"},
	{.status = 8390, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: class in poss-superiors does not exist.)"},
	{.status = 8391, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: class in subclassof list does not exist or does not satisfy hierarchy rules.)"},
	{.status = 8392, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: Rdn-Att-Id has wrong syntax.)"},
	{.status = 8393, .dos_err = 317, .posix_err = 0, .msg = R"(Schema deletion failed: class is used as auxiliary class.)"},
	{.status = 8394, .dos_err = 317, .posix_err = 0, .msg = R"(Schema deletion failed: class is used as sub class.)"},
	{.status = 8395, .dos_err = 317, .posix_err = 0, .msg = R"(Schema deletion failed: class is used as poss superior.)"},
	{.status = 8396, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed in recalculating validation cache.)"},
	{.status = 8397, .dos_err = 317, .posix_err = 0, .msg = R"(The tree deletion is not finished. The request must be made again to continue deleting the tree.)"},
	{.status = 8398, .dos_err = 317, .posix_err = 0, .msg = R"(The requested delete operation could not be performed.)"},
	{.status = 8399, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot read the governs class identifier for the schema record.)"},
	{.status = 8400, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute schema has bad syntax.)"},
	{.status = 8401, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute could not be cached.)"},
	{.status = 8402, .dos_err = 317, .posix_err = 0, .msg = R"(The class could not be cached.)"},
	{.status = 8403, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute could not be removed from the cache.)"},
	{.status = 8404, .dos_err = 317, .posix_err = 0, .msg = R"(The class could not be removed from the cache.)"},
	{.status = 8405, .dos_err = 317, .posix_err = 0, .msg = R"(The distinguished name attribute could not be read.)"},
	{.status = 8406, .dos_err = 317, .posix_err = 0, .msg = R"(No superior reference has been configured for the directory service. The directory service is therefore unable to issue referrals to objects outside this forest.)"},
	{.status = 8407, .dos_err = 317, .posix_err = 0, .msg = R"(The instance type attribute could not be retrieved.)"},
	{.status = 8408, .dos_err = 317, .posix_err = 0, .msg = R"(An internal error has occurred.)"},
	{.status = 8409, .dos_err = 317, .posix_err = 0, .msg = R"(A database error has occurred.)"},
	{.status = 8410, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute GOVERNSID is missing.)"},
	{.status = 8411, .dos_err = 317, .posix_err = 0, .msg = R"(An expected attribute is missing.)"},
	{.status = 8412, .dos_err = 317, .posix_err = 0, .msg = R"(The specified naming context is missing a cross reference.)"},
	{.status = 8413, .dos_err = 317, .posix_err = 0, .msg = R"(A security checking error has occurred.)"},
	{.status = 8414, .dos_err = 317, .posix_err = 0, .msg = R"(The schema is not loaded.)"},
	{.status = 8415, .dos_err = 317, .posix_err = 0, .msg = R"(Schema allocation failed. Please check if the machine is running low on memory.)"},
	{.status = 8416, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to obtain the required syntax for the attribute schema.)"},
	{.status = 8417, .dos_err = 317, .posix_err = 0, .msg = R"(The global catalog verification failed. The global catalog is not available or does not support the operation. Some part of the directory is currently not available.)"},
	{.status = 8418, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed because of a schema mismatch between the servers involved.)"},
	{.status = 8419, .dos_err = 317, .posix_err = 0, .msg = R"(The DSA object could not be found.)"},
	{.status = 8420, .dos_err = 317, .posix_err = 0, .msg = R"(The naming context could not be found.)"},
	{.status = 8421, .dos_err = 317, .posix_err = 0, .msg = R"(The naming context could not be found in the cache.)"},
	{.status = 8422, .dos_err = 317, .posix_err = 0, .msg = R"(The child object could not be retrieved.)"},
	{.status = 8423, .dos_err = 317, .posix_err = 0, .msg = R"(The modification was not permitted for security reasons.)"},
	{.status = 8424, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot replace the hidden record.)"},
	{.status = 8425, .dos_err = 317, .posix_err = 0, .msg = R"(The hierarchy file is invalid.)"},
	{.status = 8426, .dos_err = 317, .posix_err = 0, .msg = R"(The attempt to build the hierarchy table failed.)"},
	{.status = 8427, .dos_err = 317, .posix_err = 0, .msg = R"(The directory configuration parameter is missing from the registry.)"},
	{.status = 8428, .dos_err = 317, .posix_err = 0, .msg = R"(The attempt to count the address book indices failed.)"},
	{.status = 8429, .dos_err = 317, .posix_err = 0, .msg = R"(The allocation of the hierarchy table failed.)"},
	{.status = 8430, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service encountered an internal failure.)"},
	{.status = 8431, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service encountered an unknown failure.)"},
	{.status = 8432, .dos_err = 317, .posix_err = 0, .msg = R"(A root object requires a class of 'top'.)"},
	{.status = 8433, .dos_err = 317, .posix_err = 0, .msg = R"(This directory server is shutting down, and cannot take ownership of new floating single-master operation roles.)"},
	{.status = 8434, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service is missing mandatory configuration information, and is unable to determine the ownership of floating single-master operation roles.)"},
	{.status = 8435, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service was unable to transfer ownership of one or more floating single-master operation roles to other servers.)"},
	{.status = 8436, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed.)"},
	{.status = 8437, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid parameter was specified for this replication operation.)"},
	{.status = 8438, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service is too busy to complete the replication operation at this time.)"},
	{.status = 8439, .dos_err = 317, .posix_err = 0, .msg = R"(The distinguished name specified for this replication operation is invalid.)"},
	{.status = 8440, .dos_err = 317, .posix_err = 0, .msg = R"(The naming context specified for this replication operation is invalid.)"},
	{.status = 8441, .dos_err = 317, .posix_err = 0, .msg = R"(The distinguished name specified for this replication operation already exists.)"},
	{.status = 8442, .dos_err = 317, .posix_err = 0, .msg = R"(The replication system encountered an internal error.)"},
	{.status = 8443, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation encountered a database inconsistency.)"},
	{.status = 8444, .dos_err = 317, .posix_err = 0, .msg = R"(The server specified for this replication operation could not be contacted.)"},
	{.status = 8445, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation encountered an object with an invalid instance type.)"},
	{.status = 8446, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed to allocate memory.)"},
	{.status = 8447, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation encountered an error with the mail system.)"},
	{.status = 8448, .dos_err = 317, .posix_err = 0, .msg = R"(The replication reference information for the target server already exists.)"},
	{.status = 8449, .dos_err = 317, .posix_err = 0, .msg = R"(The replication reference information for the target server does not exist.)"},
	{.status = 8450, .dos_err = 317, .posix_err = 0, .msg = R"(The naming context cannot be removed because it is replicated to another server.)"},
	{.status = 8451, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation encountered a database error.)"},
	{.status = 8452, .dos_err = 317, .posix_err = 0, .msg = R"(The naming context is in the process of being removed or is not replicated from the specified server.)"},
	{.status = 8453, .dos_err = 317, .posix_err = 0, .msg = R"(Replication access was denied.)"},
	{.status = 8454, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is not supported by this version of the directory service.)"},
	{.status = 8455, .dos_err = 317, .posix_err = 0, .msg = R"(The replication remote procedure call was cancelled.)"},
	{.status = 8456, .dos_err = 317, .posix_err = 0, .msg = R"(The source server is currently rejecting replication requests.)"},
	{.status = 8457, .dos_err = 317, .posix_err = 0, .msg = R"(The destination server is currently rejecting replication requests.)"},
	{.status = 8458, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed due to a collision of object names.)"},
	{.status = 8459, .dos_err = 317, .posix_err = 0, .msg = R"(The replication source has been reinstalled.)"},
	{.status = 8460, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed because a required parent object is missing.)"},
	{.status = 8461, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation was preempted.)"},
	{.status = 8462, .dos_err = 317, .posix_err = 0, .msg = R"(The replication synchronization attempt was abandoned because of a lack of updates.)"},
	{.status = 8463, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation was terminated because the system is shutting down.)"},
	{.status = 8464, .dos_err = 317, .posix_err = 0, .msg = R"(Synchronization attempt failed because the destination DC is currently waiting to synchronize new partial attributes from source. This condition is normal if a recent schema change modified the partial attribute set. The destination partial attribute set is not a subset of source partial attribute set.)"},
	{.status = 8465, .dos_err = 317, .posix_err = 0, .msg = R"(The replication synchronization attempt failed because a master replica attempted to sync from a partial replica.)"},
	{.status = 8466, .dos_err = 317, .posix_err = 0, .msg = R"(The server specified for this replication operation was contacted, but that server was unable to contact an additional server needed to complete the operation.)"},
	{.status = 8467, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the directory service schema of the source forest is not compatible with the version of directory service on this computer.)"},
	{.status = 8468, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: An attribute with the same link identifier already exists.)"},
	{.status = 8469, .dos_err = 317, .posix_err = 0, .msg = R"(Name translation: Generic processing error.)"},
	{.status = 8470, .dos_err = 317, .posix_err = 0, .msg = R"(Name translation: Could not find the name or insufficient right to see name.)"},
	{.status = 8471, .dos_err = 317, .posix_err = 0, .msg = R"(Name translation: Input name mapped to more than one output name.)"},
	{.status = 8472, .dos_err = 317, .posix_err = 0, .msg = R"(Name translation: Input name found, but not the associated output format.)"},
	{.status = 8473, .dos_err = 317, .posix_err = 0, .msg = R"(Name translation: Unable to resolve completely, only the domain was found.)"},
	{.status = 8474, .dos_err = 317, .posix_err = 0, .msg = R"(Name translation: Unable to perform purely syntactical mapping at the client without going out to the wire.)"},
	{.status = 8475, .dos_err = 317, .posix_err = 0, .msg = R"(Modification of a constructed attribute is not allowed.)"},
	{.status = 8476, .dos_err = 317, .posix_err = 0, .msg = R"(The OM-Object-Class specified is incorrect for an attribute with the specified syntax.)"},
	{.status = 8477, .dos_err = 317, .posix_err = 0, .msg = R"(The replication request has been posted; waiting for reply.)"},
	{.status = 8478, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation requires a directory service, and none was available.)"},
	{.status = 8479, .dos_err = 317, .posix_err = 0, .msg = R"(The LDAP display name of the class or attribute contains non-ASCII characters.)"},
	{.status = 8480, .dos_err = 317, .posix_err = 0, .msg = R"(The requested search operation is only supported for base searches.)"},
	{.status = 8481, .dos_err = 317, .posix_err = 0, .msg = R"(The search failed to retrieve attributes from the database.)"},
	{.status = 8482, .dos_err = 317, .posix_err = 0, .msg = R"(The schema update operation tried to add a backward link attribute that has no corresponding forward link.)"},
	{.status = 8483, .dos_err = 317, .posix_err = 0, .msg = R"(Source and destination of a cross-domain move do not agree on the object's epoch number. Either source or destination does not have the latest version of the object.)"},
	{.status = 8484, .dos_err = 317, .posix_err = 0, .msg = R"(Source and destination of a cross-domain move do not agree on the object's current name. Either source or destination does not have the latest version of the object.)"},
	{.status = 8485, .dos_err = 317, .posix_err = 0, .msg = R"(Source and destination for the cross-domain move operation are identical. Caller should use local move operation instead of cross-domain move operation.)"},
	{.status = 8486, .dos_err = 317, .posix_err = 0, .msg = R"(Source and destination for a cross-domain move are not in agreement on the naming contexts in the forest. Either source or destination does not have the latest version of the Partitions container.)"},
	{.status = 8487, .dos_err = 317, .posix_err = 0, .msg = R"(Destination of a cross-domain move is not authoritative for the destination naming context.)"},
	{.status = 8488, .dos_err = 317, .posix_err = 0, .msg = R"(Source and destination of a cross-domain move do not agree on the identity of the source object. Either source or destination does not have the latest version of the source object.)"},
	{.status = 8489, .dos_err = 317, .posix_err = 0, .msg = R"(Object being moved across-domains is already known to be deleted by the destination server. The source server does not have the latest version of the source object.)"},
	{.status = 8490, .dos_err = 317, .posix_err = 0, .msg = R"(Another operation which requires exclusive access to the PDC FSMO is already in progress.)"},
	{.status = 8491, .dos_err = 317, .posix_err = 0, .msg = R"(A cross-domain move operation failed such that two versions of the moved object exist - one each in the source and destination domains. The destination object needs to be removed to restore the system to a consistent state.)"},
	{.status = 8492, .dos_err = 317, .posix_err = 0, .msg = R"(This object may not be moved across domain boundaries either because cross-domain moves for this class are disallowed, or the object has some special characteristics, e.g.: trust account or restricted RID, which prevent its move.)"},
	{.status = 8493, .dos_err = 317, .posix_err = 0, .msg = R"(Can't move objects with memberships across domain boundaries as once moved, this would violate the membership conditions of the account group. Remove the object from any account group memberships and retry.)"},
	{.status = 8494, .dos_err = 317, .posix_err = 0, .msg = R"(A naming context head must be the immediate child of another naming context head, not of an interior node.)"},
	{.status = 8495, .dos_err = 317, .posix_err = 0, .msg = R"(The directory cannot validate the proposed naming context name because it does not hold a replica of the naming context above the proposed naming context. Please ensure that the domain naming master role is held by a server that is configured as a global catalog server, and that the server is up to date with its replication partners. (Applies only to Windows 2000 Domain Naming masters))"},
	{.status = 8496, .dos_err = 317, .posix_err = 0, .msg = R"(Destination domain must be in native mode.)"},
	{.status = 8497, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed because the server does not have an infrastructure container in the domain of interest.)"},
	{.status = 8498, .dos_err = 317, .posix_err = 0, .msg = R"(Cross-domain move of non-empty account groups is not allowed.)"},
	{.status = 8499, .dos_err = 317, .posix_err = 0, .msg = R"(Cross-domain move of non-empty resource groups is not allowed.)"},
	{.status = 8500, .dos_err = 317, .posix_err = 0, .msg = R"(The search flags for the attribute are invalid. The ANR bit is valid only on attributes of Unicode or Teletex strings.)"},
	{.status = 8501, .dos_err = 317, .posix_err = 0, .msg = R"(Tree deletions starting at an object which has an NC head as a descendant are not allowed.)"},
	{.status = 8502, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service failed to lock a tree in preparation for a tree deletion because the tree was in use.)"},
	{.status = 8503, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service failed to identify the list of objects to delete while attempting a tree deletion.)"},
	{.status = 8504, .dos_err = 317, .posix_err = 0, .msg = R"(Security Accounts Manager initialization failed because of the following error: %1.

Error Status: 0x%2. Please shutdown this system and reboot into Directory Services Restore Mode, check the event log for more detailed information.)"},
	{.status = 8505, .dos_err = 317, .posix_err = 0, .msg = R"(Only an administrator can modify the membership list of an administrative group.)"},
	{.status = 8506, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot change the primary group ID of a domain controller account.)"},
	{.status = 8507, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt is made to modify the base schema.)"},
	{.status = 8508, .dos_err = 317, .posix_err = 0, .msg = R"(Adding a new mandatory attribute to an existing class, deleting a mandatory attribute from an existing class, or adding an optional attribute to the special class Top that is not a backlink attribute (directly or through inheritance, for example, by adding or deleting an auxiliary class) is not allowed.)"},
	{.status = 8509, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update is not allowed on this DC because the DC is not the schema FSMO Role Owner.)"},
	{.status = 8510, .dos_err = 317, .posix_err = 0, .msg = R"(An object of this class cannot be created under the schema container. You can only create attribute-schema and class-schema objects under the schema container.)"},
	{.status = 8511, .dos_err = 317, .posix_err = 0, .msg = R"(The replica/child install failed to get the objectVersion attribute on the schema container on the source DC. Either the attribute is missing on the schema container or the credentials supplied do not have permission to read it.)"},
	{.status = 8512, .dos_err = 317, .posix_err = 0, .msg = R"(The replica/child install failed to read the objectVersion attribute in the SCHEMA section of the file schema.ini in the system32 directory.)"},
	{.status = 8513, .dos_err = 317, .posix_err = 0, .msg = R"(The specified group type is invalid.)"},
	{.status = 8514, .dos_err = 317, .posix_err = 0, .msg = R"(You cannot nest global groups in a mixed domain if the group is security-enabled.)"},
	{.status = 8515, .dos_err = 317, .posix_err = 0, .msg = R"(You cannot nest local groups in a mixed domain if the group is security-enabled.)"},
	{.status = 8516, .dos_err = 317, .posix_err = 0, .msg = R"(A global group cannot have a local group as a member.)"},
	{.status = 8517, .dos_err = 317, .posix_err = 0, .msg = R"(A global group cannot have a universal group as a member.)"},
	{.status = 8518, .dos_err = 317, .posix_err = 0, .msg = R"(A universal group cannot have a local group as a member.)"},
	{.status = 8519, .dos_err = 317, .posix_err = 0, .msg = R"(A global group cannot have a cross-domain member.)"},
	{.status = 8520, .dos_err = 317, .posix_err = 0, .msg = R"(A local group cannot have another cross domain local group as a member.)"},
	{.status = 8521, .dos_err = 317, .posix_err = 0, .msg = R"(A group with primary members cannot change to a security-disabled group.)"},
	{.status = 8522, .dos_err = 317, .posix_err = 0, .msg = R"(The schema cache load failed to convert the string default SD on a class-schema object.)"},
	{.status = 8523, .dos_err = 317, .posix_err = 0, .msg = R"(Only DSAs configured to be Global Catalog servers should be allowed to hold the Domain Naming Master FSMO role. (Applies only to Windows 2000 servers))"},
	{.status = 8524, .dos_err = 317, .posix_err = 0, .msg = R"(The DSA operation is unable to proceed because of a DNS lookup failure.)"},
	{.status = 8525, .dos_err = 317, .posix_err = 0, .msg = R"(While processing a change to the DNS Host Name for an object, the Service Principal Name values could not be kept in sync.)"},
	{.status = 8526, .dos_err = 317, .posix_err = 0, .msg = R"(The Security Descriptor attribute could not be read.)"},
	{.status = 8527, .dos_err = 317, .posix_err = 0, .msg = R"(The object requested was not found, but an object with that key was found.)"},
	{.status = 8528, .dos_err = 317, .posix_err = 0, .msg = R"(The syntax of the linked attribute being added is incorrect. Forward links can only have syntax 2.5.5.1, 2.5.5.7, and 2.5.5.14, and backlinks can only have syntax 2.5.5.1)"},
	{.status = 8529, .dos_err = 317, .posix_err = 0, .msg = R"(Security Account Manager needs to get the boot password.)"},
	{.status = 8530, .dos_err = 317, .posix_err = 0, .msg = R"(Security Account Manager needs to get the boot key from floppy disk.)"},
	{.status = 8531, .dos_err = 317, .posix_err = 0, .msg = R"(Directory Service cannot start.)"},
	{.status = 8532, .dos_err = 317, .posix_err = 0, .msg = R"(Directory Services could not start.)"},
	{.status = 8533, .dos_err = 317, .posix_err = 0, .msg = R"(The connection between client and server requires packet privacy or better.)"},
	{.status = 8534, .dos_err = 317, .posix_err = 0, .msg = R"(The source domain may not be in the same forest as destination.)"},
	{.status = 8535, .dos_err = 317, .posix_err = 0, .msg = R"(The destination domain must be in the forest.)"},
	{.status = 8536, .dos_err = 317, .posix_err = 0, .msg = R"(The operation requires that destination domain auditing be enabled.)"},
	{.status = 8537, .dos_err = 317, .posix_err = 0, .msg = R"(The operation couldn't locate a DC for the source domain.)"},
	{.status = 8538, .dos_err = 317, .posix_err = 0, .msg = R"(The source object must be a group or user.)"},
	{.status = 8539, .dos_err = 317, .posix_err = 0, .msg = R"(The source object's SID already exists in destination forest.)"},
	{.status = 8540, .dos_err = 317, .posix_err = 0, .msg = R"(The source and destination object must be of the same type.)"},
	{.status = 8541, .dos_err = 317, .posix_err = 0, .msg = R"(Security Accounts Manager initialization failed because of the following error: %1.

Error Status: 0x%2. Click OK to shut down the system and reboot into Safe Mode. Check the event log for detailed information.)"},
	{.status = 8542, .dos_err = 317, .posix_err = 0, .msg = R"(Schema information could not be included in the replication request.)"},
	{.status = 8543, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation could not be completed due to a schema incompatibility.)"},
	{.status = 8544, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation could not be completed due to a previous schema incompatibility.)"},
	{.status = 8545, .dos_err = 317, .posix_err = 0, .msg = R"(The replication update could not be applied because either the source or the destination has not yet received information regarding a recent cross-domain move operation.)"},
	{.status = 8546, .dos_err = 317, .posix_err = 0, .msg = R"(The requested domain could not be deleted because there exist domain controllers that still host this domain.)"},
	{.status = 8547, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation can be performed only on a global catalog server.)"},
	{.status = 8548, .dos_err = 317, .posix_err = 0, .msg = R"(A local group can only be a member of other local groups in the same domain.)"},
	{.status = 8549, .dos_err = 317, .posix_err = 0, .msg = R"(Foreign security principals cannot be members of universal groups.)"},
	{.status = 8550, .dos_err = 317, .posix_err = 0, .msg = R"(The attribute is not allowed to be replicated to the GC because of security reasons.)"},
	{.status = 8551, .dos_err = 317, .posix_err = 0, .msg = R"(The checkpoint with the PDC could not be taken because there too many modifications being processed currently.)"},
	{.status = 8552, .dos_err = 317, .posix_err = 0, .msg = R"(The operation requires that source domain auditing be enabled.)"},
	{.status = 8553, .dos_err = 317, .posix_err = 0, .msg = R"(Security principal objects can only be created inside domain naming contexts.)"},
	{.status = 8554, .dos_err = 317, .posix_err = 0, .msg = R"(A Service Principal Name (SPN) could not be constructed because the provided hostname is not in the necessary format.)"},
	{.status = 8555, .dos_err = 317, .posix_err = 0, .msg = R"(A Filter was passed that uses constructed attributes.)"},
	{.status = 8556, .dos_err = 317, .posix_err = 0, .msg = R"(The unicodePwd attribute value must be enclosed in double quotes.)"},
	{.status = 8557, .dos_err = 317, .posix_err = 0, .msg = R"(Your computer could not be joined to the domain. You have exceeded the maximum number of computer accounts you are allowed to create in this domain. Contact your system administrator to have this limit reset or increased.)"},
	{.status = 8558, .dos_err = 317, .posix_err = 0, .msg = R"(For security reasons, the operation must be run on the destination DC.)"},
	{.status = 8559, .dos_err = 317, .posix_err = 0, .msg = R"(For security reasons, the source DC must be NT4SP4 or greater.)"},
	{.status = 8560, .dos_err = 317, .posix_err = 0, .msg = R"(Critical Directory Service System objects cannot be deleted during tree delete operations. The tree delete may have been partially performed.)"},
	{.status = 8561, .dos_err = 317, .posix_err = 0, .msg = R"(Directory Services could not start because of the following error: %1.

Error Status: 0x%2. Please click OK to shutdown the system. You can use the recovery console to diagnose the system further.)"},
	{.status = 8562, .dos_err = 317, .posix_err = 0, .msg = R"(Security Accounts Manager initialization failed because of the following error: %1.

Error Status: 0x%2. Please click OK to shutdown the system. You can use the recovery console to diagnose the system further.)"},
	{.status = 8563, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the operating system is incompatible with the current AD DS forest functional level or AD LDS Configuration Set functional level. You must upgrade to a new version of the operating system before this server can become an AD DS Domain Controller or add an AD LDS Instance in this AD DS Forest or AD LDS Configuration Set.)"},
	{.status = 8564, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the operating system installed is incompatible with the current domain functional level. You must upgrade to a new version of the operating system before this server can become a domain controller in this domain.)"},
	{.status = 8565, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the operating system installed on this server no longer supports the current AD DS Forest functional level or AD LDS Configuration Set functional level. You must raise the AD DS Forest functional level or AD LDS Configuration Set functional level before this server can become an AD DS Domain Controller or an AD LDS Instance in this Forest or Configuration Set.)"},
	{.status = 8566, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the operating system installed on this server no longer supports the current domain functional level. You must raise the domain functional level before this server can become a domain controller in this domain.)"},
	{.status = 8567, .dos_err = 317, .posix_err = 0, .msg = R"(The version of the operating system installed on this server is incompatible with the functional level of the domain or forest.)"},
	{.status = 8568, .dos_err = 317, .posix_err = 0, .msg = R"(The functional level of the domain (or forest) cannot be raised to the requested value, because there exist one or more domain controllers in the domain (or forest) that are at a lower incompatible functional level.)"},
	{.status = 8569, .dos_err = 317, .posix_err = 0, .msg = R"(The forest functional level cannot be raised to the requested value since one or more domains are still in mixed domain mode. All domains in the forest must be in native mode, for you to raise the forest functional level.)"},
	{.status = 8570, .dos_err = 317, .posix_err = 0, .msg = R"(The sort order requested is not supported.)"},
	{.status = 8571, .dos_err = 317, .posix_err = 0, .msg = R"(The requested name already exists as a unique identifier.)"},
	{.status = 8572, .dos_err = 317, .posix_err = 0, .msg = R"(The machine account was created pre-NT4. The account needs to be recreated.)"},
	{.status = 8573, .dos_err = 317, .posix_err = 0, .msg = R"(The database is out of version store.)"},
	{.status = 8574, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to continue operation because multiple conflicting controls were used.)"},
	{.status = 8575, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to find a valid security descriptor reference domain for this partition.)"},
	{.status = 8576, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: The link identifier is reserved.)"},
	{.status = 8577, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: There are no link identifiers available.)"},
	{.status = 8578, .dos_err = 317, .posix_err = 0, .msg = R"(An account group cannot have a universal group as a member.)"},
	{.status = 8579, .dos_err = 317, .posix_err = 0, .msg = R"(Rename or move operations on naming context heads or read-only objects are not allowed.)"},
	{.status = 8580, .dos_err = 317, .posix_err = 0, .msg = R"(Move operations on objects in the schema naming context are not allowed.)"},
	{.status = 8581, .dos_err = 317, .posix_err = 0, .msg = R"(A system flag has been set on the object and does not allow the object to be moved or renamed.)"},
	{.status = 8582, .dos_err = 317, .posix_err = 0, .msg = R"(This object is not allowed to change its grandparent container. Moves are not forbidden on this object, but are restricted to sibling containers.)"},
	{.status = 8583, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to resolve completely, a referral to another forest is generated.)"},
	{.status = 8584, .dos_err = 317, .posix_err = 0, .msg = R"(The requested action is not supported on standard server.)"},
	{.status = 8585, .dos_err = 317, .posix_err = 0, .msg = R"(Could not access a partition of the directory service located on a remote server. Make sure at least one server is running for the partition in question.)"},
	{.status = 8586, .dos_err = 317, .posix_err = 0, .msg = R"(The directory cannot validate the proposed naming context (or partition) name because it does not hold a replica nor can it contact a replica of the naming context above the proposed naming context. Please ensure that the parent naming context is properly registered in DNS, and at least one replica of this naming context is reachable by the Domain Naming master.)"},
	{.status = 8587, .dos_err = 317, .posix_err = 0, .msg = R"(The thread limit for this request was exceeded.)"},
	{.status = 8588, .dos_err = 317, .posix_err = 0, .msg = R"(The Global catalog server is not in the closest site.)"},
	{.status = 8589, .dos_err = 317, .posix_err = 0, .msg = R"(The DS cannot derive a service principal name (SPN) with which to mutually authenticate the target server because the corresponding server object in the local DS database has no serverReference attribute.)"},
	{.status = 8590, .dos_err = 317, .posix_err = 0, .msg = R"(The Directory Service failed to enter single user mode.)"},
	{.status = 8591, .dos_err = 317, .posix_err = 0, .msg = R"(The Directory Service cannot parse the script because of a syntax error.)"},
	{.status = 8592, .dos_err = 317, .posix_err = 0, .msg = R"(The Directory Service cannot process the script because of an error.)"},
	{.status = 8593, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service cannot perform the requested operation because the servers involved are of different replication epochs (which is usually related to a domain rename that is in progress).)"},
	{.status = 8594, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service binding must be renegotiated due to a change in the server extensions information.)"},
	{.status = 8595, .dos_err = 317, .posix_err = 0, .msg = R"(Operation not allowed on a disabled cross ref.)"},
	{.status = 8596, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: No values for msDS-IntId are available.)"},
	{.status = 8597, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: Duplicate msDS-INtId. Retry the operation.)"},
	{.status = 8598, .dos_err = 317, .posix_err = 0, .msg = R"(Schema deletion failed: attribute is used in rDNAttID.)"},
	{.status = 8599, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service failed to authorize the request.)"},
	{.status = 8600, .dos_err = 317, .posix_err = 0, .msg = R"(The Directory Service cannot process the script because it is invalid.)"},
	{.status = 8601, .dos_err = 317, .posix_err = 0, .msg = R"(The remote create cross reference operation failed on the Domain Naming Master FSMO. The operation's error is in the extended data.)"},
	{.status = 8602, .dos_err = 317, .posix_err = 0, .msg = R"(A cross reference is in use locally with the same name.)"},
	{.status = 8603, .dos_err = 317, .posix_err = 0, .msg = R"(The DS cannot derive a service principal name (SPN) with which to mutually authenticate the target server because the server's domain has been deleted from the forest.)"},
	{.status = 8604, .dos_err = 317, .posix_err = 0, .msg = R"(Writeable NCs prevent this DC from demoting.)"},
	{.status = 8605, .dos_err = 317, .posix_err = 0, .msg = R"(The requested object has a non-unique identifier and cannot be retrieved.)"},
	{.status = 8606, .dos_err = 317, .posix_err = 0, .msg = R"(Insufficient attributes were given to create an object. This object may not exist because it may have been deleted and already garbage collected.)"},
	{.status = 8607, .dos_err = 317, .posix_err = 0, .msg = R"(The group cannot be converted due to attribute restrictions on the requested group type.)"},
	{.status = 8608, .dos_err = 317, .posix_err = 0, .msg = R"(Cross-domain move of non-empty basic application groups is not allowed.)"},
	{.status = 8609, .dos_err = 317, .posix_err = 0, .msg = R"(Cross-domain move of non-empty query based application groups is not allowed.)"},
	{.status = 8610, .dos_err = 317, .posix_err = 0, .msg = R"(The FSMO role ownership could not be verified because its directory partition has not replicated successfully with at least one replication partner.)"},
	{.status = 8611, .dos_err = 317, .posix_err = 0, .msg = R"(The target container for a redirection of a well known object container cannot already be a special container.)"},
	{.status = 8612, .dos_err = 317, .posix_err = 0, .msg = R"(The Directory Service cannot perform the requested operation because a domain rename operation is in progress.)"},
	{.status = 8613, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service detected a child partition below the requested partition name. The partition hierarchy must be created in a top down method.)"},
	{.status = 8614, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service cannot replicate with this server because the time since the last replication with this server has exceeded the tombstone lifetime.)"},
	{.status = 8615, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is not allowed on an object under the system container.)"},
	{.status = 8616, .dos_err = 317, .posix_err = 0, .msg = R"(The LDAP servers network send queue has filled up because the client is not processing the results of its requests fast enough. No more requests will be processed until the client catches up. If the client does not catch up then it will be disconnected.)"},
	{.status = 8617, .dos_err = 317, .posix_err = 0, .msg = R"(The scheduled replication did not take place because the system was too busy to execute the request within the schedule window. The replication queue is overloaded. Consider reducing the number of partners or decreasing the scheduled replication frequency.)"},
	{.status = 8618, .dos_err = 317, .posix_err = 0, .msg = R"(At this time, it cannot be determined if the branch replication policy is available on the hub domain controller. Please retry at a later time to account for replication latencies.)"},
	{.status = 8619, .dos_err = 317, .posix_err = 0, .msg = R"(The site settings object for the specified site does not exist.)"},
	{.status = 8620, .dos_err = 317, .posix_err = 0, .msg = R"(The local account store does not contain secret material for the specified account.)"},
	{.status = 8621, .dos_err = 317, .posix_err = 0, .msg = R"(Could not find a writable domain controller in the domain.)"},
	{.status = 8622, .dos_err = 317, .posix_err = 0, .msg = R"(The server object for the domain controller does not exist.)"},
	{.status = 8623, .dos_err = 317, .posix_err = 0, .msg = R"(The NTDS Settings object for the domain controller does not exist.)"},
	{.status = 8624, .dos_err = 317, .posix_err = 0, .msg = R"(The requested search operation is not supported for ASQ searches.)"},
	{.status = 8625, .dos_err = 317, .posix_err = 0, .msg = R"(A required audit event could not be generated for the operation.)"},
	{.status = 8626, .dos_err = 317, .posix_err = 0, .msg = R"(The search flags for the attribute are invalid. The subtree index bit is valid only on single valued attributes.)"},
	{.status = 8627, .dos_err = 317, .posix_err = 0, .msg = R"(The search flags for the attribute are invalid. The tuple index bit is valid only on attributes of Unicode strings.)"},
	{.status = 8628, .dos_err = 317, .posix_err = 0, .msg = R"(The address books are nested too deeply. Failed to build the hierarchy table.)"},
	{.status = 8629, .dos_err = 317, .posix_err = 0, .msg = R"(The specified up-to-date-ness vector is corrupt.)"},
	{.status = 8630, .dos_err = 317, .posix_err = 0, .msg = R"(The request to replicate secrets is denied.)"},
	{.status = 8631, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: The MAPI identifier is reserved.)"},
	{.status = 8632, .dos_err = 317, .posix_err = 0, .msg = R"(Schema update failed: There are no MAPI identifiers available.)"},
	{.status = 8633, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed because the required attributes of the local krbtgt object are missing.)"},
	{.status = 8634, .dos_err = 317, .posix_err = 0, .msg = R"(The domain name of the trusted domain already exists in the forest.)"},
	{.status = 8635, .dos_err = 317, .posix_err = 0, .msg = R"(The flat name of the trusted domain already exists in the forest.)"},
	{.status = 8636, .dos_err = 317, .posix_err = 0, .msg = R"(The User Principal Name (UPN) is invalid.)"},
	{.status = 8637, .dos_err = 317, .posix_err = 0, .msg = R"(OID mapped groups cannot have members.)"},
	{.status = 8638, .dos_err = 317, .posix_err = 0, .msg = R"(The specified OID cannot be found.)"},
	{.status = 8639, .dos_err = 317, .posix_err = 0, .msg = R"(The replication operation failed because the target object referred by a link value is recycled.)"},
	{.status = 8640, .dos_err = 317, .posix_err = 0, .msg = R"(The redirect operation failed because the target object is in a NC different from the domain NC of the current domain controller.)"},
	{.status = 8641, .dos_err = 317, .posix_err = 0, .msg = R"(The functional level of the AD LDS configuration set cannot be lowered to the requested value.)"},
	{.status = 8642, .dos_err = 317, .posix_err = 0, .msg = R"(The functional level of the domain (or forest) cannot be lowered to the requested value.)"},
	{.status = 8643, .dos_err = 317, .posix_err = 0, .msg = R"(The functional level of the AD LDS configuration set cannot be raised to the requested value, because there exist one or more ADLDS instances that are at a lower incompatible functional level.)"},
	{.status = 8644, .dos_err = 317, .posix_err = 0, .msg = R"(The domain join cannot be completed because the SID of the domain you attempted to join was identical to the SID of this machine. This is a symptom of an improperly cloned operating system install.  You should run sysprep on this machine in order to generate a new machine SID. Please see http://go.microsoft.com/fwlink/?LinkId=168895 for more information.)"},
	{.status = 8645, .dos_err = 317, .posix_err = 0, .msg = R"(The undelete operation failed because the Sam Account Name or Additional Sam Account Name of the object being undeleted conflicts with an existing live object.)"},
	{.status = 8646, .dos_err = 317, .posix_err = 0, .msg = R"(The system is not authoritative for the specified account and therefore cannot complete the operation. Please retry the operation using the provider associated with this account. If this is an online provider please use the provider's online site.)"},
	{.status = 8647, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because SPN value provided for addition/modification is not unique forest-wide.)"},
	{.status = 8648, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because UPN value provided for addition/modification is not unique forest-wide.)"},
	{.status = 8649, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because the addition/modification referenced an inbound forest-wide trust that is not present.)"},
	{.status = 8650, .dos_err = 317, .posix_err = 0, .msg = R"(The link value specified was not found, but a link value with that key was found.)"},
	{.status = 8652, .dos_err = 317, .posix_err = 0, .msg = R"(The add object operation failed because the caller was not authorized to add one or more attributes included in the request.)"},
	{.status = 8653, .dos_err = 317, .posix_err = 0, .msg = R"(The local account policy modification request was rejected because the policy is controlled by a regional authority.)"},
	{.status = 8654, .dos_err = 317, .posix_err = 0, .msg = R"(The account is controlled by external policy and cannot be modified.)"},
	{.status = 8655, .dos_err = 317, .posix_err = 0, .msg = R"(The Local Administrator Password Solution password update operation failed because the legacy LAPS schema needs to be added to Active Directory.)"},
	{.status = 8656, .dos_err = 317, .posix_err = 0, .msg = R"(The Local Administrator Password Solution password update operation failed because the Windows LAPS schema needs to be added to Active Directory.)"},
	{.status = 8657, .dos_err = 317, .posix_err = 0, .msg = R"(The Local Administrator Password Solution encrypted password update operation failed because Active Directory is not yet running at the minimum required domain functional level (2016).)"},
	{.status = 9001, .dos_err = 317, .posix_err = 0, .msg = R"(DNS server unable to interpret format.)"},
	{.status = 9002, .dos_err = 317, .posix_err = 0, .msg = R"(DNS server failure.)"},
	{.status = 9003, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name does not exist.)"},
	{.status = 9004, .dos_err = 317, .posix_err = 0, .msg = R"(DNS request not supported by name server.)"},
	{.status = 9005, .dos_err = 317, .posix_err = 0, .msg = R"(DNS operation refused.)"},
	{.status = 9006, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name that ought not exist, does exist.)"},
	{.status = 9007, .dos_err = 317, .posix_err = 0, .msg = R"(DNS RR set that ought not exist, does exist.)"},
	{.status = 9008, .dos_err = 317, .posix_err = 0, .msg = R"(DNS RR set that ought to exist, does not exist.)"},
	{.status = 9009, .dos_err = 317, .posix_err = 0, .msg = R"(DNS server not authoritative for zone.)"},
	{.status = 9010, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name in update or prereq is not in zone.)"},
	{.status = 9016, .dos_err = 317, .posix_err = 0, .msg = R"(DNS signature failed to verify.)"},
	{.status = 9017, .dos_err = 317, .posix_err = 0, .msg = R"(DNS bad key.)"},
	{.status = 9018, .dos_err = 317, .posix_err = 0, .msg = R"(DNS signature validity expired.)"},
	{.status = 9101, .dos_err = 317, .posix_err = 0, .msg = R"(Only the DNS server acting as the key master for the zone may perform this operation.)"},
	{.status = 9102, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed on a zone that is signed or has signing keys.)"},
	{.status = 9103, .dos_err = 317, .posix_err = 0, .msg = R"(NSEC3 is not compatible with the RSA-SHA-1 algorithm. Choose a different algorithm or use NSEC.)"},
	{.status = 9104, .dos_err = 317, .posix_err = 0, .msg = R"(The zone does not have enough signing keys. There must be at least one key signing key (KSK) and at least one zone signing key (ZSK).)"},
	{.status = 9105, .dos_err = 317, .posix_err = 0, .msg = R"(The specified algorithm is not supported.)"},
	{.status = 9106, .dos_err = 317, .posix_err = 0, .msg = R"(The specified key size is not supported.)"},
	{.status = 9107, .dos_err = 317, .posix_err = 0, .msg = R"(One or more of the signing keys for a zone are not accessible to the DNS server. Zone signing will not be operational until this error is resolved.)"},
	{.status = 9108, .dos_err = 317, .posix_err = 0, .msg = R"(The specified key storage provider does not support DPAPI++ data protection. Zone signing will not be operational until this error is resolved.)"},
	{.status = 9109, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected DPAPI++ error was encountered. Zone signing will not be operational until this error is resolved.)"},
	{.status = 9110, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected crypto error was encountered. Zone signing may not be operational until this error is resolved.)"},
	{.status = 9111, .dos_err = 317, .posix_err = 0, .msg = R"(The DNS server encountered a signing key with an unknown version. Zone signing will not be operational until this error is resolved.)"},
	{.status = 9112, .dos_err = 317, .posix_err = 0, .msg = R"(The specified key service provider cannot be opened by the DNS server.)"},
	{.status = 9113, .dos_err = 317, .posix_err = 0, .msg = R"(The DNS server cannot accept any more signing keys with the specified algorithm and KSK flag value for this zone.)"},
	{.status = 9114, .dos_err = 317, .posix_err = 0, .msg = R"(The specified rollover period is invalid.)"},
	{.status = 9115, .dos_err = 317, .posix_err = 0, .msg = R"(The specified initial rollover offset is invalid.)"},
	{.status = 9116, .dos_err = 317, .posix_err = 0, .msg = R"(The specified signing key is already in process of rolling over keys.)"},
	{.status = 9117, .dos_err = 317, .posix_err = 0, .msg = R"(The specified signing key does not have a standby key to revoke.)"},
	{.status = 9118, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed on a zone signing key (ZSK).)"},
	{.status = 9119, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed on an active signing key.)"},
	{.status = 9120, .dos_err = 317, .posix_err = 0, .msg = R"(The specified signing key is already queued for rollover.)"},
	{.status = 9121, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed on an unsigned zone.)"},
	{.status = 9122, .dos_err = 317, .posix_err = 0, .msg = R"(This operation could not be completed because the DNS server listed as the current key master for this zone is down or misconfigured. Resolve the problem on the current key master for this zone or use another DNS server to seize the key master role.)"},
	{.status = 9123, .dos_err = 317, .posix_err = 0, .msg = R"(The specified signature validity period is invalid.)"},
	{.status = 9124, .dos_err = 317, .posix_err = 0, .msg = R"(The specified NSEC3 iteration count is higher than allowed by the minimum key length used in the zone.)"},
	{.status = 9125, .dos_err = 317, .posix_err = 0, .msg = R"(This operation could not be completed because the DNS server has been configured with DNSSEC features disabled. Enable DNSSEC on the DNS server.)"},
	{.status = 9126, .dos_err = 317, .posix_err = 0, .msg = R"(This operation could not be completed because the XML stream received is empty or syntactically invalid.)"},
	{.status = 9127, .dos_err = 317, .posix_err = 0, .msg = R"(This operation completed, but no trust anchors were added because all of the trust anchors received were either invalid, unsupported, expired, or would not become valid in less than 30 days.)"},
	{.status = 9128, .dos_err = 317, .posix_err = 0, .msg = R"(The specified signing key is not waiting for parental DS update.)"},
	{.status = 9129, .dos_err = 317, .posix_err = 0, .msg = R"(Hash collision detected during NSEC3 signing. Specify a different user-provided salt, or use a randomly generated salt, and attempt to sign the zone again.)"},
	{.status = 9130, .dos_err = 317, .posix_err = 0, .msg = R"(NSEC is not compatible with the NSEC3-RSA-SHA-1 algorithm. Choose a different algorithm or use NSEC3.)"},
	{.status = 9501, .dos_err = 317, .posix_err = 0, .msg = R"(No records found for given DNS query.)"},
	{.status = 9502, .dos_err = 317, .posix_err = 0, .msg = R"(Bad DNS packet.)"},
	{.status = 9503, .dos_err = 317, .posix_err = 0, .msg = R"(No DNS packet.)"},
	{.status = 9504, .dos_err = 317, .posix_err = 0, .msg = R"(DNS error, check rcode.)"},
	{.status = 9505, .dos_err = 317, .posix_err = 0, .msg = R"(Unsecured DNS packet.)"},
	{.status = 9506, .dos_err = 317, .posix_err = 0, .msg = R"(DNS query request is pending.)"},
	{.status = 9551, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid DNS type.)"},
	{.status = 9552, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid IP address.)"},
	{.status = 9553, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid property.)"},
	{.status = 9554, .dos_err = 317, .posix_err = 0, .msg = R"(Try DNS operation again later.)"},
	{.status = 9555, .dos_err = 317, .posix_err = 0, .msg = R"(Record for given name and type is not unique.)"},
	{.status = 9556, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name does not comply with RFC specifications.)"},
	{.status = 9557, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name is a fully-qualified DNS name.)"},
	{.status = 9558, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name is dotted (multi-label).)"},
	{.status = 9559, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name is a single-part name.)"},
	{.status = 9560, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name contains an invalid character.)"},
	{.status = 9561, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name is entirely numeric.)"},
	{.status = 9562, .dos_err = 317, .posix_err = 0, .msg = R"(The operation requested is not permitted on a DNS root server.)"},
	{.status = 9563, .dos_err = 317, .posix_err = 0, .msg = R"(The record could not be created because this part of the DNS namespace has been delegated to another server.)"},
	{.status = 9564, .dos_err = 317, .posix_err = 0, .msg = R"(The DNS server could not find a set of root hints.)"},
	{.status = 9565, .dos_err = 317, .posix_err = 0, .msg = R"(The DNS server found root hints but they were not consistent across all adapters.)"},
	{.status = 9566, .dos_err = 317, .posix_err = 0, .msg = R"(The specified value is too small for this parameter.)"},
	{.status = 9567, .dos_err = 317, .posix_err = 0, .msg = R"(The specified value is too large for this parameter.)"},
	{.status = 9568, .dos_err = 317, .posix_err = 0, .msg = R"(This operation is not allowed while the DNS server is loading zones in the background. Please try again later.)"},
	{.status = 9569, .dos_err = 317, .posix_err = 0, .msg = R"(The operation requested is not permitted on against a DNS server running on a read-only DC.)"},
	{.status = 9570, .dos_err = 317, .posix_err = 0, .msg = R"(No data is allowed to exist underneath a DNAME record.)"},
	{.status = 9571, .dos_err = 317, .posix_err = 0, .msg = R"(This operation requires credentials delegation.)"},
	{.status = 9572, .dos_err = 317, .posix_err = 0, .msg = R"(Name resolution policy table has been corrupted. DNS resolution will fail until it is fixed. Contact your network administrator.)"},
	{.status = 9573, .dos_err = 317, .posix_err = 0, .msg = R"(Not allowed to remove all addresses.)"},
	{.status = 9601, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone does not exist.)"},
	{.status = 9602, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone information not available.)"},
	{.status = 9603, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid operation for DNS zone.)"},
	{.status = 9604, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid DNS zone configuration.)"},
	{.status = 9605, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone has no start of authority (SOA) record.)"},
	{.status = 9606, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone has no Name Server (NS) record.)"},
	{.status = 9607, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone is locked.)"},
	{.status = 9608, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone creation failed.)"},
	{.status = 9609, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone already exists.)"},
	{.status = 9610, .dos_err = 317, .posix_err = 0, .msg = R"(DNS automatic zone already exists.)"},
	{.status = 9611, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid DNS zone type.)"},
	{.status = 9612, .dos_err = 317, .posix_err = 0, .msg = R"(Secondary DNS zone requires master IP address.)"},
	{.status = 9613, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone not secondary.)"},
	{.status = 9614, .dos_err = 317, .posix_err = 0, .msg = R"(Need secondary IP address.)"},
	{.status = 9615, .dos_err = 317, .posix_err = 0, .msg = R"(WINS initialization failed.)"},
	{.status = 9616, .dos_err = 317, .posix_err = 0, .msg = R"(Need WINS servers.)"},
	{.status = 9617, .dos_err = 317, .posix_err = 0, .msg = R"(NBTSTAT initialization call failed.)"},
	{.status = 9618, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid delete of start of authority (SOA))"},
	{.status = 9619, .dos_err = 317, .posix_err = 0, .msg = R"(A conditional forwarding zone already exists for that name.)"},
	{.status = 9620, .dos_err = 317, .posix_err = 0, .msg = R"(This zone must be configured with one or more master DNS server IP addresses.)"},
	{.status = 9621, .dos_err = 317, .posix_err = 0, .msg = R"(The operation cannot be performed because this zone is shut down.)"},
	{.status = 9622, .dos_err = 317, .posix_err = 0, .msg = R"(This operation cannot be performed because the zone is currently being signed. Please try again later.)"},
	{.status = 9651, .dos_err = 317, .posix_err = 0, .msg = R"(Primary DNS zone requires datafile.)"},
	{.status = 9652, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid datafile name for DNS zone.)"},
	{.status = 9653, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to open datafile for DNS zone.)"},
	{.status = 9654, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to write datafile for DNS zone.)"},
	{.status = 9655, .dos_err = 317, .posix_err = 0, .msg = R"(Failure while reading datafile for DNS zone.)"},
	{.status = 9701, .dos_err = 317, .posix_err = 0, .msg = R"(DNS record does not exist.)"},
	{.status = 9702, .dos_err = 317, .posix_err = 0, .msg = R"(DNS record format error.)"},
	{.status = 9703, .dos_err = 317, .posix_err = 0, .msg = R"(Node creation failure in DNS.)"},
	{.status = 9704, .dos_err = 317, .posix_err = 0, .msg = R"(Unknown DNS record type.)"},
	{.status = 9705, .dos_err = 317, .posix_err = 0, .msg = R"(DNS record timed out.)"},
	{.status = 9706, .dos_err = 317, .posix_err = 0, .msg = R"(Name not in DNS zone.)"},
	{.status = 9707, .dos_err = 317, .posix_err = 0, .msg = R"(CNAME loop detected.)"},
	{.status = 9708, .dos_err = 317, .posix_err = 0, .msg = R"(Node is a CNAME DNS record.)"},
	{.status = 9709, .dos_err = 317, .posix_err = 0, .msg = R"(A CNAME record already exists for given name.)"},
	{.status = 9710, .dos_err = 317, .posix_err = 0, .msg = R"(Record only at DNS zone root.)"},
	{.status = 9711, .dos_err = 317, .posix_err = 0, .msg = R"(DNS record already exists.)"},
	{.status = 9712, .dos_err = 317, .posix_err = 0, .msg = R"(Secondary DNS zone data error.)"},
	{.status = 9713, .dos_err = 317, .posix_err = 0, .msg = R"(Could not create DNS cache data.)"},
	{.status = 9714, .dos_err = 317, .posix_err = 0, .msg = R"(DNS name does not exist.)"},
	{.status = 9715, .dos_err = 317, .posix_err = 0, .msg = R"(Could not create pointer (PTR) record.)"},
	{.status = 9716, .dos_err = 317, .posix_err = 0, .msg = R"(DNS domain was undeleted.)"},
	{.status = 9717, .dos_err = 317, .posix_err = 0, .msg = R"(The directory service is unavailable.)"},
	{.status = 9718, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone already exists in the directory service.)"},
	{.status = 9719, .dos_err = 317, .posix_err = 0, .msg = R"(DNS server not creating or reading the boot file for the directory service integrated DNS zone.)"},
	{.status = 9720, .dos_err = 317, .posix_err = 0, .msg = R"(Node is a DNAME DNS record.)"},
	{.status = 9721, .dos_err = 317, .posix_err = 0, .msg = R"(A DNAME record already exists for given name.)"},
	{.status = 9722, .dos_err = 317, .posix_err = 0, .msg = R"(An alias loop has been detected with either CNAME or DNAME records.)"},
	{.status = 9751, .dos_err = 317, .posix_err = 0, .msg = R"(DNS AXFR (zone transfer) complete.)"},
	{.status = 9752, .dos_err = 317, .posix_err = 0, .msg = R"(DNS zone transfer failed.)"},
	{.status = 9753, .dos_err = 317, .posix_err = 0, .msg = R"(Added local WINS server.)"},
	{.status = 9801, .dos_err = 317, .posix_err = 0, .msg = R"(Secure update call needs to continue update request.)"},
	{.status = 9851, .dos_err = 317, .posix_err = 0, .msg = R"(TCP/IP network protocol not installed.)"},
	{.status = 9852, .dos_err = 317, .posix_err = 0, .msg = R"(No DNS servers configured for local system.)"},
	{.status = 9901, .dos_err = 317, .posix_err = 0, .msg = R"(The specified directory partition does not exist.)"},
	{.status = 9902, .dos_err = 317, .posix_err = 0, .msg = R"(The specified directory partition already exists.)"},
	{.status = 9903, .dos_err = 317, .posix_err = 0, .msg = R"(This DNS server is not enlisted in the specified directory partition.)"},
	{.status = 9904, .dos_err = 317, .posix_err = 0, .msg = R"(This DNS server is already enlisted in the specified directory partition.)"},
	{.status = 9905, .dos_err = 317, .posix_err = 0, .msg = R"(The directory partition is not available at this time. Please wait a few minutes and try again.)"},
	{.status = 9906, .dos_err = 317, .posix_err = 0, .msg = R"(The operation failed because the domain naming master FSMO role could not be reached. The domain controller holding the domain naming master FSMO role is down or unable to service the request or is not running Windows Server 2003 or later.)"},
	{.status = 9911, .dos_err = 317, .posix_err = 0, .msg = R"(The RRL is not enabled.)"},
	{.status = 9912, .dos_err = 317, .posix_err = 0, .msg = R"(The window size parameter is invalid. It should be greater than or equal to 1.)"},
	{.status = 9913, .dos_err = 317, .posix_err = 0, .msg = R"(The IPv4 prefix length parameter is invalid. It should be less than or equal to 32.)"},
	{.status = 9914, .dos_err = 317, .posix_err = 0, .msg = R"(The IPv6 prefix length parameter is invalid. It should be less than or equal to 128.)"},
	{.status = 9915, .dos_err = 317, .posix_err = 0, .msg = R"(The TC Rate parameter is invalid. It should be less than 10.)"},
	{.status = 9916, .dos_err = 317, .posix_err = 0, .msg = R"(The Leak Rate parameter is invalid. It should be either 0, or between 2 and 10.)"},
	{.status = 9917, .dos_err = 317, .posix_err = 0, .msg = R"(The Leak Rate or TC Rate parameter is invalid. Leak Rate should be greater than TC Rate.)"},
	{.status = 9921, .dos_err = 317, .posix_err = 0, .msg = R"(The virtualization instance already exists.)"},
	{.status = 9922, .dos_err = 317, .posix_err = 0, .msg = R"(The virtualization instance does not exist.)"},
	{.status = 9923, .dos_err = 317, .posix_err = 0, .msg = R"(The virtualization tree is locked.)"},
	{.status = 9924, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid virtualization instance name.)"},
	{.status = 9925, .dos_err = 317, .posix_err = 0, .msg = R"(The default virtualization instance cannot be added, removed or modified.)"},
	{.status = 9951, .dos_err = 317, .posix_err = 0, .msg = R"(The scope already exists for the zone.)"},
	{.status = 9952, .dos_err = 317, .posix_err = 0, .msg = R"(The scope does not exist for the zone.)"},
	{.status = 9953, .dos_err = 317, .posix_err = 0, .msg = R"(The scope is the same as the default zone scope.)"},
	{.status = 9954, .dos_err = 317, .posix_err = 0, .msg = R"(The scope name contains invalid characters.)"},
	{.status = 9955, .dos_err = 317, .posix_err = 0, .msg = R"(Operation not allowed when the zone has scopes.)"},
	{.status = 9956, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to load zone scope.)"},
	{.status = 9957, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to write data file for DNS zone scope. Please verify the file exists and is writable.)"},
	{.status = 9958, .dos_err = 317, .posix_err = 0, .msg = R"(The scope name contains invalid characters.)"},
	{.status = 9959, .dos_err = 317, .posix_err = 0, .msg = R"(The scope does not exist.)"},
	{.status = 9960, .dos_err = 317, .posix_err = 0, .msg = R"(The scope is the same as the default scope.)"},
	{.status = 9961, .dos_err = 317, .posix_err = 0, .msg = R"(The operation is invalid on the scope.)"},
	{.status = 9962, .dos_err = 317, .posix_err = 0, .msg = R"(The scope is locked.)"},
	{.status = 9963, .dos_err = 317, .posix_err = 0, .msg = R"(The scope already exists.)"},
	{.status = 9971, .dos_err = 317, .posix_err = 0, .msg = R"(A policy with the same name already exists on this level (server level or zone level) on the DNS server.)"},
	{.status = 9972, .dos_err = 317, .posix_err = 0, .msg = R"(No policy with this name exists on this level (server level or zone level) on the DNS server.)"},
	{.status = 9973, .dos_err = 317, .posix_err = 0, .msg = R"(The criteria provided in the policy are invalid.)"},
	{.status = 9974, .dos_err = 317, .posix_err = 0, .msg = R"(At least one of the settings of this policy is invalid.)"},
	{.status = 9975, .dos_err = 317, .posix_err = 0, .msg = R"(The client subnet cannot be deleted while it is being accessed by a policy.)"},
	{.status = 9976, .dos_err = 317, .posix_err = 0, .msg = R"(The client subnet does not exist on the DNS server.)"},
	{.status = 9977, .dos_err = 317, .posix_err = 0, .msg = R"(A client subnet with this name already exists on the DNS server.)"},
	{.status = 9978, .dos_err = 317, .posix_err = 0, .msg = R"(The IP subnet specified does not exist in the client subnet.)"},
	{.status = 9979, .dos_err = 317, .posix_err = 0, .msg = R"(The IP subnet that is being added, already exists in the client subnet.)"},
	{.status = 9980, .dos_err = 317, .posix_err = 0, .msg = R"(The policy is locked.)"},
	{.status = 9981, .dos_err = 317, .posix_err = 0, .msg = R"(The weight of the scope in the policy is invalid.)"},
	{.status = 9982, .dos_err = 317, .posix_err = 0, .msg = R"(The DNS policy name is invalid.)"},
	{.status = 9983, .dos_err = 317, .posix_err = 0, .msg = R"(The policy is missing criteria.)"},
	{.status = 9984, .dos_err = 317, .posix_err = 0, .msg = R"(The name of the the client subnet record is invalid.)"},
	{.status = 9985, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid policy processing order.)"},
	{.status = 9986, .dos_err = 317, .posix_err = 0, .msg = R"(The scope information has not been provided for a policy that requires it.)"},
	{.status = 9987, .dos_err = 317, .posix_err = 0, .msg = R"(The scope information has been provided for a policy that does not require it.)"},
	{.status = 9988, .dos_err = 317, .posix_err = 0, .msg = R"(The server scope cannot be deleted because it is referenced by a DNS Policy.)"},
	{.status = 9989, .dos_err = 317, .posix_err = 0, .msg = R"(The zone scope cannot be deleted because it is referenced by a DNS Policy.)"},
	{.status = 9990, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion client subnet provided in the policy is invalid.)"},
	{.status = 9991, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion transport protocol provided in the policy is invalid.)"},
	{.status = 9992, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion network protocol provided in the policy is invalid.)"},
	{.status = 9993, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion interface provided in the policy is invalid.)"},
	{.status = 9994, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion FQDN provided in the policy is invalid.)"},
	{.status = 9995, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion query type provided in the policy is invalid.)"},
	{.status = 9996, .dos_err = 317, .posix_err = 0, .msg = R"(The criterion time of day provided in the policy is invalid.)"},
	{.status = 10004, .dos_err = 317, .posix_err = 0, .msg = R"(A blocking operation was interrupted by a call to WSACancelBlockingCall.)"},
	{.status = 10009, .dos_err = 317, .posix_err = 0, .msg = R"(The file handle supplied is not valid.)"},
	{.status = 10013, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt was made to access a socket in a way forbidden by its access permissions.)"},
	{.status = 10014, .dos_err = 317, .posix_err = 0, .msg = R"(The system detected an invalid pointer address in attempting to use a pointer argument in a call.)"},
	{.status = 10022, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid argument was supplied.)"},
	{.status = 10024, .dos_err = 317, .posix_err = 0, .msg = R"(Too many open sockets.)"},
	{.status = 10035, .dos_err = 317, .posix_err = 0, .msg = R"(A non-blocking socket operation could not be completed immediately.)"},
	{.status = 10036, .dos_err = 317, .posix_err = 0, .msg = R"(A blocking operation is currently executing.)"},
	{.status = 10037, .dos_err = 317, .posix_err = 0, .msg = R"(An operation was attempted on a non-blocking socket that already had an operation in progress.)"},
	{.status = 10038, .dos_err = 317, .posix_err = 0, .msg = R"(An operation was attempted on something that is not a socket.)"},
	{.status = 10039, .dos_err = 317, .posix_err = 0, .msg = R"(A required address was omitted from an operation on a socket.)"},
	{.status = 10040, .dos_err = 317, .posix_err = 0, .msg = R"(A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram into was smaller than the datagram itself.)"},
	{.status = 10041, .dos_err = 317, .posix_err = 0, .msg = R"(A protocol was specified in the socket function call that does not support the semantics of the socket type requested.)"},
	{.status = 10042, .dos_err = 317, .posix_err = 0, .msg = R"(An unknown, invalid, or unsupported option or level was specified in a getsockopt or setsockopt call.)"},
	{.status = 10043, .dos_err = 317, .posix_err = 0, .msg = R"(The requested protocol has not been configured into the system, or no implementation for it exists.)"},
	{.status = 10044, .dos_err = 317, .posix_err = 0, .msg = R"(The support for the specified socket type does not exist in this address family.)"},
	{.status = 10045, .dos_err = 317, .posix_err = 0, .msg = R"(The attempted operation is not supported for the type of object referenced.)"},
	{.status = 10046, .dos_err = 317, .posix_err = 0, .msg = R"(The protocol family has not been configured into the system or no implementation for it exists.)"},
	{.status = 10047, .dos_err = 317, .posix_err = 0, .msg = R"(An address incompatible with the requested protocol was used.)"},
	{.status = 10048, .dos_err = 317, .posix_err = 0, .msg = R"(Only one usage of each socket address (protocol/network address/port) is normally permitted.)"},
	{.status = 10049, .dos_err = 317, .posix_err = 0, .msg = R"(The requested address is not valid in its context.)"},
	{.status = 10050, .dos_err = 317, .posix_err = 0, .msg = R"(A socket operation encountered a dead network.)"},
	{.status = 10051, .dos_err = 317, .posix_err = 0, .msg = R"(A socket operation was attempted to an unreachable network.)"},
	{.status = 10052, .dos_err = 317, .posix_err = 0, .msg = R"(The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.)"},
	{.status = 10053, .dos_err = 317, .posix_err = 0, .msg = R"(An established connection was aborted by the software in your host machine.)"},
	{.status = 10054, .dos_err = 317, .posix_err = 0, .msg = R"(An existing connection was forcibly closed by the remote host.)"},
	{.status = 10055, .dos_err = 317, .posix_err = 0, .msg = R"(An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.)"},
	{.status = 10056, .dos_err = 317, .posix_err = 0, .msg = R"(A connect request was made on an already connected socket.)"},
	{.status = 10057, .dos_err = 317, .posix_err = 0, .msg = R"(A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.)"},
	{.status = 10058, .dos_err = 317, .posix_err = 0, .msg = R"(A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.)"},
	{.status = 10059, .dos_err = 317, .posix_err = 0, .msg = R"(Too many references to some kernel object.)"},
	{.status = 10060, .dos_err = 317, .posix_err = 0, .msg = R"(A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond.)"},
	{.status = 10061, .dos_err = 317, .posix_err = 0, .msg = R"(No connection could be made because the target machine actively refused it.)"},
	{.status = 10062, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot translate name.)"},
	{.status = 10063, .dos_err = 317, .posix_err = 0, .msg = R"(Name component or name was too long.)"},
	{.status = 10064, .dos_err = 317, .posix_err = 0, .msg = R"(A socket operation failed because the destination host was down.)"},
	{.status = 10065, .dos_err = 317, .posix_err = 0, .msg = R"(A socket operation was attempted to an unreachable host.)"},
	{.status = 10066, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot remove a directory that is not empty.)"},
	{.status = 10067, .dos_err = 317, .posix_err = 0, .msg = R"(A Windows Sockets implementation may have a limit on the number of applications that may use it simultaneously.)"},
	{.status = 10068, .dos_err = 317, .posix_err = 0, .msg = R"(Ran out of quota.)"},
	{.status = 10069, .dos_err = 317, .posix_err = 0, .msg = R"(Ran out of disk quota.)"},
	{.status = 10070, .dos_err = 317, .posix_err = 0, .msg = R"(File handle reference is no longer available.)"},
	{.status = 10071, .dos_err = 317, .posix_err = 0, .msg = R"(Item is not available locally.)"},
	{.status = 10091, .dos_err = 317, .posix_err = 0, .msg = R"(WSAStartup cannot function at this time because the underlying system it uses to provide network services is currently unavailable.)"},
	{.status = 10092, .dos_err = 317, .posix_err = 0, .msg = R"(The Windows Sockets version requested is not supported.)"},
	{.status = 10093, .dos_err = 317, .posix_err = 0, .msg = R"(Either the application has not called WSAStartup, or WSAStartup failed.)"},
	{.status = 10101, .dos_err = 317, .posix_err = 0, .msg = R"(Returned by WSARecv or WSARecvFrom to indicate the remote party has initiated a graceful shutdown sequence.)"},
	{.status = 10102, .dos_err = 317, .posix_err = 0, .msg = R"(No more results can be returned by WSALookupServiceNext.)"},
	{.status = 10103, .dos_err = 317, .posix_err = 0, .msg = R"(A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled.)"},
	{.status = 10104, .dos_err = 317, .posix_err = 0, .msg = R"(The procedure call table is invalid.)"},
	{.status = 10105, .dos_err = 317, .posix_err = 0, .msg = R"(The requested service provider is invalid.)"},
	{.status = 10106, .dos_err = 317, .posix_err = 0, .msg = R"(The requested service provider could not be loaded or initialized.)"},
	{.status = 10107, .dos_err = 317, .posix_err = 0, .msg = R"(A system call has failed.)"},
	{.status = 10108, .dos_err = 317, .posix_err = 0, .msg = R"(No such service is known. The service cannot be found in the specified name space.)"},
	{.status = 10109, .dos_err = 317, .posix_err = 0, .msg = R"(The specified class was not found.)"},
	{.status = 10110, .dos_err = 317, .posix_err = 0, .msg = R"(No more results can be returned by WSALookupServiceNext.)"},
	{.status = 10111, .dos_err = 317, .posix_err = 0, .msg = R"(A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled.)"},
	{.status = 10112, .dos_err = 317, .posix_err = 0, .msg = R"(A database query failed because it was actively refused.)"},
	{.status = 11001, .dos_err = 317, .posix_err = 0, .msg = R"(No such host is known.)"},
	{.status = 11002, .dos_err = 317, .posix_err = 0, .msg = R"(This is usually a temporary error during hostname resolution and means that the local server did not receive a response from an authoritative server.)"},
	{.status = 11003, .dos_err = 317, .posix_err = 0, .msg = R"(A non-recoverable error occurred during a database lookup.)"},
	{.status = 11004, .dos_err = 317, .posix_err = 0, .msg = R"(The requested name is valid, but no data of the requested type was found.)"},
	{.status = 11005, .dos_err = 317, .posix_err = 0, .msg = R"(At least one reserve has arrived.)"},
	{.status = 11006, .dos_err = 317, .posix_err = 0, .msg = R"(At least one path has arrived.)"},
	{.status = 11007, .dos_err = 317, .posix_err = 0, .msg = R"(There are no senders.)"},
	{.status = 11008, .dos_err = 317, .posix_err = 0, .msg = R"(There are no receivers.)"},
	{.status = 11009, .dos_err = 317, .posix_err = 0, .msg = R"(Reserve has been confirmed.)"},
	{.status = 11010, .dos_err = 317, .posix_err = 0, .msg = R"(Error due to lack of resources.)"},
	{.status = 11011, .dos_err = 317, .posix_err = 0, .msg = R"(Rejected for administrative reasons - bad credentials.)"},
	{.status = 11012, .dos_err = 317, .posix_err = 0, .msg = R"(Unknown or conflicting style.)"},
	{.status = 11013, .dos_err = 317, .posix_err = 0, .msg = R"(Problem with some part of the filterspec or providerspecific buffer in general.)"},
	{.status = 11014, .dos_err = 317, .posix_err = 0, .msg = R"(Problem with some part of the flowspec.)"},
	{.status = 11015, .dos_err = 317, .posix_err = 0, .msg = R"(General QOS error.)"},
	{.status = 11016, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid or unrecognized service type was found in the flowspec.)"},
	{.status = 11017, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid or inconsistent flowspec was found in the QOS structure.)"},
	{.status = 11018, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid QOS provider-specific buffer.)"},
	{.status = 11019, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid QOS filter style was used.)"},
	{.status = 11020, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid QOS filter type was used.)"},
	{.status = 11021, .dos_err = 317, .posix_err = 0, .msg = R"(An incorrect number of QOS FILTERSPECs were specified in the FLOWDESCRIPTOR.)"},
	{.status = 11022, .dos_err = 317, .posix_err = 0, .msg = R"(An object with an invalid ObjectLength field was specified in the QOS provider-specific buffer.)"},
	{.status = 11023, .dos_err = 317, .posix_err = 0, .msg = R"(An incorrect number of flow descriptors was specified in the QOS structure.)"},
	{.status = 11024, .dos_err = 317, .posix_err = 0, .msg = R"(An unrecognized object was found in the QOS provider-specific buffer.)"},
	{.status = 11025, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid policy object was found in the QOS provider-specific buffer.)"},
	{.status = 11026, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid QOS flow descriptor was found in the flow descriptor list.)"},
	{.status = 11027, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid or inconsistent flowspec was found in the QOS provider specific buffer.)"},
	{.status = 11028, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid FILTERSPEC was found in the QOS provider-specific buffer.)"},
	{.status = 11029, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid shape discard mode object was found in the QOS provider specific buffer.)"},
	{.status = 11030, .dos_err = 317, .posix_err = 0, .msg = R"(An invalid shaping rate object was found in the QOS provider-specific buffer.)"},
	{.status = 11031, .dos_err = 317, .posix_err = 0, .msg = R"(A reserved policy element was found in the QOS provider-specific buffer.)"},
	{.status = 11032, .dos_err = 317, .posix_err = 0, .msg = R"(No such host is known securely.)"},
	{.status = 11033, .dos_err = 317, .posix_err = 0, .msg = R"(Name based IPSEC policy could not be added.)"},
	{.status = 13000, .dos_err = 317, .posix_err = 0, .msg = R"(The specified quick mode policy already exists.)"},
	{.status = 13001, .dos_err = 317, .posix_err = 0, .msg = R"(The specified quick mode policy was not found.)"},
	{.status = 13002, .dos_err = 317, .posix_err = 0, .msg = R"(The specified quick mode policy is being used.)"},
	{.status = 13003, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode policy already exists.)"},
	{.status = 13004, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode policy was not found)"},
	{.status = 13005, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode policy is being used.)"},
	{.status = 13006, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode filter already exists.)"},
	{.status = 13007, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode filter was not found.)"},
	{.status = 13008, .dos_err = 317, .posix_err = 0, .msg = R"(The specified transport mode filter already exists.)"},
	{.status = 13009, .dos_err = 317, .posix_err = 0, .msg = R"(The specified transport mode filter does not exist.)"},
	{.status = 13010, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode authentication list exists.)"},
	{.status = 13011, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode authentication list was not found.)"},
	{.status = 13012, .dos_err = 317, .posix_err = 0, .msg = R"(The specified main mode authentication list is being used.)"},
	{.status = 13013, .dos_err = 317, .posix_err = 0, .msg = R"(The specified default main mode policy was not found.)"},
	{.status = 13014, .dos_err = 317, .posix_err = 0, .msg = R"(The specified default main mode authentication list was not found.)"},
	{.status = 13015, .dos_err = 317, .posix_err = 0, .msg = R"(The specified default quick mode policy was not found.)"},
	{.status = 13016, .dos_err = 317, .posix_err = 0, .msg = R"(The specified tunnel mode filter exists.)"},
	{.status = 13017, .dos_err = 317, .posix_err = 0, .msg = R"(The specified tunnel mode filter was not found.)"},
	{.status = 13018, .dos_err = 317, .posix_err = 0, .msg = R"(The Main Mode filter is pending deletion.)"},
	{.status = 13019, .dos_err = 317, .posix_err = 0, .msg = R"(The transport filter is pending deletion.)"},
	{.status = 13020, .dos_err = 317, .posix_err = 0, .msg = R"(The tunnel filter is pending deletion.)"},
	{.status = 13021, .dos_err = 317, .posix_err = 0, .msg = R"(The Main Mode policy is pending deletion.)"},
	{.status = 13022, .dos_err = 317, .posix_err = 0, .msg = R"(The Main Mode authentication bundle is pending deletion.)"},
	{.status = 13023, .dos_err = 317, .posix_err = 0, .msg = R"(The Quick Mode policy is pending deletion.)"},
	{.status = 13024, .dos_err = 317, .posix_err = 0, .msg = R"(The Main Mode policy was successfully added, but some of the requested offers are not supported.)"},
	{.status = 13025, .dos_err = 317, .posix_err = 0, .msg = R"(The Quick Mode policy was successfully added, but some of the requested offers are not supported.)"},
	{.status = 13800, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_IPSEC_IKE_NEG_STATUS_BEGIN)"},
	{.status = 13801, .dos_err = 317, .posix_err = 0, .msg = R"(IKE authentication credentials are unacceptable)"},
	{.status = 13802, .dos_err = 317, .posix_err = 0, .msg = R"(IKE security attributes are unacceptable)"},
	{.status = 13803, .dos_err = 317, .posix_err = 0, .msg = R"(IKE Negotiation in progress)"},
	{.status = 13804, .dos_err = 317, .posix_err = 0, .msg = R"(General processing error)"},
	{.status = 13805, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation timed out)"},
	{.status = 13806, .dos_err = 317, .posix_err = 0, .msg = R"(IKE failed to find valid machine certificate. Contact your Network Security Administrator about installing a valid certificate in the appropriate Certificate Store.)"},
	{.status = 13807, .dos_err = 317, .posix_err = 0, .msg = R"(IKE SA deleted by peer before establishment completed)"},
	{.status = 13808, .dos_err = 317, .posix_err = 0, .msg = R"(IKE SA deleted before establishment completed)"},
	{.status = 13809, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation request sat in Queue too long)"},
	{.status = 13810, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation request sat in Queue too long)"},
	{.status = 13811, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation request sat in Queue too long)"},
	{.status = 13812, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation request sat in Queue too long)"},
	{.status = 13813, .dos_err = 317, .posix_err = 0, .msg = R"(No response from peer)"},
	{.status = 13814, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation took too long)"},
	{.status = 13815, .dos_err = 317, .posix_err = 0, .msg = R"(Negotiation took too long)"},
	{.status = 13816, .dos_err = 317, .posix_err = 0, .msg = R"(Unknown error occurred)"},
	{.status = 13817, .dos_err = 317, .posix_err = 0, .msg = R"(Certificate Revocation Check failed)"},
	{.status = 13818, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid certificate key usage)"},
	{.status = 13819, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid certificate type)"},
	{.status = 13820, .dos_err = 317, .posix_err = 0, .msg = R"(IKE negotiation failed because the machine certificate used does not have a private key. IPsec certificates require a private key. Contact your Network Security administrator about replacing with a certificate that has a private key.)"},
	{.status = 13821, .dos_err = 317, .posix_err = 0, .msg = R"(Simultaneous rekeys were detected.)"},
	{.status = 13822, .dos_err = 317, .posix_err = 0, .msg = R"(Failure in Diffie-Hellman computation)"},
	{.status = 13823, .dos_err = 317, .posix_err = 0, .msg = R"(Don't know how to process critical payload)"},
	{.status = 13824, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid header)"},
	{.status = 13825, .dos_err = 317, .posix_err = 0, .msg = R"(No policy configured)"},
	{.status = 13826, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to verify signature)"},
	{.status = 13827, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to authenticate using Kerberos)"},
	{.status = 13828, .dos_err = 317, .posix_err = 0, .msg = R"(Peer's certificate did not have a public key)"},
	{.status = 13829, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing error payload)"},
	{.status = 13830, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing SA payload)"},
	{.status = 13831, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Proposal payload)"},
	{.status = 13832, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Transform payload)"},
	{.status = 13833, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing KE payload)"},
	{.status = 13834, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing ID payload)"},
	{.status = 13835, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Cert payload)"},
	{.status = 13836, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Certificate Request payload)"},
	{.status = 13837, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Hash payload)"},
	{.status = 13838, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Signature payload)"},
	{.status = 13839, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Nonce payload)"},
	{.status = 13840, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Notify payload)"},
	{.status = 13841, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing Delete Payload)"},
	{.status = 13842, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing VendorId payload)"},
	{.status = 13843, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid payload received)"},
	{.status = 13844, .dos_err = 317, .posix_err = 0, .msg = R"(Soft SA loaded)"},
	{.status = 13845, .dos_err = 317, .posix_err = 0, .msg = R"(Soft SA torn down)"},
	{.status = 13846, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid cookie received.)"},
	{.status = 13847, .dos_err = 317, .posix_err = 0, .msg = R"(Peer failed to send valid machine certificate)"},
	{.status = 13848, .dos_err = 317, .posix_err = 0, .msg = R"(Certification Revocation check of peer's certificate failed)"},
	{.status = 13849, .dos_err = 317, .posix_err = 0, .msg = R"(New policy invalidated SAs formed with old policy)"},
	{.status = 13850, .dos_err = 317, .posix_err = 0, .msg = R"(There is no available Main Mode IKE policy.)"},
	{.status = 13851, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to enabled TCB privilege.)"},
	{.status = 13852, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to load SECURITY.DLL.)"},
	{.status = 13853, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to obtain security function table dispatch address from SSPI.)"},
	{.status = 13854, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to query Kerberos package to obtain max token size.)"},
	{.status = 13855, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to obtain Kerberos server credentials for ISAKMP/ERROR_IPSEC_IKE service. Kerberos authentication will not function. The most likely reason for this is lack of domain membership. This is normal if your computer is a member of a workgroup.)"},
	{.status = 13856, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to determine SSPI principal name for ISAKMP/ERROR_IPSEC_IKE service (QueryCredentialsAttributes).)"},
	{.status = 13857, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to obtain new SPI for the inbound SA from IPsec driver. The most common cause for this is that the driver does not have the correct filter. Check your policy to verify the filters.)"},
	{.status = 13858, .dos_err = 317, .posix_err = 0, .msg = R"(Given filter is invalid)"},
	{.status = 13859, .dos_err = 317, .posix_err = 0, .msg = R"(Memory allocation failed.)"},
	{.status = 13860, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to add Security Association to IPsec Driver. The most common cause for this is if the IKE negotiation took too long to complete. If the problem persists, reduce the load on the faulting machine.)"},
	{.status = 13861, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid policy)"},
	{.status = 13862, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid DOI)"},
	{.status = 13863, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid situation)"},
	{.status = 13864, .dos_err = 317, .posix_err = 0, .msg = R"(Diffie-Hellman failure)"},
	{.status = 13865, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid Diffie-Hellman group)"},
	{.status = 13866, .dos_err = 317, .posix_err = 0, .msg = R"(Error encrypting payload)"},
	{.status = 13867, .dos_err = 317, .posix_err = 0, .msg = R"(Error decrypting payload)"},
	{.status = 13868, .dos_err = 317, .posix_err = 0, .msg = R"(Policy match error)"},
	{.status = 13869, .dos_err = 317, .posix_err = 0, .msg = R"(Unsupported ID)"},
	{.status = 13870, .dos_err = 317, .posix_err = 0, .msg = R"(Hash verification failed)"},
	{.status = 13871, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid hash algorithm)"},
	{.status = 13872, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid hash size)"},
	{.status = 13873, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid encryption algorithm)"},
	{.status = 13874, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid authentication algorithm)"},
	{.status = 13875, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid certificate signature)"},
	{.status = 13876, .dos_err = 317, .posix_err = 0, .msg = R"(Load failed)"},
	{.status = 13877, .dos_err = 317, .posix_err = 0, .msg = R"(Deleted via RPC call)"},
	{.status = 13878, .dos_err = 317, .posix_err = 0, .msg = R"(Temporary state created to perform reinitialization. This is not a real failure.)"},
	{.status = 13879, .dos_err = 317, .posix_err = 0, .msg = R"(The lifetime value received in the Responder Lifetime Notify is below the Windows 2000 configured minimum value. Please fix the policy on the peer machine.)"},
	{.status = 13880, .dos_err = 317, .posix_err = 0, .msg = R"(The recipient cannot handle version of IKE specified in the header.)"},
	{.status = 13881, .dos_err = 317, .posix_err = 0, .msg = R"(Key length in certificate is too small for configured security requirements.)"},
	{.status = 13882, .dos_err = 317, .posix_err = 0, .msg = R"(Max number of established MM SAs to peer exceeded.)"},
	{.status = 13883, .dos_err = 317, .posix_err = 0, .msg = R"(IKE received a policy that disables negotiation.)"},
	{.status = 13884, .dos_err = 317, .posix_err = 0, .msg = R"(Reached maximum quick mode limit for the main mode. New main mode will be started.)"},
	{.status = 13885, .dos_err = 317, .posix_err = 0, .msg = R"(Main mode SA lifetime expired or peer sent a main mode delete.)"},
	{.status = 13886, .dos_err = 317, .posix_err = 0, .msg = R"(Main mode SA assumed to be invalid because peer stopped responding.)"},
	{.status = 13887, .dos_err = 317, .posix_err = 0, .msg = R"(Certificate doesn't chain to a trusted root in IPsec policy.)"},
	{.status = 13888, .dos_err = 317, .posix_err = 0, .msg = R"(Received unexpected message ID.)"},
	{.status = 13889, .dos_err = 317, .posix_err = 0, .msg = R"(Received invalid authentication offers.)"},
	{.status = 13890, .dos_err = 317, .posix_err = 0, .msg = R"(Sent DoS cookie notify to initiator.)"},
	{.status = 13891, .dos_err = 317, .posix_err = 0, .msg = R"(IKE service is shutting down.)"},
	{.status = 13892, .dos_err = 317, .posix_err = 0, .msg = R"(Could not verify binding between CGA address and certificate.)"},
	{.status = 13893, .dos_err = 317, .posix_err = 0, .msg = R"(Error processing NatOA payload.)"},
	{.status = 13894, .dos_err = 317, .posix_err = 0, .msg = R"(Parameters of the main mode are invalid for this quick mode.)"},
	{.status = 13895, .dos_err = 317, .posix_err = 0, .msg = R"(Quick mode SA was expired by IPsec driver.)"},
	{.status = 13896, .dos_err = 317, .posix_err = 0, .msg = R"(Too many dynamically added IKEEXT filters were detected.)"},
	{.status = 13897, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_IPSEC_IKE_NEG_STATUS_END)"},
	{.status = 13898, .dos_err = 317, .posix_err = 0, .msg = R"(NAP reauth succeeded and must delete the dummy NAP IKEv2 tunnel.)"},
	{.status = 13899, .dos_err = 317, .posix_err = 0, .msg = R"(Error in assigning inner IP address to initiator in tunnel mode.)"},
	{.status = 13900, .dos_err = 317, .posix_err = 0, .msg = R"(Require configuration payload missing.)"},
	{.status = 13901, .dos_err = 317, .posix_err = 0, .msg = R"(A negotiation running as the security principle who issued the connection is in progress)"},
	{.status = 13902, .dos_err = 317, .posix_err = 0, .msg = R"(SA was deleted due to IKEv1/AuthIP co-existence suppress check.)"},
	{.status = 13903, .dos_err = 317, .posix_err = 0, .msg = R"(Incoming SA request was dropped due to peer IP address rate limiting.)"},
	{.status = 13904, .dos_err = 317, .posix_err = 0, .msg = R"(Peer does not support MOBIKE.)"},
	{.status = 13905, .dos_err = 317, .posix_err = 0, .msg = R"(SA establishment is not authorized.)"},
	{.status = 13906, .dos_err = 317, .posix_err = 0, .msg = R"(SA establishment is not authorized because there is not a sufficiently strong PKINIT-based credential.)"},
	{.status = 13907, .dos_err = 317, .posix_err = 0, .msg = R"(SA establishment is not authorized.  You may need to enter updated or different credentials such as a smartcard.)"},
	{.status = 13908, .dos_err = 317, .posix_err = 0, .msg = R"(SA establishment is not authorized because there is not a sufficiently strong PKINIT-based credential. This might be related to certificate-to-account mapping failure for the SA.)"},
	{.status = 13909, .dos_err = 317, .posix_err = 0, .msg = R"(ERROR_IPSEC_IKE_NEG_STATUS_EXTENDED_END)"},
	{.status = 13910, .dos_err = 317, .posix_err = 0, .msg = R"(The SPI in the packet does not match a valid IPsec SA.)"},
	{.status = 13911, .dos_err = 317, .posix_err = 0, .msg = R"(Packet was received on an IPsec SA whose lifetime has expired.)"},
	{.status = 13912, .dos_err = 317, .posix_err = 0, .msg = R"(Packet was received on an IPsec SA that does not match the packet characteristics.)"},
	{.status = 13913, .dos_err = 317, .posix_err = 0, .msg = R"(Packet sequence number replay check failed.)"},
	{.status = 13914, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec header and/or trailer in the packet is invalid.)"},
	{.status = 13915, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec integrity check failed.)"},
	{.status = 13916, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec dropped a clear text packet.)"},
	{.status = 13917, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec dropped an incoming ESP packet in authenticated firewall mode. This drop is benign.)"},
	{.status = 13918, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec dropped a packet due to DoS throttling.)"},
	{.status = 13925, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection matched an explicit block rule.)"},
	{.status = 13926, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection received an IPsec specific multicast packet which is not allowed.)"},
	{.status = 13927, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection received an incorrectly formatted packet.)"},
	{.status = 13928, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection failed to look up state.)"},
	{.status = 13929, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection failed to create state because the maximum number of entries allowed by policy has been reached.)"},
	{.status = 13930, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection received an IPsec negotiation packet for a keying module which is not allowed by policy.)"},
	{.status = 13931, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection has not been enabled.)"},
	{.status = 13932, .dos_err = 317, .posix_err = 0, .msg = R"(IPsec DoS Protection failed to create a per internal IP rate limit queue because the maximum number of queues allowed by policy has been reached.)"},
	{.status = 14000, .dos_err = 317, .posix_err = 0, .msg = R"(The requested section was not present in the activation context.)"},
	{.status = 14001, .dos_err = 317, .posix_err = 0, .msg = R"(The application has failed to start because its side-by-side configuration is incorrect. Please see the application event log or use the command-line sxstrace.exe tool for more detail.)"},
	{.status = 14002, .dos_err = 317, .posix_err = 0, .msg = R"(The application binding data format is invalid.)"},
	{.status = 14003, .dos_err = 317, .posix_err = 0, .msg = R"(The referenced assembly is not installed on your system.)"},
	{.status = 14004, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest file does not begin with the required tag and format information.)"},
	{.status = 14005, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest file contains one or more syntax errors.)"},
	{.status = 14006, .dos_err = 317, .posix_err = 0, .msg = R"(The application attempted to activate a disabled activation context.)"},
	{.status = 14007, .dos_err = 317, .posix_err = 0, .msg = R"(The requested lookup key was not found in any active activation context.)"},
	{.status = 14008, .dos_err = 317, .posix_err = 0, .msg = R"(A component version required by the application conflicts with another component version already active.)"},
	{.status = 14009, .dos_err = 317, .posix_err = 0, .msg = R"(The type requested activation context section does not match the query API used.)"},
	{.status = 14010, .dos_err = 317, .posix_err = 0, .msg = R"(Lack of system resources has required isolated activation to be disabled for the current thread of execution.)"},
	{.status = 14011, .dos_err = 317, .posix_err = 0, .msg = R"(An attempt to set the process default activation context failed because the process default activation context was already set.)"},
	{.status = 14012, .dos_err = 317, .posix_err = 0, .msg = R"(The encoding group identifier specified is not recognized.)"},
	{.status = 14013, .dos_err = 317, .posix_err = 0, .msg = R"(The encoding requested is not recognized.)"},
	{.status = 14014, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest contains a reference to an invalid URI.)"},
	{.status = 14015, .dos_err = 317, .posix_err = 0, .msg = R"(The application manifest contains a reference to a dependent assembly which is not installed)"},
	{.status = 14016, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest for an assembly used by the application has a reference to a dependent assembly which is not installed)"},
	{.status = 14017, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest contains an attribute for the assembly identity which is not valid.)"},
	{.status = 14018, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest is missing the required default namespace specification on the assembly element.)"},
	{.status = 14019, .dos_err = 317, .posix_err = 0, .msg = R"(The manifest has a default namespace specified on the assembly element but its value is not "urn:schemas-microsoft-com:asm.v1".)"},
	{.status = 14020, .dos_err = 317, .posix_err = 0, .msg = R"(The private manifest probed has crossed a path with an unsupported reparse point.)"},
	{.status = 14021, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have files by the same name.)"},
	{.status = 14022, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have window classes with the same name.)"},
	{.status = 14023, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have the same COM server CLSIDs.)"},
	{.status = 14024, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have proxies for the same COM interface IIDs.)"},
	{.status = 14025, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have the same COM type library TLBIDs.)"},
	{.status = 14026, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have the same COM ProgIDs.)"},
	{.status = 14027, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest are different versions of the same component which is not permitted.)"},
	{.status = 14028, .dos_err = 317, .posix_err = 0, .msg = R"(A component's file does not match the verification information present in the component manifest.)"},
	{.status = 14029, .dos_err = 317, .posix_err = 0, .msg = R"(The policy manifest contains one or more syntax errors.)"},
	{.status = 14030, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A string literal was expected, but no opening quote character was found.)"},
	{.status = 14031, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Incorrect syntax was used in a comment.)"},
	{.status = 14032, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A name was started with an invalid character.)"},
	{.status = 14033, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A name contained an invalid character.)"},
	{.status = 14034, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A string literal contained an invalid character.)"},
	{.status = 14035, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid syntax for an xml declaration.)"},
	{.status = 14036, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : An Invalid character was found in text content.)"},
	{.status = 14037, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Required white space was missing.)"},
	{.status = 14038, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : The character '>' was expected.)"},
	{.status = 14039, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A semi colon character was expected.)"},
	{.status = 14040, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Unbalanced parentheses.)"},
	{.status = 14041, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Internal error.)"},
	{.status = 14042, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Whitespace is not allowed at this location.)"},
	{.status = 14043, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : End of file reached in invalid state for current encoding.)"},
	{.status = 14044, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Missing parenthesis.)"},
	{.status = 14045, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A single or double closing quote character (\' or \") is missing.)"},
	{.status = 14046, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Multiple colons are not allowed in a name.)"},
	{.status = 14047, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid character for decimal digit.)"},
	{.status = 14048, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid character for hexadecimal digit.)"},
	{.status = 14049, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid unicode character value for this platform.)"},
	{.status = 14050, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Expecting whitespace or '?'.)"},
	{.status = 14051, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : End tag was not expected at this location.)"},
	{.status = 14052, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : The following tags were not closed: %1.)"},
	{.status = 14053, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Duplicate attribute.)"},
	{.status = 14054, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Only one top level element is allowed in an XML document.)"},
	{.status = 14055, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid at the top level of the document.)"},
	{.status = 14056, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid xml declaration.)"},
	{.status = 14057, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : XML document must have a top level element.)"},
	{.status = 14058, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Unexpected end of file.)"},
	{.status = 14059, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Parameter entities cannot be used inside markup declarations in an internal subset.)"},
	{.status = 14060, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Element was not closed.)"},
	{.status = 14061, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : End element was missing the character '>'.)"},
	{.status = 14062, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A string literal was not closed.)"},
	{.status = 14063, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A comment was not closed.)"},
	{.status = 14064, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A declaration was not closed.)"},
	{.status = 14065, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : A CDATA section was not closed.)"},
	{.status = 14066, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : The namespace prefix is not allowed to start with the reserved string "xml".)"},
	{.status = 14067, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : System does not support the specified encoding.)"},
	{.status = 14068, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Switch from current encoding to specified encoding not supported.)"},
	{.status = 14069, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : The name 'xml' is reserved and must be lower case.)"},
	{.status = 14070, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : The standalone attribute must have the value 'yes' or 'no'.)"},
	{.status = 14071, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : The standalone attribute cannot be used in external entities.)"},
	{.status = 14072, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Invalid version number.)"},
	{.status = 14073, .dos_err = 317, .posix_err = 0, .msg = R"(Manifest Parse Error : Missing equals sign between attribute and attribute value.)"},
	{.status = 14074, .dos_err = 317, .posix_err = 0, .msg = R"(Assembly Protection Error : Unable to recover the specified assembly.)"},
	{.status = 14075, .dos_err = 317, .posix_err = 0, .msg = R"(Assembly Protection Error : The public key for an assembly was too short to be allowed.)"},
	{.status = 14076, .dos_err = 317, .posix_err = 0, .msg = R"(Assembly Protection Error : The catalog for an assembly is not valid, or does not match the assembly's manifest.)"},
	{.status = 14077, .dos_err = 317, .posix_err = 0, .msg = R"(An HRESULT could not be translated to a corresponding Win32 error code.)"},
	{.status = 14078, .dos_err = 317, .posix_err = 0, .msg = R"(Assembly Protection Error : The catalog for an assembly is missing.)"},
	{.status = 14079, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied assembly identity is missing one or more attributes which must be present in this context.)"},
	{.status = 14080, .dos_err = 317, .posix_err = 0, .msg = R"(The supplied assembly identity has one or more attribute names that contain characters not permitted in XML names.)"},
	{.status = 14081, .dos_err = 317, .posix_err = 0, .msg = R"(The referenced assembly could not be found.)"},
	{.status = 14082, .dos_err = 317, .posix_err = 0, .msg = R"(The activation context activation stack for the running thread of execution is corrupt.)"},
	{.status = 14083, .dos_err = 317, .posix_err = 0, .msg = R"(The application isolation metadata for this process or thread has become corrupt.)"},
	{.status = 14084, .dos_err = 317, .posix_err = 0, .msg = R"(The activation context being deactivated is not the most recently activated one.)"},
	{.status = 14085, .dos_err = 317, .posix_err = 0, .msg = R"(The activation context being deactivated is not active for the current thread of execution.)"},
	{.status = 14086, .dos_err = 317, .posix_err = 0, .msg = R"(The activation context being deactivated has already been deactivated.)"},
	{.status = 14087, .dos_err = 317, .posix_err = 0, .msg = R"(A component used by the isolation facility has requested to terminate the process.)"},
	{.status = 14088, .dos_err = 317, .posix_err = 0, .msg = R"(A kernel mode component is releasing a reference on an activation context.)"},
	{.status = 14089, .dos_err = 317, .posix_err = 0, .msg = R"(The activation context of system default assembly could not be generated.)"},
	{.status = 14090, .dos_err = 317, .posix_err = 0, .msg = R"(The value of an attribute in an identity is not within the legal range.)"},
	{.status = 14091, .dos_err = 317, .posix_err = 0, .msg = R"(The name of an attribute in an identity is not within the legal range.)"},
	{.status = 14092, .dos_err = 317, .posix_err = 0, .msg = R"(An identity contains two definitions for the same attribute.)"},
	{.status = 14093, .dos_err = 317, .posix_err = 0, .msg = R"(The identity string is malformed. This may be due to a trailing comma, more than two unnamed attributes, missing attribute name or missing attribute value.)"},
	{.status = 14094, .dos_err = 317, .posix_err = 0, .msg = R"(A string containing localized substitutable content was malformed. Either a dollar sign ($) was followed by something other than a left parenthesis or another dollar sign or an substitution's right parenthesis was not found.)"},
	{.status = 14095, .dos_err = 317, .posix_err = 0, .msg = R"(The public key token does not correspond to the public key specified.)"},
	{.status = 14096, .dos_err = 317, .posix_err = 0, .msg = R"(A substitution string had no mapping.)"},
	{.status = 14097, .dos_err = 317, .posix_err = 0, .msg = R"(The component must be locked before making the request.)"},
	{.status = 14098, .dos_err = 317, .posix_err = 0, .msg = R"(The component store has been corrupted.)"},
	{.status = 14099, .dos_err = 317, .posix_err = 0, .msg = R"(An advanced installer failed during setup or servicing.)"},
	{.status = 14100, .dos_err = 317, .posix_err = 0, .msg = R"(The character encoding in the XML declaration did not match the encoding used in the document.)"},
	{.status = 14101, .dos_err = 317, .posix_err = 0, .msg = R"(The identities of the manifests are identical but their contents are different.)"},
	{.status = 14102, .dos_err = 317, .posix_err = 0, .msg = R"(The component identities are different.)"},
	{.status = 14103, .dos_err = 317, .posix_err = 0, .msg = R"(The assembly is not a deployment.)"},
	{.status = 14104, .dos_err = 317, .posix_err = 0, .msg = R"(The file is not a part of the assembly.)"},
	{.status = 14105, .dos_err = 317, .posix_err = 0, .msg = R"(The size of the manifest exceeds the maximum allowed.)"},
	{.status = 14106, .dos_err = 317, .posix_err = 0, .msg = R"(The setting is not registered.)"},
	{.status = 14107, .dos_err = 317, .posix_err = 0, .msg = R"(One or more required members of the transaction are not present.)"},
	{.status = 14108, .dos_err = 317, .posix_err = 0, .msg = R"(The SMI primitive installer failed during setup or servicing.)"},
	{.status = 14109, .dos_err = 317, .posix_err = 0, .msg = R"(A generic command executable returned a result that indicates failure.)"},
	{.status = 14110, .dos_err = 317, .posix_err = 0, .msg = R"(A component is missing file verification information in its manifest.)"},
	{.status = 14111, .dos_err = 317, .posix_err = 0, .msg = R"(Two or more components referenced directly or indirectly by the application manifest have the same WinRT ActivatableClass IDs.)"},
	{.status = 15000, .dos_err = 317, .posix_err = 0, .msg = R"(The specified channel path is invalid.)"},
	{.status = 15001, .dos_err = 317, .posix_err = 0, .msg = R"(The specified query is invalid.)"},
	{.status = 15002, .dos_err = 317, .posix_err = 0, .msg = R"(The publisher metadata cannot be found in the resource.)"},
	{.status = 15003, .dos_err = 317, .posix_err = 0, .msg = R"(The template for an event definition cannot be found in the resource (error = %1).)"},
	{.status = 15004, .dos_err = 317, .posix_err = 0, .msg = R"(The specified publisher name is invalid.)"},
	{.status = 15005, .dos_err = 317, .posix_err = 0, .msg = R"(The event data raised by the publisher is not compatible with the event template definition in the publisher's manifest.)"},
	{.status = 15007, .dos_err = 317, .posix_err = 0, .msg = R"(The specified channel could not be found.)"},
	{.status = 15008, .dos_err = 317, .posix_err = 0, .msg = R"(The specified XML text was not well-formed. See Extended Error for more details.)"},
	{.status = 15009, .dos_err = 317, .posix_err = 0, .msg = R"(The events for a direct channel go directly to a log file and cannot be subscribed to.)"},
	{.status = 15010, .dos_err = 317, .posix_err = 0, .msg = R"(Configuration error.)"},
	{.status = 15011, .dos_err = 317, .posix_err = 0, .msg = R"(The query result is stale or invalid and must be recreated. This may be due to the log being cleared or rolling over after the query result was created.)"},
	{.status = 15012, .dos_err = 317, .posix_err = 0, .msg = R"(The query result is currently at an invalid position.)"},
	{.status = 15013, .dos_err = 317, .posix_err = 0, .msg = R"(Registered MSXML doesn't support validation.)"},
	{.status = 15014, .dos_err = 317, .posix_err = 0, .msg = R"(An expression can only be followed by a change-of-scope operation if the expression evaluates to a node set and is not already part of another change-of-scope operation.)"},
	{.status = 15015, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot perform a step operation from a term that does not represent an element set.)"},
	{.status = 15016, .dos_err = 317, .posix_err = 0, .msg = R"(Left-hand side arguments to binary operators must be either attributes, nodes or variables. Right-hand side arguments must be constants.)"},
	{.status = 15017, .dos_err = 317, .posix_err = 0, .msg = R"(A step operation must involve a node test or, in the case of a predicate, an algebraic expression against which to test each node in the preceeding node set.)"},
	{.status = 15018, .dos_err = 317, .posix_err = 0, .msg = R"(This data type is currently unsupported.)"},
	{.status = 15019, .dos_err = 317, .posix_err = 0, .msg = R"(A syntax error occurred at position %1!d!)"},
	{.status = 15020, .dos_err = 317, .posix_err = 0, .msg = R"(This operator is unsupported by this implementation of the filter.)"},
	{.status = 15021, .dos_err = 317, .posix_err = 0, .msg = R"(An unexpected token was encountered.)"},
	{.status = 15022, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation cannot be performed over an enabled direct channel. The channel must first be disabled.)"},
	{.status = 15023, .dos_err = 317, .posix_err = 0, .msg = R"(Channel property %1 contains an invalid value. The value has an invalid type, is outside of its valid range, cannot be changed, or is not supported by this type of channel.)"},
	{.status = 15024, .dos_err = 317, .posix_err = 0, .msg = R"(Publisher property %1 contains an invalid value. The value has an invalid type, is outside of its valid range, cannot be changed, or is not supported by this type of publisher.)"},
	{.status = 15025, .dos_err = 317, .posix_err = 0, .msg = R"(The channel failed to activate.)"},
	{.status = 15026, .dos_err = 317, .posix_err = 0, .msg = R"(The XPath expression exceeded the supported complexity. Simplify the expression or split it into multiple expressions.)"},
	{.status = 15027, .dos_err = 317, .posix_err = 0, .msg = R"(The message resource is present but the message was not found in the message table.)"},
	{.status = 15028, .dos_err = 317, .posix_err = 0, .msg = R"(The message ID for the desired message could not be found.)"},
	{.status = 15029, .dos_err = 317, .posix_err = 0, .msg = R"(The substitution string for insert index (%1) could not be found.)"},
	{.status = 15030, .dos_err = 317, .posix_err = 0, .msg = R"(The description string for parameter reference (%1) could not be found.)"},
	{.status = 15031, .dos_err = 317, .posix_err = 0, .msg = R"(The maximum number of replacements has been reached.)"},
	{.status = 15032, .dos_err = 317, .posix_err = 0, .msg = R"(The event definition could not be found for event ID (%1).)"},
	{.status = 15033, .dos_err = 317, .posix_err = 0, .msg = R"(The locale specific resource for the desired message is not present.)"},
	{.status = 15034, .dos_err = 317, .posix_err = 0, .msg = R"(The resource is too old and is not supported.)"},
	{.status = 15035, .dos_err = 317, .posix_err = 0, .msg = R"(The resource is too new and is not supported.)"},
	{.status = 15036, .dos_err = 317, .posix_err = 0, .msg = R"(The channel at index %1!d! of the query can't be opened.)"},
	{.status = 15037, .dos_err = 317, .posix_err = 0, .msg = R"(The publisher has been disabled and its resource is not available. This usually occurs when the publisher is in the process of being uninstalled or upgraded.)"},
	{.status = 15038, .dos_err = 317, .posix_err = 0, .msg = R"(Attempted to create a numeric type that is outside of its valid range.)"},
	{.status = 15080, .dos_err = 317, .posix_err = 0, .msg = R"(The subscription fails to activate.)"},
	{.status = 15081, .dos_err = 317, .posix_err = 0, .msg = R"(The log of the subscription is in disabled state, and can not be used to forward events to. The log must first be enabled before the subscription can be activated.)"},
	{.status = 15082, .dos_err = 317, .posix_err = 0, .msg = R"(When forwarding events from local machine to itself, the query of the subscription can't contain target log of the subscription.)"},
	{.status = 15083, .dos_err = 317, .posix_err = 0, .msg = R"(The credential store that is used to save credentials is full.)"},
	{.status = 15084, .dos_err = 317, .posix_err = 0, .msg = R"(The credential used by this subscription can't be found in credential store.)"},
	{.status = 15085, .dos_err = 317, .posix_err = 0, .msg = R"(No active channel is found for the query.)"},
	{.status = 15100, .dos_err = 317, .posix_err = 0, .msg = R"(The resource loader failed to find MUI file.)"},
	{.status = 15101, .dos_err = 317, .posix_err = 0, .msg = R"(The resource loader failed to load MUI file because the file fail to pass validation.)"},
	{.status = 15102, .dos_err = 317, .posix_err = 0, .msg = R"(The RC Manifest is corrupted with garbage data or unsupported version or missing required item.)"},
	{.status = 15103, .dos_err = 317, .posix_err = 0, .msg = R"(The RC Manifest has invalid culture name.)"},
	{.status = 15104, .dos_err = 317, .posix_err = 0, .msg = R"(The RC Manifest has invalid ultimatefallback name.)"},
	{.status = 15105, .dos_err = 317, .posix_err = 0, .msg = R"(The resource loader cache doesn't have loaded MUI entry.)"},
	{.status = 15106, .dos_err = 317, .posix_err = 0, .msg = R"(User stopped resource enumeration.)"},
	{.status = 15107, .dos_err = 317, .posix_err = 0, .msg = R"(UI language installation failed.)"},
	{.status = 15108, .dos_err = 317, .posix_err = 0, .msg = R"(Locale installation failed.)"},
	{.status = 15110, .dos_err = 317, .posix_err = 0, .msg = R"(A resource does not have default or neutral value.)"},
	{.status = 15111, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid PRI config file.)"},
	{.status = 15112, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid file type.)"},
	{.status = 15113, .dos_err = 317, .posix_err = 0, .msg = R"(Unknown qualifier.)"},
	{.status = 15114, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid qualifier value.)"},
	{.status = 15115, .dos_err = 317, .posix_err = 0, .msg = R"(No Candidate found.)"},
	{.status = 15116, .dos_err = 317, .posix_err = 0, .msg = R"(The ResourceMap or NamedResource has an item that does not have default or neutral resource..)"},
	{.status = 15117, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid ResourceCandidate type.)"},
	{.status = 15118, .dos_err = 317, .posix_err = 0, .msg = R"(Duplicate Resource Map.)"},
	{.status = 15119, .dos_err = 317, .posix_err = 0, .msg = R"(Duplicate Entry.)"},
	{.status = 15120, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid Resource Identifier.)"},
	{.status = 15121, .dos_err = 317, .posix_err = 0, .msg = R"(Filepath too long.)"},
	{.status = 15122, .dos_err = 317, .posix_err = 0, .msg = R"(Unsupported directory type.)"},
	{.status = 15126, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid PRI File.)"},
	{.status = 15127, .dos_err = 317, .posix_err = 0, .msg = R"(NamedResource Not Found.)"},
	{.status = 15135, .dos_err = 317, .posix_err = 0, .msg = R"(ResourceMap Not Found.)"},
	{.status = 15136, .dos_err = 317, .posix_err = 0, .msg = R"(Unsupported MRT profile type.)"},
	{.status = 15137, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid qualifier operator.)"},
	{.status = 15138, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to determine qualifier value or qualifier value has not been set.)"},
	{.status = 15139, .dos_err = 317, .posix_err = 0, .msg = R"(Automerge is enabled in the PRI file.)"},
	{.status = 15140, .dos_err = 317, .posix_err = 0, .msg = R"(Too many resources defined for package.)"},
	{.status = 15141, .dos_err = 317, .posix_err = 0, .msg = R"(Resource File can not be used for merge operation.)"},
	{.status = 15142, .dos_err = 317, .posix_err = 0, .msg = R"(Load/UnloadPriFiles cannot be used with resource packages.)"},
	{.status = 15143, .dos_err = 317, .posix_err = 0, .msg = R"(Resource Contexts may not be created on threads that do not have a CoreWindow.)"},
	{.status = 15144, .dos_err = 317, .posix_err = 0, .msg = R"(The singleton Resource Manager with different profile is already created.)"},
	{.status = 15145, .dos_err = 317, .posix_err = 0, .msg = R"(The system component cannot operate given API operation)"},
	{.status = 15146, .dos_err = 317, .posix_err = 0, .msg = R"(The resource is a direct reference to a non-default resource candidate.)"},
	{.status = 15147, .dos_err = 317, .posix_err = 0, .msg = R"(Resource Map has been re-generated and the query string is not valid anymore.)"},
	{.status = 15148, .dos_err = 317, .posix_err = 0, .msg = R"(The PRI files to be merged have incompatible versions.)"},
	{.status = 15149, .dos_err = 317, .posix_err = 0, .msg = R"(The primary PRI files to be merged does not contain a schema.)"},
	{.status = 15150, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to load one of the PRI files to be merged.)"},
	{.status = 15151, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to add one of the PRI files to the merged file.)"},
	{.status = 15152, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to create the merged PRI file.)"},
	{.status = 15153, .dos_err = 317, .posix_err = 0, .msg = R"(Packages for a PRI file merge must all be from the same package family.)"},
	{.status = 15154, .dos_err = 317, .posix_err = 0, .msg = R"(Packages for a PRI file merge must not include multiple main packages.)"},
	{.status = 15155, .dos_err = 317, .posix_err = 0, .msg = R"(Packages for a PRI file merge must not include bundle packages.)"},
	{.status = 15156, .dos_err = 317, .posix_err = 0, .msg = R"(Packages for a PRI file merge must include one main package.)"},
	{.status = 15157, .dos_err = 317, .posix_err = 0, .msg = R"(Packages for a PRI file merge must include at least one resource package.)"},
	{.status = 15158, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid name supplied for a canonical merged PRI file.)"},
	{.status = 15159, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to find the specified package.)"},
	{.status = 15160, .dos_err = 317, .posix_err = 0, .msg = R"(No default value for language was specified.)"},
	{.status = 15200, .dos_err = 317, .posix_err = 0, .msg = R"(The monitor returned a DDC/CI capabilities string that did not comply with the ACCESS.bus 3.0, DDC/CI 1.1 or MCCS 2 Revision 1 specification.)"},
	{.status = 15201, .dos_err = 317, .posix_err = 0, .msg = R"(The monitor's VCP Version (0xDF) VCP code returned an invalid version value.)"},
	{.status = 15202, .dos_err = 317, .posix_err = 0, .msg = R"(The monitor does not comply with the MCCS specification it claims to support.)"},
	{.status = 15203, .dos_err = 317, .posix_err = 0, .msg = R"(The MCCS version in a monitor's mccs_ver capability does not match the MCCS version the monitor reports when the VCP Version (0xDF) VCP code is used.)"},
	{.status = 15204, .dos_err = 317, .posix_err = 0, .msg = R"(The Monitor Configuration API only works with monitors that support the MCCS 1.0 specification, MCCS 2.0 specification or the MCCS 2.0 Revision 1 specification.)"},
	{.status = 15205, .dos_err = 317, .posix_err = 0, .msg = R"(An internal Monitor Configuration API error occurred.)"},
	{.status = 15206, .dos_err = 317, .posix_err = 0, .msg = R"(The monitor returned an invalid monitor technology type. CRT, Plasma and LCD (TFT) are examples of monitor technology types. This error implies that the monitor violated the MCCS 2.0 or MCCS 2.0 Revision 1 specification.)"},
	{.status = 15207, .dos_err = 317, .posix_err = 0, .msg = R"(The caller of SetMonitorColorTemperature specified a color temperature that the current monitor did not support. This error implies that the monitor violated the MCCS 2.0 or MCCS 2.0 Revision 1 specification.)"},
	{.status = 15250, .dos_err = 317, .posix_err = 0, .msg = R"(The requested system device cannot be identified due to multiple indistinguishable devices potentially matching the identification criteria.)"},
	{.status = 15299, .dos_err = 317, .posix_err = 0, .msg = R"(The requested system device cannot be found.)"},
	{.status = 15300, .dos_err = 317, .posix_err = 0, .msg = R"(Hash generation for the specified hash version and hash type is not enabled on the server.)"},
	{.status = 15301, .dos_err = 317, .posix_err = 0, .msg = R"(The hash requested from the server is not available or no longer valid.)"},
	{.status = 15321, .dos_err = 317, .posix_err = 0, .msg = R"(The secondary interrupt controller instance that manages the specified interrupt is not registered.)"},
	{.status = 15322, .dos_err = 317, .posix_err = 0, .msg = R"(The information supplied by the GPIO client driver is invalid.)"},
	{.status = 15323, .dos_err = 317, .posix_err = 0, .msg = R"(The version specified by the GPIO client driver is not supported.)"},
	{.status = 15324, .dos_err = 317, .posix_err = 0, .msg = R"(The registration packet supplied by the GPIO client driver is not valid.)"},
	{.status = 15325, .dos_err = 317, .posix_err = 0, .msg = R"(The requested operation is not supported for the specified handle.)"},
	{.status = 15326, .dos_err = 317, .posix_err = 0, .msg = R"(The requested connect mode conflicts with an existing mode on one or more of the specified pins.)"},
	{.status = 15327, .dos_err = 317, .posix_err = 0, .msg = R"(The interrupt requested to be unmasked is not masked.)"},
	{.status = 15400, .dos_err = 317, .posix_err = 0, .msg = R"(The requested run level switch cannot be completed successfully.)"},
	{.status = 15401, .dos_err = 317, .posix_err = 0, .msg = R"(The service has an invalid run level setting. The run level for a service

must not be higher than the run level of its dependent services.)"},
	{.status = 15402, .dos_err = 317, .posix_err = 0, .msg = R"(The requested run level switch cannot be completed successfully since

one or more services will not stop or restart within the specified timeout.)"},
	{.status = 15403, .dos_err = 317, .posix_err = 0, .msg = R"(A run level switch agent did not respond within the specified timeout.)"},
	{.status = 15404, .dos_err = 317, .posix_err = 0, .msg = R"(A run level switch is currently in progress.)"},
	{.status = 15405, .dos_err = 317, .posix_err = 0, .msg = R"(One or more services failed to start during the service startup phase of a run level switch.)"},
	{.status = 15501, .dos_err = 317, .posix_err = 0, .msg = R"(The task stop request cannot be completed immediately since

task needs more time to shutdown.)"},
	{.status = 15600, .dos_err = 317, .posix_err = 0, .msg = R"(Package could not be opened.)"},
	{.status = 15601, .dos_err = 317, .posix_err = 0, .msg = R"(Package was not found.)"},
	{.status = 15602, .dos_err = 317, .posix_err = 0, .msg = R"(Package data is invalid.)"},
	{.status = 15603, .dos_err = 317, .posix_err = 0, .msg = R"(Package failed updates, dependency or conflict validation.)"},
	{.status = 15604, .dos_err = 317, .posix_err = 0, .msg = R"(There is not enough disk space on your computer. Please free up some space and try again.)"},
	{.status = 15605, .dos_err = 317, .posix_err = 0, .msg = R"(There was a problem downloading your product.)"},
	{.status = 15606, .dos_err = 317, .posix_err = 0, .msg = R"(Package could not be registered.)"},
	{.status = 15607, .dos_err = 317, .posix_err = 0, .msg = R"(Package could not be unregistered.)"},
	{.status = 15608, .dos_err = 317, .posix_err = 0, .msg = R"(User cancelled the install request.)"},
	{.status = 15609, .dos_err = 317, .posix_err = 0, .msg = R"(Install failed. Please contact your software vendor.)"},
	{.status = 15610, .dos_err = 317, .posix_err = 0, .msg = R"(Removal failed. Please contact your software vendor.)"},
	{.status = 15611, .dos_err = 317, .posix_err = 0, .msg = R"(The provided package is already installed, and reinstallation of the package was blocked. Check the AppXDeployment-Server event log for details.)"},
	{.status = 15612, .dos_err = 317, .posix_err = 0, .msg = R"(The application cannot be started. Try reinstalling the application to fix the problem.)"},
	{.status = 15613, .dos_err = 317, .posix_err = 0, .msg = R"(A Prerequisite for an install could not be satisfied.)"},
	{.status = 15614, .dos_err = 317, .posix_err = 0, .msg = R"(The package repository is corrupted.)"},
	{.status = 15615, .dos_err = 317, .posix_err = 0, .msg = R"(To install this application you need either a Windows developer license or a sideloading-enabled system.)"},
	{.status = 15616, .dos_err = 317, .posix_err = 0, .msg = R"(The application cannot be started because it is currently updating.)"},
	{.status = 15617, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment operation is blocked by policy. Please contact your system administrator.)"},
	{.status = 15618, .dos_err = 317, .posix_err = 0, .msg = R"(The package could not be installed because resources it modifies are currently in use.)"},
	{.status = 15619, .dos_err = 317, .posix_err = 0, .msg = R"(The package could not be recovered because necessary data for recovery have been corrupted.)"},
	{.status = 15620, .dos_err = 317, .posix_err = 0, .msg = R"(The signature is invalid. To register in developer mode, AppxSignature.p7x and AppxBlockMap.xml must be valid or should not be present.)"},
	{.status = 15621, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred while deleting the package's previously existing application data.)"},
	{.status = 15622, .dos_err = 317, .posix_err = 0, .msg = R"(The package could not be installed because a higher version of this package is already installed.)"},
	{.status = 15623, .dos_err = 317, .posix_err = 0, .msg = R"(An error in a system binary was detected. Try refreshing the PC to fix the problem.)"},
	{.status = 15624, .dos_err = 317, .posix_err = 0, .msg = R"(A corrupted CLR NGEN binary was detected on the system.)"},
	{.status = 15625, .dos_err = 317, .posix_err = 0, .msg = R"(The operation could not be resumed because necessary data for recovery have been corrupted.)"},
	{.status = 15626, .dos_err = 317, .posix_err = 0, .msg = R"(The package could not be installed because the Windows Firewall service is not running. Enable the Windows Firewall service and try again.)"},
	{.status = 15627, .dos_err = 317, .posix_err = 0, .msg = R"(Package move failed.)"},
	{.status = 15628, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation failed because the volume is not empty.)"},
	{.status = 15629, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation failed because the volume is offline. For a package update, the volume refers to the installed volume of all package versions.)"},
	{.status = 15630, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation failed because the specified volume is corrupt.)"},
	{.status = 15631, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation failed because the specified application needs to be registered first.)"},
	{.status = 15632, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation failed because the package targets the wrong processor architecture.)"},
	{.status = 15633, .dos_err = 317, .posix_err = 0, .msg = R"(You have reached the maximum number of developer sideloaded packages allowed on this device. Please uninstall a sideloaded package and try again.)"},
	{.status = 15634, .dos_err = 317, .posix_err = 0, .msg = R"(A main app package is required to install this optional package.  Install the main package first and try again.)"},
	{.status = 15635, .dos_err = 317, .posix_err = 0, .msg = R"(This app package type is not supported on this filesystem)"},
	{.status = 15636, .dos_err = 317, .posix_err = 0, .msg = R"(Package move operation is blocked until the application has finished streaming)"},
	{.status = 15637, .dos_err = 317, .posix_err = 0, .msg = R"(A main or another optional app package has the same application ID as this optional package.  Change the application ID for the optional package to avoid conflicts.)"},
	{.status = 15638, .dos_err = 317, .posix_err = 0, .msg = R"(This staging session has been held to allow another staging operation to be prioritized.)"},
	{.status = 15639, .dos_err = 317, .posix_err = 0, .msg = R"(A related set cannot be updated because the updated set is invalid. All packages in the related set must be updated at the same time.)"},
	{.status = 15640, .dos_err = 317, .posix_err = 0, .msg = R"(An optional package with a FullTrust entry point requires the main package to have the runFullTrust capability.)"},
	{.status = 15641, .dos_err = 317, .posix_err = 0, .msg = R"(An error occurred because a user was logged off.)"},
	{.status = 15642, .dos_err = 317, .posix_err = 0, .msg = R"(An optional package provision requires the dependency main package to also be provisioned.)"},
	{.status = 15643, .dos_err = 317, .posix_err = 0, .msg = R"(The packages failed the SmartScreen reputation check.)"},
	{.status = 15644, .dos_err = 317, .posix_err = 0, .msg = R"(The SmartScreen reputation check operation timed out.)"},
	{.status = 15645, .dos_err = 317, .posix_err = 0, .msg = R"(The current deployment option is not supported.)"},
	{.status = 15646, .dos_err = 317, .posix_err = 0, .msg = R"(Activation is blocked due to the .appinstaller update settings for this app.)"},
	{.status = 15647, .dos_err = 317, .posix_err = 0, .msg = R"(Remote drives are not supported; use \\server\share to register a remote package.)"},
	{.status = 15648, .dos_err = 317, .posix_err = 0, .msg = R"(Failed to process and write downloaded APPX package data to disk.)"},
	{.status = 15649, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation was blocked due to a per-package-family policy restricting deployments on a non-system volume. Per policy, this app must be installed to the system drive, but that's not set as the default. In Storage Settings, make the system drive the default location to save new content, then retry the install.)"},
	{.status = 15650, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation was blocked due to a machine-wide policy restricting deployments on a non-system volume. Per policy, this app must be installed to the system drive, but that's not set as the default. In Storage Settings, make the system drive the default location to save new content, then retry the install.)"},
	{.status = 15651, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation was blocked because Special profile deployment is not allowed. Please try logging into an account that is not a Special profile. You can try logging out and logging back into the current account, or try logging into a different account.)"},
	{.status = 15652, .dos_err = 317, .posix_err = 0, .msg = R"(The deployment operation failed due to a conflicting package's mutable package directory. To install this package remove the existing package with the conflicting mutable package directory.)"},
	{.status = 15653, .dos_err = 317, .posix_err = 0, .msg = R"(The package installation failed because a singleton resource was specified and another user with that package installed is logged in. Please make sure that all active users with the package installed are logged out and retry installation.)"},
	{.status = 15654, .dos_err = 317, .posix_err = 0, .msg = R"(The package installation failed because a different version of the service is installed. Try installing a newer version of the package.)"},
	{.status = 15655, .dos_err = 317, .posix_err = 0, .msg = R"(The package installation failed because a version of the service exists outside of APPX packaging. Please contact your software vendor.)"},
	{.status = 15656, .dos_err = 317, .posix_err = 0, .msg = R"(The package installation failed because administrator privileges are required. Please contact an administrator to install this package.)"},
	{.status = 15657, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment failed because the operation would have redirected to default account, when the caller said not to do so.)"},
	{.status = 15658, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment failed because the package requires a capability to natively target this host.)"},
	{.status = 15659, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment failed because its content is not valid for an unsigned package.)"},
	{.status = 15660, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment failed because its publisher is not in the unsigned namespace.)"},
	{.status = 15661, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment failed because its publisher is not in the signed namespace.)"},
	{.status = 15662, .dos_err = 317, .posix_err = 0, .msg = R"(The package deployment failed because its publisher is not in the signed namespace.)"},
	{.status = 15663, .dos_err = 317, .posix_err = 0, .msg = R"(A host runtime dependency resolving to a package with full trust content requires the main package to have the runFullTrust capability.)"},
	{.status = 15664, .dos_err = 317, .posix_err = 0, .msg = R"(The application cannot be started for the target user.  Please have the user explicitly install this package.)"},
	{.status = 15700, .dos_err = 317, .posix_err = 0, .msg = R"(The process has no package identity.)"},
	{.status = 15701, .dos_err = 317, .posix_err = 0, .msg = R"(The package runtime information is corrupted.)"},
	{.status = 15702, .dos_err = 317, .posix_err = 0, .msg = R"(The package identity is corrupted.)"},
	{.status = 15703, .dos_err = 317, .posix_err = 0, .msg = R"(The process has no application identity.)"},
	{.status = 15704, .dos_err = 317, .posix_err = 0, .msg = R"(One or more AppModel Runtime group policy values could not be read. Please contact your system administrator with the contents of your AppModel Runtime event log.)"},
	{.status = 15705, .dos_err = 317, .posix_err = 0, .msg = R"(One or more AppModel Runtime group policy values are invalid. Please contact your system administrator with the contents of your AppModel Runtime event log.)"},
	{.status = 15706, .dos_err = 317, .posix_err = 0, .msg = R"(The package is currently not available.)"},
	{.status = 15707, .dos_err = 317, .posix_err = 0, .msg = R"(The package does not have a mutable directory.)"},
	{.status = 15800, .dos_err = 317, .posix_err = 0, .msg = R"(Loading the state store failed.)"},
	{.status = 15801, .dos_err = 317, .posix_err = 0, .msg = R"(Retrieving the state version for the application failed.)"},
	{.status = 15802, .dos_err = 317, .posix_err = 0, .msg = R"(Setting the state version for the application failed.)"},
	{.status = 15803, .dos_err = 317, .posix_err = 0, .msg = R"(Resetting the structured state of the application failed.)"},
	{.status = 15804, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to open the container.)"},
	{.status = 15805, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to create the container.)"},
	{.status = 15806, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to delete the container.)"},
	{.status = 15807, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to read the setting.)"},
	{.status = 15808, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to write the setting.)"},
	{.status = 15809, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to delete the setting.)"},
	{.status = 15810, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to query the setting.)"},
	{.status = 15811, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to read the composite setting.)"},
	{.status = 15812, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to write the composite setting.)"},
	{.status = 15813, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to enumerate the containers.)"},
	{.status = 15814, .dos_err = 317, .posix_err = 0, .msg = R"(State Manager failed to enumerate the settings.)"},
	{.status = 15815, .dos_err = 317, .posix_err = 0, .msg = R"(The size of the state manager composite setting value has exceeded the limit.)"},
	{.status = 15816, .dos_err = 317, .posix_err = 0, .msg = R"(The size of the state manager setting value has exceeded the limit.)"},
	{.status = 15817, .dos_err = 317, .posix_err = 0, .msg = R"(The length of the state manager setting name has exceeded the limit.)"},
	{.status = 15818, .dos_err = 317, .posix_err = 0, .msg = R"(The length of the state manager container name has exceeded the limit.)"},
	{.status = 15841, .dos_err = 317, .posix_err = 0, .msg = R"(This API cannot be used in the context of the caller's application type.)"},
	{.status = 15861, .dos_err = 317, .posix_err = 0, .msg = R"(This PC does not have a valid license for the application or product.)"},
	{.status = 15862, .dos_err = 317, .posix_err = 0, .msg = R"(The authenticated user does not have a valid license for the application or product.)"},
	{.status = 15863, .dos_err = 317, .posix_err = 0, .msg = R"(The commerce transaction associated with this license is still pending verification.)"},
	{.status = 15864, .dos_err = 317, .posix_err = 0, .msg = R"(The license has been revoked for this user.)"},
	{.status = 1073741824, .dos_err = 698, .posix_err = 0, .msg = R"({Object Exists}

An attempt was made to create an object and the object name already existed.)"},
	{.status = 1073741825, .dos_err = 699, .posix_err = 0, .msg = R"({Thread Suspended}

A thread termination occurred while the thread was suspended. The thread was resumed, and termination proceeded.)"},
	{.status = 1073741826, .dos_err = 87, .posix_err = 22, .msg = R"({Working Set Range Error}

An attempt was made to set the working set minimum or maximum to values which are outside of the allowable range.)"},
	{.status = 1073741827, .dos_err = 700, .posix_err = 0, .msg = R"({Image Relocated}

An image file could not be mapped at the address specified in the image file. Local fixups must be performed on this image.)"},
	{.status = 1073741828, .dos_err = 701, .posix_err = 0, .msg = R"(This informational level status indicates that a specified registry sub-tree transaction state did not yet exist and had to be created.)"},
	{.status = 1073741829, .dos_err = 702, .posix_err = 0, .msg = R"({Segment Load}

A virtual DOS machine (VDM) is loading, unloading, or moving an MS-DOS or Win16 program segment image.

An exception is raised so a debugger can load, unload or track symbols and breakpoints within these 16-bit segments.)"},
	{.status = 1073741830, .dos_err = 1303, .posix_err = 0, .msg = R"({Local Session Key}

A user session key was requested for a local RPC connection. The session key returned is a constant value and not unique to this connection.)"},
	{.status = 1073741831, .dos_err = 703, .posix_err = 0, .msg = R"({Invalid Current Directory}

The process cannot switch to the startup current directory %hs.

Select OK to set current directory to %hs, or select CANCEL to exit.)"},
	{.status = 1073741832, .dos_err = 1120, .posix_err = 0, .msg = R"({Serial IOCTL Complete}

A serial I/O operation was completed by another write to a serial port.

(The IOCTL_SERIAL_XOFF_COUNTER reached zero.))"},
	{.status = 1073741833, .dos_err = 1014, .posix_err = 0, .msg = R"({Registry Recovery}

One of the files containing the system's Registry data had to be recovered by use of a log or alternate copy. The recovery was successful.)"},
	{.status = 1073741834, .dos_err = 704, .posix_err = 0, .msg = R"({Redundant Read}

To satisfy a read request, the NT fault-tolerant file system successfully read the requested data from a redundant copy.

This was done because the file system encountered a failure on a member of the fault-tolerant volume, but was unable to reassign the failing area of the device.)"},
	{.status = 1073741835, .dos_err = 705, .posix_err = 0, .msg = R"({Redundant Write}

To satisfy a write request, the NT fault-tolerant file system successfully wrote a redundant copy of the information.

This was done because the file system encountered a failure on a member of the fault-tolerant volume, but was not able to reassign the failing area of the device.)"},
	{.status = 1073741836, .dos_err = 1121, .posix_err = 0, .msg = R"({Serial IOCTL Timeout}

A serial I/O operation completed because the time-out period expired. (The IOCTL_SERIAL_XOFF_COUNTER had not reached zero.))"},
	{.status = 1073741837, .dos_err = 1304, .posix_err = 0, .msg = R"({Password Too Complex}

The Windows password is too complex to be converted to a LAN Manager password. The LAN Manager password returned is a NULL string.)"},
	{.status = 1073741838, .dos_err = 706, .posix_err = 0, .msg = R"({Machine Type Mismatch}

The image file %hs is valid, but is for a machine type other than the current machine. Select OK to continue, or CANCEL to fail the DLL load.)"},
	{.status = 1073741839, .dos_err = 707, .posix_err = 0, .msg = R"({Partial Data Received}

The network transport returned partial data to its client. The remaining data will be sent later.)"},
	{.status = 1073741840, .dos_err = 708, .posix_err = 0, .msg = R"({Expedited Data Received}

The network transport returned data to its client that was marked as expedited by the remote system.)"},
	{.status = 1073741841, .dos_err = 709, .posix_err = 0, .msg = R"({Partial Expedited Data Received}

The network transport returned partial data to its client and this data was marked as expedited by the remote system. The remaining data will be sent later.)"},
	{.status = 1073741842, .dos_err = 710, .posix_err = 0, .msg = R"({TDI Event Done}

The TDI indication has completed successfully.)"},
	{.status = 1073741843, .dos_err = 711, .posix_err = 0, .msg = R"({TDI Event Pending}

The TDI indication has entered the pending state.)"},
	{.status = 1073741844, .dos_err = 712, .posix_err = 0, .msg = R"(Checking file system on %wZ)"},
	{.status = 1073741845, .dos_err = 713, .posix_err = 0, .msg = R"({Fatal Application Exit}

%hs)"},
	{.status = 1073741846, .dos_err = 714, .posix_err = 0, .msg = R"(The specified registry key is referenced by a predefined handle.)"},
	{.status = 1073741847, .dos_err = 715, .posix_err = 0, .msg = R"({Page Unlocked}

The page protection of a locked page was changed to 'No Access' and the page was unlocked from memory and from the process.)"},
	{.status = 1073741848, .dos_err = 716, .posix_err = 0, .msg = R"(%hs)"},
	{.status = 1073741849, .dos_err = 717, .posix_err = 0, .msg = R"({Page Locked}

One of the pages to lock was already locked.)"},
	{.status = 1073741850, .dos_err = 718, .posix_err = 0, .msg = R"(Application popup: %1 : %2)"},
	{.status = 1073741851, .dos_err = 719, .posix_err = 0, .msg = R"(STATUS_ALREADY_WIN32)"},
	{.status = 1073741852, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741853, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741854, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741855, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741856, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741857, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741858, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741859, .dos_err = 720, .posix_err = 0, .msg = R"({Machine Type Mismatch}

The image file %hs is valid, but is for a machine type other than the current machine.)"},
	{.status = 1073741860, .dos_err = 721, .posix_err = 0, .msg = R"(A yield execution was performed and no thread was available to run.)"},
	{.status = 1073741861, .dos_err = 722, .posix_err = 0, .msg = R"(The resumable flag to a timer API was ignored.)"},
	{.status = 1073741862, .dos_err = 723, .posix_err = 0, .msg = R"(The arbiter has deferred arbitration of these resources to its parent)"},
	{.status = 1073741863, .dos_err = 724, .posix_err = 0, .msg = R"(The device "%hs" has detected a CardBus card in its slot, but the firmware on this system is not configured to allow the CardBus controller to be run in CardBus mode.

The operating system will currently accept only 16-bit (R2) pc-cards on this controller.)"},
	{.status = 1073741864, .dos_err = 539, .posix_err = 0, .msg = R"(Exception status code used by Win32 x86 emulation subsystem.)"},
	{.status = 1073741865, .dos_err = 725, .posix_err = 0, .msg = R"(The CPUs in this multiprocessor system are not all the same revision level. To use all processors the operating system restricts itself to the features of the least capable processor in the system. Should problems occur with this system, contact the CPU manufacturer to see if this mix of processors is supported.)"},
	{.status = 1073741866, .dos_err = 726, .posix_err = 0, .msg = R"(The system was put into hibernation.)"},
	{.status = 1073741867, .dos_err = 727, .posix_err = 0, .msg = R"(The system was resumed from hibernation.)"},
	{.status = 1073741868, .dos_err = 728, .posix_err = 0, .msg = R"(Windows has detected that the system firmware (BIOS) was updated [previous firmware date = %2, current firmware date %3].)"},
	{.status = 1073741869, .dos_err = 729, .posix_err = 0, .msg = R"(A device driver is leaking locked I/O pages causing system degradation. The system has automatically enabled tracking code in order to try and catch the culprit.)"},
	{.status = 1073741870, .dos_err = 1359, .posix_err = 0, .msg = R"(The ALPC message being canceled has already been retrieved from the queue on the other side.)"},
	{.status = 1073741871, .dos_err = 782, .posix_err = 0, .msg = R"(The system power state is transitioning from %2 to %3.)"},
	{.status = 1073741872, .dos_err = 1359, .posix_err = 0, .msg = R"(The receive operation was successful. Check the ALPC completion list for the received message.)"},
	{.status = 1073741873, .dos_err = 783, .posix_err = 0, .msg = R"(The system power state is transitioning from %2 to %3 but could enter %4.)"},
	{.status = 1073741874, .dos_err = 785, .posix_err = 0, .msg = R"(Access to %1 is monitored by policy rule %2.)"},
	{.status = 1073741875, .dos_err = 787, .posix_err = 0, .msg = R"(A valid hibernation file has been invalidated and should be abandoned.)"},
	{.status = 1073741876, .dos_err = 3221815299, .posix_err = 0, .msg = R"(Business rule scripts are disabled for the calling application.)"},
	{.status = 1073741877, .dos_err = 0, .posix_err = 0, .msg = R"(The specified copy of the requested data was successfully read.)"},
	{.status = 1073741878, .dos_err = 807, .posix_err = 0, .msg = R"({Image Relocated}

An image file was mapped at a different address from the one specified in the image file but fixups will still be automatically performed on the image.)"},
	{.status = 1073741879, .dos_err = 0, .posix_err = 0, .msg = R"(A system patch was successfully loaded but is not applicable to any currently loaded images.)"},
	{.status = 1073742484, .dos_err = 730, .posix_err = 0, .msg = R"(The system has awoken)"},
	{.status = 1073742704, .dos_err = 8364, .posix_err = 0, .msg = R"(The Directory Service is shutting down.)"},
	{.status = 1073743879, .dos_err = 792, .posix_err = 0, .msg = R"(The volume repair could not be performed while it is online.

Please schedule to take the volume offline so that it can be repaired.)"},
	{.status = 1073783108, .dos_err = 15405, .posix_err = 0, .msg = R"(One or more services failed to start during the service startup phase of a run level switch.)"},
	{.status = 2147483649, .dos_err = 2147483649, .posix_err = 0, .msg = R"({EXCEPTION}

Guard Page Exception

A page of memory that marks the end of a data structure, such as a stack or an array, has been accessed.)"},
	{.status = 2147483650, .dos_err = 998, .posix_err = 13, .msg = R"({EXCEPTION}

Alignment Fault

A datatype misalignment was detected in a load or store instruction.)"},
	{.status = 2147483651, .dos_err = 2147483651, .posix_err = 0, .msg = R"({EXCEPTION}

Breakpoint

A breakpoint has been reached.)"},
	{.status = 2147483652, .dos_err = 2147483652, .posix_err = 0, .msg = R"({EXCEPTION}

Single Step

A single step or trace operation has just been completed.)"},
	{.status = 2147483653, .dos_err = 234, .posix_err = 0, .msg = R"({Buffer Overflow}

The data was too large to fit into the specified buffer.)"},
	{.status = 2147483654, .dos_err = 18, .posix_err = 0, .msg = R"({No More Files}

No more files were found which match the file specification.)"},
	{.status = 2147483655, .dos_err = 675, .posix_err = 0, .msg = R"({Kernel Debugger Awakened}

the system debugger was awakened by an interrupt.)"},
	{.status = 2147483656, .dos_err = 317, .posix_err = 0, .msg = R"(Unspecified error)"},
	{.status = 2147483657, .dos_err = 317, .posix_err = 0, .msg = R"(General access denied error)"},
	{.status = 2147483658, .dos_err = 676, .posix_err = 0, .msg = R"({Handles Closed}

Handles to objects have been automatically closed as a result of the requested operation.)"},
	{.status = 2147483659, .dos_err = 1391, .posix_err = 0, .msg = R"({Non-Inheritable ACL}

An access control list (ACL) contains no components that can be inherited.)"},
	{.status = 2147483660, .dos_err = 680, .posix_err = 0, .msg = R"({GUID Substitution}

During the translation of a global identifier (GUID) to a Windows security ID (SID), no administratively-defined GUID prefix was found. A substitute prefix was used, which will not compromise system security. However, this may provide a more restrictive access than intended.)"},
	{.status = 2147483661, .dos_err = 299, .posix_err = 0, .msg = R"({Partial Copy}

Due to protection conflicts not all the requested bytes could be copied.)"},
	{.status = 2147483662, .dos_err = 28, .posix_err = 0, .msg = R"({Out of Paper}

The printer is out of paper.)"},
	{.status = 2147483663, .dos_err = 21, .posix_err = 11, .msg = R"({Device Power Is Off}

The printer power has been turned off.)"},
	{.status = 2147483664, .dos_err = 21, .posix_err = 11, .msg = R"({Device Offline}

The printer has been taken offline.)"},
	{.status = 2147483665, .dos_err = 170, .posix_err = 16, .msg = R"({Device Busy}

The device is currently busy.)"},
	{.status = 2147483666, .dos_err = 259, .posix_err = 0, .msg = R"({No More EAs}

No more extended attributes (EAs) were found for the file.)"},
	{.status = 2147483667, .dos_err = 254, .posix_err = 0, .msg = R"({Illegal EA}

The specified extended attribute (EA) name contains at least one illegal character.)"},
	{.status = 2147483668, .dos_err = 255, .posix_err = 0, .msg = R"({Inconsistent EA List}

The extended attribute (EA) list is inconsistent.)"},
	{.status = 2147483669, .dos_err = 255, .posix_err = 0, .msg = R"({Invalid EA Flag}

An invalid extended attribute (EA) flag was set.)"},
	{.status = 2147483670, .dos_err = 1110, .posix_err = 0, .msg = R"({Verifying Disk}

The media has changed and a verify operation is in progress so no reads or writes may be performed to the device, except those used in the verify operation.)"},
	{.status = 2147483671, .dos_err = 677, .posix_err = 0, .msg = R"({Too Much Information}

The specified access control list (ACL) contained more information than was expected.)"},
	{.status = 2147483672, .dos_err = 678, .posix_err = 0, .msg = R"(This warning level status indicates that the transaction state already exists for the registry sub-tree, but that a transaction commit was previously aborted.

The commit has NOT been completed, but has not been rolled back either (so it may still be committed if desired).)"},
	{.status = 2147483673, .dos_err = 317, .posix_err = 0, .msg = R"(An async operation was not properly started.)"},
	{.status = 2147483674, .dos_err = 259, .posix_err = 0, .msg = R"({No More Entries}

No more entries are available from an enumeration operation.)"},
	{.status = 2147483675, .dos_err = 1101, .posix_err = 0, .msg = R"({Filemark Found}

A filemark was detected.)"},
	{.status = 2147483676, .dos_err = 1110, .posix_err = 0, .msg = R"({Media Changed}

The media may have changed.)"},
	{.status = 2147483677, .dos_err = 1111, .posix_err = 0, .msg = R"({I/O Bus Reset}

An I/O bus reset was detected.)"},
	{.status = 2147483678, .dos_err = 1100, .posix_err = 0, .msg = R"({End of Media}

The end of the media was encountered.)"},
	{.status = 2147483679, .dos_err = 1102, .posix_err = 0, .msg = R"(Beginning of tape or partition has been detected.)"},
	{.status = 2147483680, .dos_err = 679, .posix_err = 0, .msg = R"({Media Changed}

The media may have changed.)"},
	{.status = 2147483681, .dos_err = 1103, .posix_err = 0, .msg = R"(A tape access reached a setmark.)"},
	{.status = 2147483682, .dos_err = 1104, .posix_err = 0, .msg = R"(During a tape access, the end of the data written is reached.)"},
	{.status = 2147483683, .dos_err = 1794, .posix_err = 0, .msg = R"(The redirector is in use and cannot be unloaded.)"},
	{.status = 2147483684, .dos_err = 1811, .posix_err = 0, .msg = R"(The server is in use and cannot be unloaded.)"},
	{.status = 2147483685, .dos_err = 2402, .posix_err = 0, .msg = R"(The specified connection has already been disconnected.)"},
	{.status = 2147483686, .dos_err = 682, .posix_err = 0, .msg = R"(A long jump has been executed.)"},
	{.status = 2147483687, .dos_err = 4340, .posix_err = 0, .msg = R"(A cleaner cartridge is present in the tape library.)"},
	{.status = 2147483688, .dos_err = 683, .posix_err = 0, .msg = R"(The Plug and Play query operation was not successful.)"},
	{.status = 2147483689, .dos_err = 684, .posix_err = 0, .msg = R"(A frame consolidation has been executed.)"},
	{.status = 2147483690, .dos_err = 685, .posix_err = 0, .msg = R"({Registry Hive Recovered}

Registry hive (file):

%hs

was corrupted and it has been recovered. Some data might have been lost.)"},
	{.status = 2147483691, .dos_err = 686, .posix_err = 0, .msg = R"(The application is attempting to run executable code from the module %hs. This may be insecure. An alternative, %hs, is available. Should the application use the secure module %hs?)"},
	{.status = 2147483692, .dos_err = 687, .posix_err = 0, .msg = R"(The application is loading executable code from the module %hs. This is secure, but may be incompatible with previous releases of the operating system. An alternative, %hs, is available. Should the application use the secure module %hs?)"},
	{.status = 2147483693, .dos_err = 681, .posix_err = 0, .msg = R"(The create operation stopped after reaching a symbolic link.)"},
	{.status = 2147483694, .dos_err = 801, .posix_err = 0, .msg = R"(An oplock of the requested level cannot be granted.  An oplock of a lower level may be available.)"},
	{.status = 2147483695, .dos_err = 804, .posix_err = 0, .msg = R"({No ACE Condition}

The specified access control entry (ACE) does not contain a condition.)"},
	{.status = 2147483696, .dos_err = 171, .posix_err = 0, .msg = R"({Support Being Determined}

Device's command support detection is in progress.)"},
	{.status = 2147483697, .dos_err = 197127, .posix_err = 0, .msg = R"(The device needs to be power cycled.)"},
	{.status = 2147483698, .dos_err = 235, .posix_err = 0, .msg = R"(The action requested resulted in no work being done. Error-style clean-up has been performed.)"},
	{.status = 2147483699, .dos_err = 1662, .posix_err = 0, .msg = R"(A return address hijack is being attempted. This is supported by the operating system when user-mode shadow stacks are enabled.)"},
	{.status = 2147484296, .dos_err = 1165, .posix_err = 0, .msg = R"(The device has indicated that cleaning is necessary.)"},
	{.status = 2147484297, .dos_err = 1166, .posix_err = 0, .msg = R"(The device has indicated that its door is open. Further operations require it closed and secured.)"},
	{.status = 2147485699, .dos_err = 6843, .posix_err = 0, .msg = R"(Windows discovered a corruption in the file "%hs".

This file has now been repaired.

Please check if any data in the file was lost because of the corruption.)"},
	{.status = 2147500033, .dos_err = 317, .posix_err = 0, .msg = R"(Not implemented)"},
	{.status = 2147500034, .dos_err = 317, .posix_err = 0, .msg = R"(No such interface supported)"},
	{.status = 2147500035, .dos_err = 317, .posix_err = 0, .msg = R"(Invalid pointer)"},
	{.status = 2147500036, .dos_err = 317, .posix_err = 0, .msg = R"(Operation aborted)"},
	{.status = 2147500037, .dos_err = 317, .posix_err = 0, .msg = R"(Unspecified error)"},
	{.status = 2147500038, .dos_err = 317, .posix_err = 0, .msg = R"(Thread local storage failure)"},
	{.status = 2147500039, .dos_err = 317, .posix_err = 0, .msg = R"(Get shared memory allocator failure)"},
	{.status = 2147500040, .dos_err = 317, .posix_err = 0, .msg = R"(Get memory allocator failure)"},
	{.status = 2147500041, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to initialize class cache)"},
	{.status = 2147500042, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to initialize RPC services)"},
	{.status = 2147500043, .dos_err = 317, .posix_err = 0, .msg = R"(Cannot set thread local storage channel control)"},
	{.status = 2147500044, .dos_err = 317, .posix_err = 0, .msg = R"(Could not allocate thread local storage channel control)"},
	{.status = 2147500045, .dos_err = 317, .posix_err = 0, .msg = R"(The user supplied memory allocator is unacceptable)"},
	{.status = 2147500046, .dos_err = 317, .posix_err = 0, .msg = R"(The OLE service mutex already exists)"},
	{.status = 2147500047, .dos_err = 317, .posix_err = 0, .msg = R"(The OLE service file mapping already exists)"},
	{.status = 2147500048, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to map view of file for OLE service)"},
	{.status = 2147500049, .dos_err = 317, .posix_err = 0, .msg = R"(Failure attempting to launch OLE service)"},
	{.status = 2147500050, .dos_err = 317, .posix_err = 0, .msg = R"(There was an attempt to call CoInitialize a second time while single threaded)"},
	{.status = 2147500051, .dos_err = 317, .posix_err = 0, .msg = R"(A Remote activation was necessary but was not allowed)"},
	{.status = 2147500052, .dos_err = 317, .posix_err = 0, .msg = R"(A Remote activation was necessary but the server name provided was invalid)"},
	{.status = 2147500053, .dos_err = 317, .posix_err = 0, .msg = R"(The class is configured to run as a security id different from the caller)"},
	{.status = 2147500054, .dos_err = 317, .posix_err = 0, .msg = R"(Use of Ole1 services requiring DDE windows is disabled)"},
	{.status = 2147500055, .dos_err = 317, .posix_err = 0, .msg = R"(A RunAs specification must be <domain name>\<user name> or simply <user name>)"},
	{.status = 2147500056, .dos_err = 317, .posix_err = 0, .msg = R"(The server process could not be started. The pathname may be incorrect.)"},
	{.status = 2147500057, .dos_err = 317, .posix_err = 0, .msg = R"(The server process could not be started as the configured identity. The pathname may be incorrect or unavailable.)"},
	{.status = 2147500058, .dos_err = 317, .posix_err = 0, .msg = R"(The server process could not be started because the configured identity is incorrect. Check the username and password.)"},
	{.status = 2147500059, .dos_err = 317, .posix_err = 0, .msg = R"(The client is not allowed to launch this server.)"},
	{.status = 2147500060, .dos_err = 317, .posix_err = 0, .msg = R"(The service providing this server could not be started.)"},
	{.status = 2147500061, .dos_err = 317, .posix_err = 0, .msg = R"(This computer was unable to communicate with the computer providing the server.)"},
	{.status = 2147500062, .dos_err = 317, .posix_err = 0, .msg = R"(The server did not respond after being launched.)"},
	{.status = 2147500063, .dos_err = 317, .posix_err = 0, .msg = R"(The registration information for this server is inconsistent or incomplete.)"},
	{.status = 2147500064, .dos_err = 317, .posix_err = 0, .msg = R"(The registration information for this interface is inconsistent or incomplete.)"},
	{.status = 2147500065, .dos_err = 317, .posix_err = 0, .msg = R"(The operation attempted is not supported.)"},
	{.status = 2147500066, .dos_err = 317, .posix_err = 0, .msg = R"(A dll must be loaded.)"},
	{.status = 2147500067, .dos_err = 317, .posix_err = 0, .msg = R"(A Microsoft Software Installer error was encountered.)"},
	{.status = 2147500068, .dos_err = 317, .posix_err = 0, .msg = R"(The specified activation could not occur in the client context as specified.)"},
	{.status = 2147500069, .dos_err = 317, .posix_err = 0, .msg = R"(Activations on the server are paused.)"},
	{.status = 2147500070, .dos_err = 317, .posix_err = 0, .msg = R"(Activations on the server are not paused.)"},
	{.status = 2147500071, .dos_err = 317, .posix_err = 0, .msg = R"(The component or application containing the component has been disabled.)"},
	{.status = 2147500072, .dos_err = 317, .posix_err = 0, .msg = R"(The common language runtime is not available)"},
	{.status = 2147500073, .dos_err = 317, .posix_err = 0, .msg = R"(The thread-pool rejected the submitted asynchronous work.)"},
	{.status = 2147500074, .dos_err = 317, .posix_err = 0, .msg = R"(The server started, but did not finish initializing in a timely fashion.)"},
	{.status = 2147500075, .dos_err = 317, .posix_err = 0, .msg = R"(Unable to complete the call since there is no COM+ security context inside IObjectControl.Activate.)"},
	{.status = 2147500080, .dos_err = 317, .posix_err = 0, .msg = R"(The provided tracker configuration is invalid)"},
	{.status = 2147500081, .dos_err = 317, .posix_err = 0, .msg = R"(The provided thread pool configuration is invalid)"},
	{.status = 2147500082, .dos_err = 317, .posix_err = 0, .msg = R"(The provided side-by-side configuration is invalid)"},
	{.status = 2147500083, .dos_err = 317, .posix_err = 0, .msg = R"(The server principal name (SPN) obtained during security negotiation is malformed.)"},
	{.status = 2147500084, .dos_err = 317, .posix_err = 0, .msg = R"(The caller failed to revoke a per-apartment registration before apartment shutdown.)"},
	{.status = 2147500085, .dos_err = 317, .posix_err = 0, .msg = R"(The object has been rundown by the stub manager while there are external clients.)"},
	{.status = 2147524903, .dos_err = 15327, .posix_err = 0, .msg = R"(The interrupt requested to be unmasked is not masked.)"},
	{.status = 2147536640, .dos_err = 366, .posix_err = 0, .msg = R"(The cloud file property is possibly corrupt. The on-disk checksum does not match the computed checksum.)"},
	{.status = 2147536644, .dos_err = 365, .posix_err = 0, .msg = R"(The cloud file property is too large.)"},
	{.status = 2147536645, .dos_err = 374, .posix_err = 0, .msg = R"(The maximum number of cloud file properties has been reached.)"},
	{.status = 3221225473, .dos_err = 31, .posix_err = 0, .msg = R"({Operation Failed}

The requested operation was unsuccessful.)"},
	{.status = 3221225474, .dos_err = 1, .posix_err = 40, .msg = R"({Not Implemented}

The requested operation is not implemented.)"},
	{.status = 3221225475, .dos_err = 87, .posix_err = 22, .msg = R"({Invalid Parameter}

The specified information class is not a valid information class for the specified object.)"},
	{.status = 3221225476, .dos_err = 24, .posix_err = 0, .msg = R"(The specified information record length does not match the length required for the specified information class.)"},
	{.status = 3221225477, .dos_err = 998, .posix_err = 13, .msg = R"(The instruction at 0x%p referenced memory at 0x%p. The memory could not be %s.)"},
	{.status = 3221225478, .dos_err = 999, .posix_err = 0, .msg = R"(The instruction at 0x%p referenced memory at 0x%p. The required data was not placed into memory because of an I/O error status of 0x%x.)"},
	{.status = 3221225479, .dos_err = 1454, .posix_err = 0, .msg = R"(The pagefile quota for the process has been exhausted.)"},
	{.status = 3221225480, .dos_err = 6, .posix_err = 22, .msg = R"(An invalid HANDLE was specified.)"},
	{.status = 3221225481, .dos_err = 1001, .posix_err = 0, .msg = R"(An invalid initial stack was specified in a call to NtCreateThread.)"},
	{.status = 3221225482, .dos_err = 193, .posix_err = 0, .msg = R"(An invalid initial start address was specified in a call to NtCreateThread.)"},
	{.status = 3221225483, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid Client ID was specified.)"},
	{.status = 3221225484, .dos_err = 541, .posix_err = 0, .msg = R"(An attempt was made to cancel or set a timer that has an associated APC and the subject thread is not the thread that originally set the timer with an associated APC routine.)"},
	{.status = 3221225485, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function.)"},
	{.status = 3221225486, .dos_err = 433, .posix_err = 0, .msg = R"(A device which does not exist was specified.)"},
	{.status = 3221225487, .dos_err = 2, .posix_err = 2, .msg = R"({File Not Found}

The file %hs does not exist.)"},
	{.status = 3221225488, .dos_err = 1, .posix_err = 40, .msg = R"(The specified request is not a valid operation for the target device.)"},
	{.status = 3221225489, .dos_err = 38, .posix_err = 0, .msg = R"(The end-of-file marker has been reached. There is no valid data in the file beyond this marker.)"},
	{.status = 3221225490, .dos_err = 34, .posix_err = 0, .msg = R"({Wrong Volume}

The wrong volume is in the drive.

Please insert volume %hs into drive %hs.)"},
	{.status = 3221225491, .dos_err = 21, .posix_err = 11, .msg = R"({No Disk}

There is no disk in the drive.

Please insert a disk into drive %hs.)"},
	{.status = 3221225492, .dos_err = 1785, .posix_err = 0, .msg = R"({Unknown Disk Format}

The disk in drive %hs is not formatted properly.

Please check the disk, and reformat if necessary.)"},
	{.status = 3221225493, .dos_err = 27, .posix_err = 0, .msg = R"({Sector Not Found}

The specified sector does not exist.)"},
	{.status = 3221225494, .dos_err = 234, .posix_err = 0, .msg = R"({Still Busy}

The specified I/O request packet (IRP) cannot be disposed of because the I/O operation is not complete.)"},
	{.status = 3221225495, .dos_err = 8, .posix_err = 12, .msg = R"({Not Enough Quota}

Not enough virtual memory or paging file quota is available to complete the specified operation.)"},
	{.status = 3221225496, .dos_err = 487, .posix_err = 0, .msg = R"({Conflicting Address Range}

The specified address range conflicts with the address space.)"},
	{.status = 3221225497, .dos_err = 487, .posix_err = 0, .msg = R"(Address range to unmap is not a mapped view.)"},
	{.status = 3221225498, .dos_err = 87, .posix_err = 22, .msg = R"(Virtual memory cannot be freed.)"},
	{.status = 3221225499, .dos_err = 87, .posix_err = 22, .msg = R"(Specified section cannot be deleted.)"},
	{.status = 3221225500, .dos_err = 1, .posix_err = 40, .msg = R"(An invalid system service was specified in a system service call.)"},
	{.status = 3221225501, .dos_err = 3221225501, .posix_err = 0, .msg = R"({EXCEPTION}

Illegal Instruction

An attempt was made to execute an illegal instruction.)"},
	{.status = 3221225502, .dos_err = 5, .posix_err = 13, .msg = R"({Invalid Lock Sequence}

An attempt was made to execute an invalid lock sequence.)"},
	{.status = 3221225503, .dos_err = 5, .posix_err = 13, .msg = R"({Invalid Mapping}

An attempt was made to create a view for a section which is bigger than the section.)"},
	{.status = 3221225504, .dos_err = 193, .posix_err = 0, .msg = R"({Bad File}

The attributes of the specified mapping file for a section of memory cannot be read.)"},
	{.status = 3221225505, .dos_err = 5, .posix_err = 13, .msg = R"({Already Committed}

The specified address range is already committed.)"},
	{.status = 3221225506, .dos_err = 5, .posix_err = 13, .msg = R"({Access Denied}

A process has requested access to an object, but has not been granted those access rights.)"},
	{.status = 3221225507, .dos_err = 122, .posix_err = 0, .msg = R"({Buffer Too Small}

The buffer is too small to contain the entry. No information has been written to the buffer.)"},
	{.status = 3221225508, .dos_err = 6, .posix_err = 22, .msg = R"({Wrong Type}

There is a mismatch between the type of object required by the requested operation and the type of object that is specified in the request.)"},
	{.status = 3221225509, .dos_err = 3221225509, .posix_err = 0, .msg = R"({EXCEPTION}

Cannot Continue

Windows cannot continue from this exception.)"},
	{.status = 3221225510, .dos_err = 3221225510, .posix_err = 0, .msg = R"(An invalid exception disposition was returned by an exception handler.)"},
	{.status = 3221225511, .dos_err = 542, .posix_err = 0, .msg = R"(Unwind exception code.)"},
	{.status = 3221225512, .dos_err = 543, .posix_err = 0, .msg = R"(An invalid or unaligned stack was encountered during an unwind operation.)"},
	{.status = 3221225513, .dos_err = 544, .posix_err = 0, .msg = R"(An invalid unwind target was encountered during an unwind operation.)"},
	{.status = 3221225514, .dos_err = 158, .posix_err = 0, .msg = R"(An attempt was made to unlock a page of memory which was not locked.)"},
	{.status = 3221225515, .dos_err = 3221225515, .posix_err = 0, .msg = R"(Device parity error on I/O operation.)"},
	{.status = 3221225516, .dos_err = 487, .posix_err = 0, .msg = R"(An attempt was made to decommit uncommitted virtual memory.)"},
	{.status = 3221225517, .dos_err = 487, .posix_err = 0, .msg = R"(An attempt was made to change the attributes on memory that has not been committed.)"},
	{.status = 3221225518, .dos_err = 545, .posix_err = 0, .msg = R"(Invalid Object Attributes specified to NtCreatePort or invalid Port Attributes specified to NtConnectPort)"},
	{.status = 3221225519, .dos_err = 546, .posix_err = 0, .msg = R"(Length of message passed to NtRequestPort or NtRequestWaitReplyPort was longer than the maximum message allowed by the port.)"},
	{.status = 3221225520, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid combination of parameters was specified.)"},
	{.status = 3221225521, .dos_err = 547, .posix_err = 0, .msg = R"(An attempt was made to lower a quota limit below the current usage.)"},
	{.status = 3221225522, .dos_err = 1393, .posix_err = 0, .msg = R"({Corrupt Disk}

The file system structure on the disk is corrupt and unusable.

Please run the Chkdsk utility on the volume %hs.)"},
	{.status = 3221225523, .dos_err = 123, .posix_err = 2, .msg = R"(Object Name invalid.)"},
	{.status = 3221225524, .dos_err = 2, .posix_err = 2, .msg = R"(Object Name not found.)"},
	{.status = 3221225525, .dos_err = 183, .posix_err = 17, .msg = R"(Object Name already exists.)"},
	{.status = 3221225526, .dos_err = 1834, .posix_err = 0, .msg = R"(A port with the 'do not disturb' flag set attempted to send a message to a port in a suspended process.

The process was not woken, and the message was not delivered.)"},
	{.status = 3221225527, .dos_err = 6, .posix_err = 22, .msg = R"(Attempt to send a message to a disconnected communication port.)"},
	{.status = 3221225528, .dos_err = 548, .posix_err = 0, .msg = R"(An attempt was made to attach to a device that was already attached to another device.)"},
	{.status = 3221225529, .dos_err = 161, .posix_err = 0, .msg = R"(Object Path Component was not a directory object.)"},
	{.status = 3221225530, .dos_err = 3, .posix_err = 2, .msg = R"({Path Not Found}

The path %hs does not exist.)"},
	{.status = 3221225531, .dos_err = 161, .posix_err = 0, .msg = R"(Object Path Component was not a directory object.)"},
	{.status = 3221225532, .dos_err = 1117, .posix_err = 0, .msg = R"({Data Overrun}

A data overrun error occurred.)"},
	{.status = 3221225533, .dos_err = 1117, .posix_err = 0, .msg = R"({Data Late}

A data late error occurred.)"},
	{.status = 3221225534, .dos_err = 23, .posix_err = 0, .msg = R"({Data Error}

An error in reading or writing data occurred.)"},
	{.status = 3221225535, .dos_err = 23, .posix_err = 0, .msg = R"({Bad CRC}

A cyclic redundancy check (CRC) checksum error occurred.)"},
	{.status = 3221225536, .dos_err = 8, .posix_err = 12, .msg = R"({Section Too Large}

The specified section is too big to map the file.)"},
	{.status = 3221225537, .dos_err = 5, .posix_err = 13, .msg = R"(The NtConnectPort request is refused.)"},
	{.status = 3221225538, .dos_err = 6, .posix_err = 22, .msg = R"(The type of port handle is invalid for the operation requested.)"},
	{.status = 3221225539, .dos_err = 32, .posix_err = 13, .msg = R"(A file cannot be opened because the share access flags are incompatible.)"},
	{.status = 3221225540, .dos_err = 1816, .posix_err = 0, .msg = R"(Insufficient quota exists to complete the operation)"},
	{.status = 3221225541, .dos_err = 87, .posix_err = 22, .msg = R"(The specified page protection was not valid.)"},
	{.status = 3221225542, .dos_err = 288, .posix_err = 0, .msg = R"(An attempt to release a mutant object was made by a thread that was not the owner of the mutant object.)"},
	{.status = 3221225543, .dos_err = 298, .posix_err = 0, .msg = R"(An attempt was made to release a semaphore such that its maximum count would have been exceeded.)"},
	{.status = 3221225544, .dos_err = 87, .posix_err = 22, .msg = R"(An attempt to set a process's DebugPort or ExceptionPort was made, but a port already exists in the process or an attempt to set a file's CompletionPort made, but a port was already set in the file or an attempt to set an ALPC port's associated completion port was made, but it is already set.)"},
	{.status = 3221225545, .dos_err = 87, .posix_err = 22, .msg = R"(An attempt was made to query image information on a section which does not map an image.)"},
	{.status = 3221225546, .dos_err = 156, .posix_err = 0, .msg = R"(An attempt was made to suspend a thread whose suspend count was at its maximum.)"},
	{.status = 3221225547, .dos_err = 5, .posix_err = 13, .msg = R"(An attempt was made to access a thread that has begun termination.)"},
	{.status = 3221225548, .dos_err = 87, .posix_err = 22, .msg = R"(An attempt was made to set the working set limit to an invalid value (minimum greater than maximum, etc).)"},
	{.status = 3221225549, .dos_err = 87, .posix_err = 22, .msg = R"(A section was created to map a file which is not compatible to an already existing section which maps the same file.)"},
	{.status = 3221225550, .dos_err = 87, .posix_err = 22, .msg = R"(A view to a section specifies a protection which is incompatible with the initial view's protection.)"},
	{.status = 3221225551, .dos_err = 282, .posix_err = 0, .msg = R"(An operation involving EAs failed because the file system does not support EAs.)"},
	{.status = 3221225552, .dos_err = 255, .posix_err = 0, .msg = R"(An EA operation failed because EA set is too large.)"},
	{.status = 3221225553, .dos_err = 1392, .posix_err = 0, .msg = R"(An EA operation failed because the name or EA index is invalid.)"},
	{.status = 3221225554, .dos_err = 1392, .posix_err = 0, .msg = R"(The file for which EAs were requested has no EAs.)"},
	{.status = 3221225555, .dos_err = 1392, .posix_err = 0, .msg = R"(The EA is corrupt and non-readable.)"},
	{.status = 3221225556, .dos_err = 33, .posix_err = 39, .msg = R"(A requested read/write cannot be granted due to a conflicting file lock.)"},
	{.status = 3221225557, .dos_err = 33, .posix_err = 39, .msg = R"(A requested file lock cannot be granted due to other existing locks.)"},
	{.status = 3221225558, .dos_err = 5, .posix_err = 13, .msg = R"(A non close operation has been requested of a file object with a delete pending.)"},
	{.status = 3221225559, .dos_err = 50, .posix_err = 129, .msg = R"(An attempt was made to set the control attribute on a file. This attribute is not supported in the target file system.)"},
	{.status = 3221225560, .dos_err = 1305, .posix_err = 0, .msg = R"(Indicates a revision number encountered or specified is not one known by the service. It may be a more recent revision than the service is aware of.)"},
	{.status = 3221225561, .dos_err = 1306, .posix_err = 0, .msg = R"(Indicates two revision levels are incompatible.)"},
	{.status = 3221225562, .dos_err = 1307, .posix_err = 0, .msg = R"(Indicates a particular Security ID may not be assigned as the owner of an object.)"},
	{.status = 3221225563, .dos_err = 1308, .posix_err = 0, .msg = R"(Indicates a particular Security ID may not be assigned as the primary group of an object.)"},
	{.status = 3221225564, .dos_err = 1309, .posix_err = 0, .msg = R"(An attempt has been made to operate on an impersonation token by a thread that is not currently impersonating a client.)"},
	{.status = 3221225565, .dos_err = 1310, .posix_err = 0, .msg = R"(A mandatory group may not be disabled.)"},
	{.status = 3221225566, .dos_err = 1311, .posix_err = 0, .msg = R"(We can't sign you in with this credential because your domain isn't available. Make sure your device is connected to your organization's network and try again. If you previously signed in on this device with another credential, you can sign in with that credential.)"},
	{.status = 3221225567, .dos_err = 1312, .posix_err = 0, .msg = R"(A specified logon session does not exist. It may already have been terminated.)"},
	{.status = 3221225568, .dos_err = 1313, .posix_err = 0, .msg = R"(A specified privilege does not exist.)"},
	{.status = 3221225569, .dos_err = 1314, .posix_err = 0, .msg = R"(A required privilege is not held by the client.)"},
	{.status = 3221225570, .dos_err = 1315, .posix_err = 0, .msg = R"(The name provided is not a properly formed account name.)"},
	{.status = 3221225571, .dos_err = 1316, .posix_err = 0, .msg = R"(The specified account already exists.)"},
	{.status = 3221225572, .dos_err = 1317, .posix_err = 0, .msg = R"(The specified account does not exist.)"},
	{.status = 3221225573, .dos_err = 1318, .posix_err = 0, .msg = R"(The specified group already exists.)"},
	{.status = 3221225574, .dos_err = 1319, .posix_err = 0, .msg = R"(The specified group does not exist.)"},
	{.status = 3221225575, .dos_err = 1320, .posix_err = 0, .msg = R"(The specified user account is already in the specified group account. Also used to indicate a group cannot be deleted because it contains a member.)"},
	{.status = 3221225576, .dos_err = 1321, .posix_err = 0, .msg = R"(The specified user account is not a member of the specified group account.)"},
	{.status = 3221225577, .dos_err = 1322, .posix_err = 0, .msg = R"(Indicates the requested operation would disable, delete or could prevent logon for an administration account.

This is not allowed to prevent creating a situation in which the system cannot be administrated.)"},
	{.status = 3221225578, .dos_err = 86, .posix_err = 0, .msg = R"(When trying to update a password, this return status indicates that the value provided as the current password is not correct.)"},
	{.status = 3221225579, .dos_err = 1324, .posix_err = 0, .msg = R"(When trying to update a password, this return status indicates that the value provided for the new password contains values that are not allowed in passwords.)"},
	{.status = 3221225580, .dos_err = 1325, .posix_err = 0, .msg = R"(When trying to update a password, this status indicates that some password update rule has been violated. For example, the password may not meet length criteria.)"},
	{.status = 3221225581, .dos_err = 1326, .posix_err = 0, .msg = R"(The attempted logon is invalid. This is either due to a bad username or authentication information.)"},
	{.status = 3221225582, .dos_err = 1327, .posix_err = 0, .msg = R"(Indicates a referenced user name and authentication information are valid, but some user account restriction has prevented successful authentication (such as time-of-day restrictions).)"},
	{.status = 3221225583, .dos_err = 1328, .posix_err = 0, .msg = R"(The user account has time restrictions and may not be logged onto at this time.)"},
	{.status = 3221225584, .dos_err = 1329, .posix_err = 0, .msg = R"(The user account is restricted such that it may not be used to log on from the source workstation.)"},
	{.status = 3221225585, .dos_err = 1330, .posix_err = 0, .msg = R"(The user account's password has expired.)"},
	{.status = 3221225586, .dos_err = 1331, .posix_err = 0, .msg = R"(The referenced account is currently disabled and may not be logged on to.)"},
	{.status = 3221225587, .dos_err = 1332, .posix_err = 0, .msg = R"(None of the information to be translated has been translated.)"},
	{.status = 3221225588, .dos_err = 1333, .posix_err = 0, .msg = R"(The number of LUIDs requested may not be allocated with a single allocation.)"},
	{.status = 3221225589, .dos_err = 1334, .posix_err = 0, .msg = R"(Indicates there are no more LUIDs to allocate.)"},
	{.status = 3221225590, .dos_err = 1335, .posix_err = 0, .msg = R"(Indicates the sub-authority value is invalid for the particular use.)"},
	{.status = 3221225591, .dos_err = 1336, .posix_err = 0, .msg = R"(Indicates the ACL structure is not valid.)"},
	{.status = 3221225592, .dos_err = 1337, .posix_err = 0, .msg = R"(Indicates the SID structure is not valid.)"},
	{.status = 3221225593, .dos_err = 1338, .posix_err = 0, .msg = R"(Indicates the SECURITY_DESCRIPTOR structure is not valid.)"},
	{.status = 3221225594, .dos_err = 127, .posix_err = 0, .msg = R"(Indicates the specified procedure address cannot be found in the DLL.)"},
	{.status = 3221225595, .dos_err = 193, .posix_err = 0, .msg = R"({Bad Image}

%hs is either not designed to run on Windows or it contains an error. Try installing the program again using the original installation media or contact your system administrator or the software vendor for support. Error status 0x)"},
	{.status = 3221225596, .dos_err = 1008, .posix_err = 0, .msg = R"(An attempt was made to reference a token that doesn't exist.

This is typically done by referencing the token associated with a thread when the thread is not impersonating a client.)"},
	{.status = 3221225597, .dos_err = 1340, .posix_err = 0, .msg = R"(Indicates that an attempt to build either an inherited ACL or ACE was not successful.

This can be caused by a number of things. One of the more probable causes is the replacement of a CreatorId with an SID that didn't fit into the ACE or ACL.)"},
	{.status = 3221225598, .dos_err = 158, .posix_err = 0, .msg = R"(The range specified in NtUnlockFile was not locked.)"},
	{.status = 3221225599, .dos_err = 112, .posix_err = 28, .msg = R"(An operation failed because the disk was full.

If this is a thinly provisioned volume the physical storage backing this volume has been exhausted.)"},
	{.status = 3221225600, .dos_err = 1341, .posix_err = 0, .msg = R"(The GUID allocation server is [already] disabled at the moment.)"},
	{.status = 3221225601, .dos_err = 1342, .posix_err = 0, .msg = R"(The GUID allocation server is [already] enabled at the moment.)"},
	{.status = 3221225602, .dos_err = 68, .posix_err = 0, .msg = R"(Too many GUIDs were requested from the allocation server at once.)"},
	{.status = 3221225603, .dos_err = 259, .posix_err = 0, .msg = R"(The GUIDs could not be allocated because the Authority Agent was exhausted.)"},
	{.status = 3221225604, .dos_err = 1343, .posix_err = 0, .msg = R"(The value provided was an invalid value for an identifier authority.)"},
	{.status = 3221225605, .dos_err = 259, .posix_err = 0, .msg = R"(There are no more authority agent values available for the given identifier authority value.)"},
	{.status = 3221225606, .dos_err = 154, .posix_err = 0, .msg = R"(An invalid volume label has been specified.)"},
	{.status = 3221225607, .dos_err = 14, .posix_err = 12, .msg = R"(A mapped section could not be extended.)"},
	{.status = 3221225608, .dos_err = 487, .posix_err = 0, .msg = R"(Specified section to flush does not map a data file.)"},
	{.status = 3221225609, .dos_err = 1812, .posix_err = 0, .msg = R"(Indicates the specified image file did not contain a resource section.)"},
	{.status = 3221225610, .dos_err = 1813, .posix_err = 0, .msg = R"(Indicates the specified resource type cannot be found in the image file.)"},
	{.status = 3221225611, .dos_err = 1814, .posix_err = 0, .msg = R"(Indicates the specified resource name cannot be found in the image file.)"},
	{.status = 3221225612, .dos_err = 3221225612, .posix_err = 0, .msg = R"({EXCEPTION}

Array bounds exceeded.)"},
	{.status = 3221225613, .dos_err = 3221225613, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point denormal operand.)"},
	{.status = 3221225614, .dos_err = 3221225614, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point division by zero.)"},
	{.status = 3221225615, .dos_err = 3221225615, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point inexact result.)"},
	{.status = 3221225616, .dos_err = 3221225616, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point invalid operation.)"},
	{.status = 3221225617, .dos_err = 3221225617, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point overflow.)"},
	{.status = 3221225618, .dos_err = 3221225618, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point stack check.)"},
	{.status = 3221225619, .dos_err = 3221225619, .posix_err = 0, .msg = R"({EXCEPTION}

Floating-point underflow.)"},
	{.status = 3221225620, .dos_err = 3221225620, .posix_err = 0, .msg = R"({EXCEPTION}

Integer division by zero.)"},
	{.status = 3221225621, .dos_err = 534, .posix_err = 0, .msg = R"({EXCEPTION}

Integer overflow.)"},
	{.status = 3221225622, .dos_err = 3221225622, .posix_err = 0, .msg = R"({EXCEPTION}

Privileged instruction.)"},
	{.status = 3221225623, .dos_err = 8, .posix_err = 12, .msg = R"(An attempt was made to install more paging files than the system supports.)"},
	{.status = 3221225624, .dos_err = 1006, .posix_err = 0, .msg = R"(The volume for a file has been externally altered such that the opened file is no longer valid.)"},
	{.status = 3221225625, .dos_err = 1344, .posix_err = 0, .msg = R"(When a block of memory is allotted for future updates, such as the memory allocated to hold discretionary access control and primary group information, successive updates may exceed the amount of memory originally allotted.

Since quota may already have been charged to several processes which have handles to the object, it is not reasonable to alter the size of the allocated memory.

Instead, a request that requires more memory than has been allotted must fail and the STATUS_ALLOTED_SPACE_EXCEEDED error returned.)"},
	{.status = 3221225626, .dos_err = 1450, .posix_err = 0, .msg = R"(Insufficient system resources exist to complete the API.)"},
	{.status = 3221225627, .dos_err = 3, .posix_err = 2, .msg = R"(An attempt has been made to open a DFS exit path control file.)"},
	{.status = 3221225628, .dos_err = 23, .posix_err = 0, .msg = R"(STATUS_DEVICE_DATA_ERROR)"},
	{.status = 3221225629, .dos_err = 1167, .posix_err = 0, .msg = R"(STATUS_DEVICE_NOT_CONNECTED)"},
	{.status = 3221225630, .dos_err = 21, .posix_err = 11, .msg = R"(STATUS_DEVICE_POWER_FAILURE)"},
	{.status = 3221225631, .dos_err = 487, .posix_err = 0, .msg = R"(Virtual memory cannot be freed as base address is not the base of the region and a region size of zero was specified.)"},
	{.status = 3221225632, .dos_err = 487, .posix_err = 0, .msg = R"(An attempt was made to free virtual memory which is not allocated.)"},
	{.status = 3221225633, .dos_err = 1453, .posix_err = 0, .msg = R"(The working set is not big enough to allow the requested pages to be locked.)"},
	{.status = 3221225634, .dos_err = 19, .posix_err = 13, .msg = R"({Write Protect Error}

The disk cannot be written to because it is write protected. Please remove the write protection from the volume %hs in drive %hs.)"},
	{.status = 3221225635, .dos_err = 21, .posix_err = 11, .msg = R"({Drive Not Ready}

The drive is not ready for use; its door may be open. Please check drive %hs and make sure that a disk is inserted and that the drive door is closed.)"},
	{.status = 3221225636, .dos_err = 1345, .posix_err = 0, .msg = R"(The specified attributes are invalid, or incompatible with the attributes for the group as a whole.)"},
	{.status = 3221225637, .dos_err = 1346, .posix_err = 0, .msg = R"(A specified impersonation level is invalid.

Also used to indicate a required impersonation level was not provided.)"},
	{.status = 3221225638, .dos_err = 1347, .posix_err = 0, .msg = R"(An attempt was made to open an Anonymous level token.

Anonymous tokens may not be opened.)"},
	{.status = 3221225639, .dos_err = 1348, .posix_err = 0, .msg = R"(The validation information class requested was invalid.)"},
	{.status = 3221225640, .dos_err = 1349, .posix_err = 0, .msg = R"(The type of a token object is inappropriate for its attempted use.)"},
	{.status = 3221225641, .dos_err = 87, .posix_err = 22, .msg = R"(The type of a token object is inappropriate for its attempted use.)"},
	{.status = 3221225642, .dos_err = 549, .posix_err = 0, .msg = R"(An attempt was made to execute an instruction at an unaligned address and the host system does not support unaligned instruction references.)"},
	{.status = 3221225643, .dos_err = 231, .posix_err = 0, .msg = R"(The maximum named pipe instance count has been reached.)"},
	{.status = 3221225644, .dos_err = 231, .posix_err = 0, .msg = R"(An instance of a named pipe cannot be found in the listening state.)"},
	{.status = 3221225645, .dos_err = 230, .posix_err = 0, .msg = R"(The named pipe is not in the connected or closing state.)"},
	{.status = 3221225646, .dos_err = 231, .posix_err = 0, .msg = R"(The specified pipe is set to complete operations and there are current I/O operations queued so it cannot be changed to queue operations.)"},
	{.status = 3221225647, .dos_err = 1, .posix_err = 40, .msg = R"(The specified handle is not open to the server end of the named pipe.)"},
	{.status = 3221225648, .dos_err = 233, .posix_err = 0, .msg = R"(The specified named pipe is in the disconnected state.)"},
	{.status = 3221225649, .dos_err = 232, .posix_err = 0, .msg = R"(The specified named pipe is in the closing state.)"},
	{.status = 3221225650, .dos_err = 535, .posix_err = 0, .msg = R"(The specified named pipe is in the connected state.)"},
	{.status = 3221225651, .dos_err = 536, .posix_err = 0, .msg = R"(The specified named pipe is in the listening state.)"},
	{.status = 3221225652, .dos_err = 230, .posix_err = 0, .msg = R"(The specified named pipe is not in message mode.)"},
	{.status = 3221225653, .dos_err = 121, .posix_err = 138, .msg = R"({Device Timeout}

The specified I/O operation on %hs was not completed before the time-out period expired.)"},
	{.status = 3221225654, .dos_err = 38, .posix_err = 0, .msg = R"(The specified file has been closed by another process.)"},
	{.status = 3221225655, .dos_err = 550, .posix_err = 0, .msg = R"(Profiling not started.)"},
	{.status = 3221225656, .dos_err = 551, .posix_err = 0, .msg = R"(Profiling not stopped.)"},
	{.status = 3221225657, .dos_err = 552, .posix_err = 0, .msg = R"(The passed ACL did not contain the minimum required information.)"},
	{.status = 3221225658, .dos_err = 5, .posix_err = 13, .msg = R"(The file that was specified as a target is a directory and the caller specified that it could be anything but a directory.)"},
	{.status = 3221225659, .dos_err = 50, .posix_err = 129, .msg = R"(The request is not supported.)"},
	{.status = 3221225660, .dos_err = 51, .posix_err = 0, .msg = R"(This remote computer is not listening.)"},
	{.status = 3221225661, .dos_err = 52, .posix_err = 0, .msg = R"(A duplicate name exists on the network.)"},
	{.status = 3221225662, .dos_err = 53, .posix_err = 2, .msg = R"(The network path cannot be located.)"},
	{.status = 3221225663, .dos_err = 54, .posix_err = 0, .msg = R"(The network is busy.)"},
	{.status = 3221225664, .dos_err = 55, .posix_err = 19, .msg = R"(This device does not exist.)"},
	{.status = 3221225665, .dos_err = 56, .posix_err = 0, .msg = R"(The network BIOS command limit has been reached.)"},
	{.status = 3221225666, .dos_err = 57, .posix_err = 0, .msg = R"(An I/O adapter hardware error has occurred.)"},
	{.status = 3221225667, .dos_err = 58, .posix_err = 0, .msg = R"(The network responded incorrectly.)"},
	{.status = 3221225668, .dos_err = 59, .posix_err = 0, .msg = R"(An unexpected network error occurred.)"},
	{.status = 3221225669, .dos_err = 60, .posix_err = 0, .msg = R"(The remote adapter is not compatible.)"},
	{.status = 3221225670, .dos_err = 61, .posix_err = 0, .msg = R"(The printer queue is full.)"},
	{.status = 3221225671, .dos_err = 62, .posix_err = 0, .msg = R"(Space to store the file waiting to be printed is not available on the server.)"},
	{.status = 3221225672, .dos_err = 63, .posix_err = 0, .msg = R"(The requested print file has been canceled.)"},
	{.status = 3221225673, .dos_err = 64, .posix_err = 0, .msg = R"(The network name was deleted.)"},
	{.status = 3221225674, .dos_err = 65, .posix_err = 0, .msg = R"(Network access is denied.)"},
	{.status = 3221225675, .dos_err = 66, .posix_err = 0, .msg = R"({Incorrect Network Resource Type}

The specified device type (LPT, for example) conflicts with the actual device type on the remote resource.)"},
	{.status = 3221225676, .dos_err = 67, .posix_err = 2, .msg = R"({Network Name Not Found}

The specified share name cannot be found on the remote server.)"},
	{.status = 3221225677, .dos_err = 68, .posix_err = 0, .msg = R"(The name limit for the local computer network adapter card was exceeded.)"},
	{.status = 3221225678, .dos_err = 69, .posix_err = 0, .msg = R"(The network BIOS session limit was exceeded.)"},
	{.status = 3221225679, .dos_err = 70, .posix_err = 0, .msg = R"(File sharing has been temporarily paused.)"},
	{.status = 3221225680, .dos_err = 71, .posix_err = 0, .msg = R"(No more connections can be made to this remote computer at this time because there are already as many connections as the computer can accept.)"},
	{.status = 3221225681, .dos_err = 72, .posix_err = 0, .msg = R"(Print or disk redirection is temporarily paused.)"},
	{.status = 3221225682, .dos_err = 88, .posix_err = 0, .msg = R"(A network data fault occurred.)"},
	{.status = 3221225683, .dos_err = 553, .posix_err = 0, .msg = R"(The number of active profiling objects is at the maximum and no more may be started.)"},
	{.status = 3221225684, .dos_err = 17, .posix_err = 18, .msg = R"({Incorrect Volume}

The target file of a rename request is located on a different device than the source of the rename request.)"},
	{.status = 3221225685, .dos_err = 5, .posix_err = 13, .msg = R"(The file specified has been renamed and thus cannot be modified.)"},
	{.status = 3221225686, .dos_err = 240, .posix_err = 0, .msg = R"({Network Request Timeout}

The session with a remote server has been disconnected because the time-out interval for a request has expired.)"},
	{.status = 3221225687, .dos_err = 1350, .posix_err = 0, .msg = R"(Indicates an attempt was made to operate on the security of an object that does not have security associated with it.)"},
	{.status = 3221225688, .dos_err = 554, .posix_err = 0, .msg = R"(Used to indicate that an operation cannot continue without blocking for I/O.)"},
	{.status = 3221225689, .dos_err = 232, .posix_err = 0, .msg = R"(Used to indicate that a read operation was done on an empty pipe.)"},
	{.status = 3221225690, .dos_err = 1351, .posix_err = 0, .msg = R"(Configuration information could not be read from the domain controller, either because the machine is unavailable, or access has been denied.)"},
	{.status = 3221225691, .dos_err = 555, .posix_err = 0, .msg = R"(Indicates that a thread attempted to terminate itself by default (called NtTerminateThread with NULL) and it was the last thread in the current process.)"},
	{.status = 3221225692, .dos_err = 1352, .posix_err = 0, .msg = R"(Indicates the Sam Server was in the wrong state to perform the desired operation.)"},
	{.status = 3221225693, .dos_err = 1353, .posix_err = 0, .msg = R"(Indicates the Domain was in the wrong state to perform the desired operation.)"},
	{.status = 3221225694, .dos_err = 1354, .posix_err = 0, .msg = R"(This operation is only allowed for the Primary Domain Controller of the domain.)"},
	{.status = 3221225695, .dos_err = 1355, .posix_err = 0, .msg = R"(The specified Domain did not exist.)"},
	{.status = 3221225696, .dos_err = 1356, .posix_err = 0, .msg = R"(The specified Domain already exists.)"},
	{.status = 3221225697, .dos_err = 1357, .posix_err = 0, .msg = R"(An attempt was made to exceed the limit on the number of domains per server for this release.)"},
	{.status = 3221225698, .dos_err = 300, .posix_err = 0, .msg = R"(Error status returned when oplock request is denied.)"},
	{.status = 3221225699, .dos_err = 301, .posix_err = 0, .msg = R"(Error status returned when an invalid oplock acknowledgment is received by a file system.)"},
	{.status = 3221225700, .dos_err = 1358, .posix_err = 0, .msg = R"(This error indicates that the requested operation cannot be completed due to a catastrophic media failure or on-disk data structure corruption.)"},
	{.status = 3221225701, .dos_err = 1359, .posix_err = 0, .msg = R"(An internal error occurred.)"},
	{.status = 3221225702, .dos_err = 1360, .posix_err = 0, .msg = R"(Indicates generic access types were contained in an access mask which should already be mapped to non-generic access types.)"},
	{.status = 3221225703, .dos_err = 1361, .posix_err = 0, .msg = R"(Indicates a security descriptor is not in the necessary format (absolute or self-relative).)"},
	{.status = 3221225704, .dos_err = 1784, .posix_err = 0, .msg = R"(An access to a user buffer failed at an "expected" point in time. This code is defined since the caller does not want to accept STATUS_ACCESS_VIOLATION in its filter.)"},
	{.status = 3221225705, .dos_err = 1117, .posix_err = 0, .msg = R"(If an I/O error is returned which is not defined in the standard FsRtl filter, it is converted to the following error which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 3221225706, .dos_err = 556, .posix_err = 0, .msg = R"(If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 3221225707, .dos_err = 557, .posix_err = 0, .msg = R"(If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 3221225708, .dos_err = 558, .posix_err = 0, .msg = R"(If an MM error is returned which is not defined in the standard FsRtl filter, it is converted to one of the following errors which is guaranteed to be in the filter. In this case information is lost, however, the filter correctly handles the exception.)"},
	{.status = 3221225709, .dos_err = 1362, .posix_err = 0, .msg = R"(The requested action is restricted for use by logon processes only. The calling process has not registered as a logon process.)"},
	{.status = 3221225710, .dos_err = 1363, .posix_err = 0, .msg = R"(An attempt has been made to start a new session manager or LSA logon session with an ID that is already in use.)"},
	{.status = 3221225711, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the first argument.)"},
	{.status = 3221225712, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the second argument.)"},
	{.status = 3221225713, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the third argument.)"},
	{.status = 3221225714, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the fourth argument.)"},
	{.status = 3221225715, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the fifth argument.)"},
	{.status = 3221225716, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the sixth argument.)"},
	{.status = 3221225717, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the seventh argument.)"},
	{.status = 3221225718, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the eighth argument.)"},
	{.status = 3221225719, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the ninth argument.)"},
	{.status = 3221225720, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the tenth argument.)"},
	{.status = 3221225721, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the eleventh argument.)"},
	{.status = 3221225722, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid parameter was passed to a service or function as the twelfth argument.)"},
	{.status = 3221225723, .dos_err = 3, .posix_err = 2, .msg = R"(An attempt was made to access a network file, but the network software was not yet started.)"},
	{.status = 3221225724, .dos_err = 1056, .posix_err = 0, .msg = R"(An attempt was made to start the redirector, but the redirector has already been started.)"},
	{.status = 3221225725, .dos_err = 1001, .posix_err = 0, .msg = R"(A new guard page for the stack cannot be created.)"},
	{.status = 3221225726, .dos_err = 1364, .posix_err = 0, .msg = R"(A specified authentication package is unknown.)"},
	{.status = 3221225727, .dos_err = 559, .posix_err = 0, .msg = R"(A malformed function table was encountered during an unwind operation.)"},
	{.status = 3221225728, .dos_err = 203, .posix_err = 0, .msg = R"(Indicates the specified environment variable name was not found in the specified environment block.)"},
	{.status = 3221225729, .dos_err = 145, .posix_err = 41, .msg = R"(Indicates that the directory trying to be deleted is not empty.)"},
	{.status = 3221225730, .dos_err = 1392, .posix_err = 0, .msg = R"({Corrupt File}

The file or directory %hs is corrupt and unreadable.

Please run the Chkdsk utility.)"},
	{.status = 3221225731, .dos_err = 267, .posix_err = 22, .msg = R"(A requested opened file is not a directory.)"},
	{.status = 3221225732, .dos_err = 1365, .posix_err = 0, .msg = R"(The logon session is not in a state that is consistent with the requested operation.)"},
	{.status = 3221225733, .dos_err = 1366, .posix_err = 0, .msg = R"(An internal LSA error has occurred. An authentication package has requested the creation of a Logon Session but the ID of an already existing Logon Session has been specified.)"},
	{.status = 3221225734, .dos_err = 206, .posix_err = 38, .msg = R"(A specified name string is too long for its intended use.)"},
	{.status = 3221225735, .dos_err = 2401, .posix_err = 16, .msg = R"(The user attempted to force close the files on a redirected drive, but there were opened files on the drive, and the user did not specify a sufficient level of force.)"},
	{.status = 3221225736, .dos_err = 2404, .posix_err = 16, .msg = R"(The user attempted to force close the files on a redirected drive, but there were opened directories on the drive, and the user did not specify a sufficient level of force.)"},
	{.status = 3221225737, .dos_err = 317, .posix_err = 0, .msg = R"(RtlFindMessage could not locate the requested message ID in the message table resource.)"},
	{.status = 3221225738, .dos_err = 5, .posix_err = 13, .msg = R"(An attempt was made to access an exiting process.)"},
	{.status = 3221225739, .dos_err = 1367, .posix_err = 0, .msg = R"(Indicates an invalid value has been provided for the LogonType requested.)"},
	{.status = 3221225740, .dos_err = 560, .posix_err = 0, .msg = R"(Indicates that an attempt was made to assign protection to a file system file or directory and one of the SIDs in the security descriptor could not be translated into a GUID that could be stored by the file system.

This causes the protection attempt to fail, which may cause a file creation attempt to fail.)"},
	{.status = 3221225741, .dos_err = 1368, .posix_err = 0, .msg = R"(Indicates that an attempt has been made to impersonate via a named pipe that has not yet been read from.)"},
	{.status = 3221225742, .dos_err = 1056, .posix_err = 0, .msg = R"(Indicates that the specified image is already loaded.)"},
	{.status = 3221225743, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_NOT_PRESENT)"},
	{.status = 3221225744, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_LID_NOT_EXIST)"},
	{.status = 3221225745, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_LID_ALREADY_OWNED)"},
	{.status = 3221225746, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_NOT_LID_OWNER)"},
	{.status = 3221225747, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_INVALID_COMMAND)"},
	{.status = 3221225748, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_INVALID_LID)"},
	{.status = 3221225749, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_SELECTOR_NOT_AVAILABLE)"},
	{.status = 3221225750, .dos_err = 538, .posix_err = 0, .msg = R"(STATUS_ABIOS_INVALID_SELECTOR)"},
	{.status = 3221225751, .dos_err = 1444, .posix_err = 0, .msg = R"(Indicates that an attempt was made to change the size of the LDT for a process that has no LDT.)"},
	{.status = 3221225752, .dos_err = 561, .posix_err = 0, .msg = R"(Indicates that an attempt was made to grow an LDT by setting its size, or that the size was not an even number of selectors.)"},
	{.status = 3221225753, .dos_err = 563, .posix_err = 0, .msg = R"(Indicates that the starting value for the LDT information was not an integral multiple of the selector size.)"},
	{.status = 3221225754, .dos_err = 564, .posix_err = 0, .msg = R"(Indicates that the user supplied an invalid descriptor when trying to set up Ldt descriptors.)"},
	{.status = 3221225755, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format. It appears to be NE format.)"},
	{.status = 3221225756, .dos_err = 1369, .posix_err = 0, .msg = R"(Indicates that the transaction state of a registry sub-tree is incompatible with the requested operation. For example, a request has been made to start a new transaction with one already in progress, or a request has been made to apply a transaction when one is not currently in progress.)"},
	{.status = 3221225757, .dos_err = 1370, .posix_err = 0, .msg = R"(Indicates an error has occurred during a registry transaction commit. The database has been left in an unknown, but probably inconsistent, state. The state of the registry transaction is left as COMMITTING.)"},
	{.status = 3221225758, .dos_err = 1006, .posix_err = 0, .msg = R"(An attempt was made to map a file of size zero with the maximum size specified as zero.)"},
	{.status = 3221225759, .dos_err = 4, .posix_err = 24, .msg = R"(Too many files are opened on a remote server.

This error should only be returned by the Windows redirector on a remote drive.)"},
	{.status = 3221225760, .dos_err = 995, .posix_err = 105, .msg = R"(The I/O request was canceled.)"},
	{.status = 3221225761, .dos_err = 5, .posix_err = 13, .msg = R"(An attempt has been made to remove a file or directory that cannot be deleted.)"},
	{.status = 3221225762, .dos_err = 1210, .posix_err = 0, .msg = R"(Indicates a name specified as a remote computer name is syntactically invalid.)"},
	{.status = 3221225763, .dos_err = 5, .posix_err = 13, .msg = R"(An I/O request other than close was performed on a file after it has been deleted, which can only happen to a request which did not complete before the last handle was closed via NtClose.)"},
	{.status = 3221225764, .dos_err = 1371, .posix_err = 0, .msg = R"(Indicates an operation has been attempted on a built-in (special) SAM account which is incompatible with built-in accounts. For example, built-in accounts cannot be deleted.)"},
	{.status = 3221225765, .dos_err = 1372, .posix_err = 0, .msg = R"(The operation requested may not be performed on the specified group because it is a built-in special group.)"},
	{.status = 3221225766, .dos_err = 1373, .posix_err = 0, .msg = R"(The operation requested may not be performed on the specified user because it is a built-in special user.)"},
	{.status = 3221225767, .dos_err = 1374, .posix_err = 0, .msg = R"(Indicates a member cannot be removed from a group because the group is currently the member's primary group.)"},
	{.status = 3221225768, .dos_err = 6, .posix_err = 22, .msg = R"(An I/O request other than close and several other special case operations was attempted using a file object that had already been closed.)"},
	{.status = 3221225769, .dos_err = 565, .posix_err = 0, .msg = R"(Indicates a process has too many threads to perform the requested action. For example, assignment of a primary token may only be performed when a process has zero or one threads.)"},
	{.status = 3221225770, .dos_err = 566, .posix_err = 0, .msg = R"(An attempt was made to operate on a thread within a specific process, but the thread specified is not in the process specified.)"},
	{.status = 3221225771, .dos_err = 1375, .posix_err = 0, .msg = R"(An attempt was made to establish a token for use as a primary token but the token is already in use. A token can only be the primary token of one process at a time.)"},
	{.status = 3221225772, .dos_err = 567, .posix_err = 0, .msg = R"(Page file quota was exceeded.)"},
	{.status = 3221225773, .dos_err = 1455, .posix_err = 0, .msg = R"({Out of Virtual Memory}

Your system is low on virtual memory. To ensure that Windows runs properly, increase the size of your virtual memory paging file. For more information, see Help.)"},
	{.status = 3221225774, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format, it appears to be LE format.)"},
	{.status = 3221225775, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format, it did not have an initial MZ.)"},
	{.status = 3221225776, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format, it did not have a proper e_lfarlc in the MZ header.)"},
	{.status = 3221225777, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format, it appears to be a 16-bit Windows image.)"},
	{.status = 3221225778, .dos_err = 568, .posix_err = 0, .msg = R"(The Netlogon service cannot start because another Netlogon service running in the domain conflicts with the specified role.)"},
	{.status = 3221225779, .dos_err = 1398, .posix_err = 0, .msg = R"(The time at the Primary Domain Controller is different than the time at the Backup Domain Controller or member server by too large an amount.)"},
	{.status = 3221225780, .dos_err = 569, .posix_err = 0, .msg = R"(The SAM database on a Windows Server is significantly out of synchronization with the copy on the Domain Controller. A complete synchronization is required.)"},
	{.status = 3221225781, .dos_err = 126, .posix_err = 0, .msg = R"(The code execution cannot proceed because %hs was not found. Reinstalling the program may fix this problem.)"},
	{.status = 3221225782, .dos_err = 570, .posix_err = 0, .msg = R"(The NtCreateFile API failed. This error should never be returned to an application, it is a place holder for the Windows Lan Manager Redirector to use in its internal error mapping routines.)"},
	{.status = 3221225783, .dos_err = 571, .posix_err = 0, .msg = R"({Privilege Failed}

The I/O permissions for the process could not be changed.)"},
	{.status = 3221225784, .dos_err = 182, .posix_err = 0, .msg = R"({Ordinal Not Found}

The ordinal %ld could not be located in the dynamic link library %hs.)"},
	{.status = 3221225785, .dos_err = 127, .posix_err = 0, .msg = R"({Entry Point Not Found}

The procedure entry point %hs could not be located in the dynamic link library %hs.)"},
	{.status = 3221225786, .dos_err = 572, .posix_err = 0, .msg = R"({Application Exit by CTRL+C}

The application terminated as a result of a CTRL+C.)"},
	{.status = 3221225787, .dos_err = 64, .posix_err = 0, .msg = R"({Virtual Circuit Closed}

The network transport on your computer has closed a network connection. There may or may not be I/O requests outstanding.)"},
	{.status = 3221225788, .dos_err = 64, .posix_err = 0, .msg = R"({Virtual Circuit Closed}

The network transport on a remote computer has closed a network connection. There may or may not be I/O requests outstanding.)"},
	{.status = 3221225789, .dos_err = 51, .posix_err = 0, .msg = R"({Insufficient Resources on Remote Computer}

The remote computer has insufficient resources to complete the network request. For instance, there may not be enough memory available on the remote computer to carry out the request at this time.)"},
	{.status = 3221225790, .dos_err = 59, .posix_err = 0, .msg = R"({Virtual Circuit Closed}

An existing connection (virtual circuit) has been broken at the remote computer. There is probably something wrong with the network software protocol or the network hardware on the remote computer.)"},
	{.status = 3221225791, .dos_err = 59, .posix_err = 0, .msg = R"({Virtual Circuit Closed}

The network transport on your computer has closed a network connection because it had to wait too long for a response from the remote computer.)"},
	{.status = 3221225792, .dos_err = 59, .posix_err = 0, .msg = R"(The connection handle given to the transport was invalid.)"},
	{.status = 3221225793, .dos_err = 59, .posix_err = 0, .msg = R"(The address handle given to the transport was invalid.)"},
	{.status = 3221225794, .dos_err = 1114, .posix_err = 0, .msg = R"({DLL Initialization Failed}

Initialization of the dynamic link library %hs failed. The process is terminating abnormally.)"},
	{.status = 3221225795, .dos_err = 573, .posix_err = 0, .msg = R"({Missing System File}

The required system file %hs is bad or missing.)"},
	{.status = 3221225796, .dos_err = 574, .posix_err = 0, .msg = R"({Application Error}

The exception %s (0x)"},
	{.status = 3221225797, .dos_err = 575, .posix_err = 0, .msg = R"({Application Error}

The application was unable to start correctly (0x%lx). Click OK to close the application.)"},
	{.status = 3221225798, .dos_err = 576, .posix_err = 0, .msg = R"({Unable to Create Paging File}

The creation of the paging file %hs failed (%lx). The requested size was %ld.)"},
	{.status = 3221225799, .dos_err = 578, .posix_err = 0, .msg = R"({No Paging File Specified}

No paging file was specified in the system configuration.)"},
	{.status = 3221225800, .dos_err = 124, .posix_err = 0, .msg = R"({Incorrect System Call Level}

An invalid level was passed into the specified system call.)"},
	{.status = 3221225801, .dos_err = 86, .posix_err = 0, .msg = R"({Incorrect Password to LAN Manager Server}

You specified an incorrect password to a LAN Manager 2.x or MS-NET server.)"},
	{.status = 3221225802, .dos_err = 579, .posix_err = 0, .msg = R"({EXCEPTION}

A real-mode application issued a floating-point instruction and floating-point hardware is not present.)"},
	{.status = 3221225803, .dos_err = 109, .posix_err = 32, .msg = R"(The pipe operation has failed because the other end of the pipe has been closed.)"},
	{.status = 3221225804, .dos_err = 1009, .posix_err = 0, .msg = R"({The Registry Is Corrupt}

The structure of one of the files that contains Registry data is corrupt, or the image of the file in memory is corrupt, or the file could not be recovered because the alternate copy or log was absent or corrupt.)"},
	{.status = 3221225805, .dos_err = 1016, .posix_err = 0, .msg = R"(An I/O operation initiated by the Registry failed unrecoverably. The Registry could not read in, or write out, or flush, one of the files that contain the system's image of the Registry.)"},
	{.status = 3221225806, .dos_err = 580, .posix_err = 0, .msg = R"(An event pair synchronization operation was performed using the thread specific client/server event pair object, but no event pair object was associated with the thread.)"},
	{.status = 3221225807, .dos_err = 1005, .posix_err = 0, .msg = R"(The volume does not contain a recognized file system. Please make sure that all required file system drivers are loaded and that the volume is not corrupt.)"},
	{.status = 3221225808, .dos_err = 1118, .posix_err = 0, .msg = R"(No serial device was successfully initialized. The serial driver will unload.)"},
	{.status = 3221225809, .dos_err = 1376, .posix_err = 0, .msg = R"(The specified local group does not exist.)"},
	{.status = 3221225810, .dos_err = 1377, .posix_err = 0, .msg = R"(The specified account name is not a member of the group.)"},
	{.status = 3221225811, .dos_err = 1378, .posix_err = 0, .msg = R"(The specified account name is already a member of the group.)"},
	{.status = 3221225812, .dos_err = 1379, .posix_err = 0, .msg = R"(The specified local group already exists.)"},
	{.status = 3221225813, .dos_err = 1380, .posix_err = 0, .msg = R"(A requested type of logon (e.g., Interactive, Network, Service) is not granted by the target system's local security policy.

Please ask the system administrator to grant the necessary form of logon.)"},
	{.status = 3221225814, .dos_err = 1381, .posix_err = 0, .msg = R"(The maximum number of secrets that may be stored in a single system has been exceeded. The length and number of secrets is limited to satisfy United States State Department export restrictions.)"},
	{.status = 3221225815, .dos_err = 1382, .posix_err = 0, .msg = R"(The length of a secret exceeds the maximum length allowed. The length and number of secrets is limited to satisfy United States State Department export restrictions.)"},
	{.status = 3221225816, .dos_err = 1383, .posix_err = 0, .msg = R"(The Local Security Authority (LSA) database contains an internal inconsistency.)"},
	{.status = 3221225817, .dos_err = 1007, .posix_err = 0, .msg = R"(The requested operation cannot be performed in fullscreen mode.)"},
	{.status = 3221225818, .dos_err = 1384, .posix_err = 0, .msg = R"(During a logon attempt, the user's security context accumulated too many security IDs. This is a very unusual situation. Remove the user from some global or local groups to reduce the number of security ids to incorporate into the security context.)"},
	{.status = 3221225819, .dos_err = 1385, .posix_err = 0, .msg = R"(A user has requested a type of logon (e.g., interactive or network) that has not been granted. An administrator has control over who may logon interactively and through the network.)"},
	{.status = 3221225820, .dos_err = 1017, .posix_err = 0, .msg = R"(The system has attempted to load or restore a file into the registry, and the specified file is not in the format of a registry file.)"},
	{.status = 3221225821, .dos_err = 1386, .posix_err = 0, .msg = R"(An attempt was made to change a user password in the security account manager without providing the necessary Windows cross-encrypted password.)"},
	{.status = 3221225822, .dos_err = 581, .posix_err = 0, .msg = R"(A Windows Server has an incorrect configuration.)"},
	{.status = 3221225823, .dos_err = 1117, .posix_err = 0, .msg = R"(An attempt was made to explicitly access the secondary copy of information via a device control to the Fault Tolerance driver and the secondary copy is not present in the system.)"},
	{.status = 3221225824, .dos_err = 1243, .posix_err = 0, .msg = R"(A configuration registry node representing a driver service entry was ill-formed and did not contain required value entries.)"},
	{.status = 3221225825, .dos_err = 582, .posix_err = 0, .msg = R"(An illegal character was encountered. For a multi-byte character set this includes a lead byte without a succeeding trail byte. For the Unicode character set this includes the characters 0xFFFF and 0xFFFE.)"},
	{.status = 3221225826, .dos_err = 1113, .posix_err = 0, .msg = R"(No mapping for the Unicode character exists in the target multi-byte code page.)"},
	{.status = 3221225827, .dos_err = 583, .posix_err = 0, .msg = R"(The Unicode character is not defined in the Unicode character set installed on the system.)"},
	{.status = 3221225828, .dos_err = 584, .posix_err = 0, .msg = R"(The paging file cannot be created on a floppy diskette.)"},
	{.status = 3221225829, .dos_err = 1122, .posix_err = 0, .msg = R"({Floppy Disk Error}

While accessing a floppy disk, an ID address mark was not found.)"},
	{.status = 3221225830, .dos_err = 1123, .posix_err = 0, .msg = R"({Floppy Disk Error}

While accessing a floppy disk, the track address from the sector ID field was found to be different than the track address maintained by the controller.)"},
	{.status = 3221225831, .dos_err = 1124, .posix_err = 0, .msg = R"({Floppy Disk Error}

The floppy disk controller reported an error that is not recognized by the floppy disk driver.)"},
	{.status = 3221225832, .dos_err = 1125, .posix_err = 0, .msg = R"({Floppy Disk Error}

While accessing a floppy-disk, the controller returned inconsistent results via its registers.)"},
	{.status = 3221225833, .dos_err = 1126, .posix_err = 0, .msg = R"({Hard Disk Error}

While accessing the hard disk, a recalibrate operation failed, even after retries.)"},
	{.status = 3221225834, .dos_err = 1127, .posix_err = 0, .msg = R"({Hard Disk Error}

While accessing the hard disk, a disk operation failed even after retries.)"},
	{.status = 3221225835, .dos_err = 1128, .posix_err = 0, .msg = R"({Hard Disk Error}

While accessing the hard disk, a disk controller reset was needed, but even that failed.)"},
	{.status = 3221225836, .dos_err = 1119, .posix_err = 0, .msg = R"(An attempt was made to open a device that was sharing an IRQ with other devices.

At least one other device that uses that IRQ was already opened.

Two concurrent opens of devices that share an IRQ and only work via interrupts is not supported for the particular bus type that the devices use.)"},
	{.status = 3221225837, .dos_err = 1117, .posix_err = 0, .msg = R"({FT Orphaning}

A disk that is part of a fault-tolerant volume can no longer be accessed.)"},
	{.status = 3221225838, .dos_err = 585, .posix_err = 0, .msg = R"(The system bios failed to connect a system interrupt to the device or bus for which the device is connected.)"},
	{.status = 3221225842, .dos_err = 1105, .posix_err = 0, .msg = R"(Tape could not be partitioned.)"},
	{.status = 3221225843, .dos_err = 1106, .posix_err = 0, .msg = R"(When accessing a new tape of a multivolume partition, the current blocksize is incorrect.)"},
	{.status = 3221225844, .dos_err = 1107, .posix_err = 0, .msg = R"(Tape partition information could not be found when loading a tape.)"},
	{.status = 3221225845, .dos_err = 1108, .posix_err = 0, .msg = R"(Attempt to lock the eject media mechanism fails.)"},
	{.status = 3221225846, .dos_err = 1109, .posix_err = 0, .msg = R"(Unload media fails.)"},
	{.status = 3221225847, .dos_err = 1129, .posix_err = 0, .msg = R"(Physical end of tape was detected.)"},
	{.status = 3221225848, .dos_err = 1112, .posix_err = 0, .msg = R"({No Media}

There is no media in the drive. Please insert media into drive %hs.)"},
	{.status = 3221225850, .dos_err = 1387, .posix_err = 0, .msg = R"(A member could not be added to or removed from the local group because the member does not exist.)"},
	{.status = 3221225851, .dos_err = 1388, .posix_err = 0, .msg = R"(A new member could not be added to a local group because the member has the wrong account type.)"},
	{.status = 3221225852, .dos_err = 1018, .posix_err = 0, .msg = R"(Illegal operation attempted on a registry key which has been marked for deletion.)"},
	{.status = 3221225853, .dos_err = 1019, .posix_err = 0, .msg = R"(System could not allocate required space in a registry log.)"},
	{.status = 3221225854, .dos_err = 1389, .posix_err = 0, .msg = R"(Too many Sids have been specified.)"},
	{.status = 3221225855, .dos_err = 1390, .posix_err = 0, .msg = R"(An attempt was made to change a user password in the security account manager without providing the necessary LM cross-encrypted password.)"},
	{.status = 3221225856, .dos_err = 1020, .posix_err = 0, .msg = R"(An attempt was made to create a symbolic link in a registry key that already has subkeys or values.)"},
	{.status = 3221225857, .dos_err = 1021, .posix_err = 0, .msg = R"(An attempt was made to create a Stable subkey under a Volatile parent key.)"},
	{.status = 3221225858, .dos_err = 87, .posix_err = 22, .msg = R"(The I/O device is configured incorrectly or the configuration parameters to the driver are incorrect.)"},
	{.status = 3221225859, .dos_err = 1117, .posix_err = 0, .msg = R"(An error was detected between two drivers or within an I/O driver.)"},
	{.status = 3221225860, .dos_err = 22, .posix_err = 0, .msg = R"(The device is not in a valid state to perform this request.)"},
	{.status = 3221225861, .dos_err = 1117, .posix_err = 0, .msg = R"(The I/O device reported an I/O error.)"},
	{.status = 3221225862, .dos_err = 1117, .posix_err = 0, .msg = R"(A protocol error was detected between the driver and the device.)"},
	{.status = 3221225863, .dos_err = 586, .posix_err = 0, .msg = R"(This operation is only allowed for the Primary Domain Controller of the domain.)"},
	{.status = 3221225864, .dos_err = 1502, .posix_err = 0, .msg = R"(Log file space is insufficient to support this operation.)"},
	{.status = 3221225865, .dos_err = 19, .posix_err = 13, .msg = R"(A write operation was attempted to a volume after it was dismounted.)"},
	{.status = 3221225866, .dos_err = 1786, .posix_err = 0, .msg = R"(The workstation does not have a trust secret for the primary domain in the local LSA database.)"},
	{.status = 3221225867, .dos_err = 1787, .posix_err = 0, .msg = R"(The SAM database on the Windows Server does not have a computer account for this workstation trust relationship.)"},
	{.status = 3221225868, .dos_err = 1788, .posix_err = 0, .msg = R"(The logon request failed because the trust relationship between the primary domain and the trusted domain failed.)"},
	{.status = 3221225869, .dos_err = 1789, .posix_err = 0, .msg = R"(The logon request failed because the trust relationship between this workstation and the primary domain failed.)"},
	{.status = 3221225870, .dos_err = 1500, .posix_err = 0, .msg = R"(The Eventlog log file is corrupt.)"},
	{.status = 3221225871, .dos_err = 1501, .posix_err = 0, .msg = R"(No Eventlog log file could be opened. The Eventlog service did not start.)"},
	{.status = 3221225872, .dos_err = 1790, .posix_err = 0, .msg = R"(The network logon failed. This may be because the validation authority can't be reached.)"},
	{.status = 3221225873, .dos_err = 587, .posix_err = 0, .msg = R"(An attempt was made to acquire a mutant such that its maximum count would have been exceeded.)"},
	{.status = 3221225874, .dos_err = 1792, .posix_err = 0, .msg = R"(An attempt was made to logon, but the netlogon service was not started.)"},
	{.status = 3221225875, .dos_err = 1793, .posix_err = 0, .msg = R"(The user's account has expired.)"},
	{.status = 3221225876, .dos_err = 1131, .posix_err = 0, .msg = R"({EXCEPTION}

Possible deadlock condition.)"},
	{.status = 3221225877, .dos_err = 1219, .posix_err = 0, .msg = R"(Multiple connections to a server or shared resource by the same user, using more than one user name, are not allowed. Disconnect all previous connections to the server or shared resource and try again.)"},
	{.status = 3221225878, .dos_err = 1220, .posix_err = 0, .msg = R"(An attempt was made to establish a session to a network server, but there are already too many sessions established to that server.)"},
	{.status = 3221225879, .dos_err = 1503, .posix_err = 0, .msg = R"(The log file has changed between reads.)"},
	{.status = 3221225880, .dos_err = 1807, .posix_err = 0, .msg = R"(The account used is an Interdomain Trust account. Use your global user account or local user account to access this server.)"},
	{.status = 3221225881, .dos_err = 1808, .posix_err = 0, .msg = R"(The account used is a Computer Account. Use your global user account or local user account to access this server.)"},
	{.status = 3221225882, .dos_err = 1809, .posix_err = 0, .msg = R"(The account used is an Server Trust account. Use your global user account or local user account to access this server.)"},
	{.status = 3221225883, .dos_err = 1810, .posix_err = 0, .msg = R"(The name or SID of the domain specified is inconsistent with the trust information for that domain.)"},
	{.status = 3221225884, .dos_err = 588, .posix_err = 0, .msg = R"(A volume has been accessed for which a file system driver is required that has not yet been loaded.)"},
	{.status = 3221225885, .dos_err = 1056, .posix_err = 0, .msg = R"(Indicates that the specified image is already loaded as a DLL.)"},
	{.status = 3221225886, .dos_err = 304, .posix_err = 0, .msg = R"(Short name settings may not be changed on this volume due to the global registry setting.)"},
	{.status = 3221225887, .dos_err = 305, .posix_err = 0, .msg = R"(Short names are not enabled on this volume.)"},
	{.status = 3221225888, .dos_err = 306, .posix_err = 0, .msg = R"(The security stream for the given volume is in an inconsistent state.

Please run CHKDSK on the volume.)"},
	{.status = 3221225889, .dos_err = 307, .posix_err = 0, .msg = R"(A requested file lock operation cannot be processed due to an invalid byte range.)"},
	{.status = 3221225890, .dos_err = 805, .posix_err = 0, .msg = R"({Invalid ACE Condition}

The specified access control entry (ACE) contains an invalid condition.)"},
	{.status = 3221225891, .dos_err = 308, .posix_err = 0, .msg = R"(The subsystem needed to support the image type is not present.)"},
	{.status = 3221225892, .dos_err = 309, .posix_err = 0, .msg = R"({Invalid ACE Condition}

The specified file already has a notification GUID associated with it.)"},
	{.status = 3221225893, .dos_err = 310, .posix_err = 0, .msg = R"(An invalid exception handler routine has been detected.)"},
	{.status = 3221225894, .dos_err = 311, .posix_err = 0, .msg = R"(Duplicate privileges were specified for the token.)"},
	{.status = 3221225895, .dos_err = 313, .posix_err = 0, .msg = R"(Requested action not allowed on a file system internal file.)"},
	{.status = 3221225896, .dos_err = 6843, .posix_err = 0, .msg = R"(A portion of the file system requires repair.)"},
	{.status = 3221225897, .dos_err = 50, .posix_err = 129, .msg = R"(Quota support is not enabled on the system.)"},
	{.status = 3221225898, .dos_err = 15700, .posix_err = 0, .msg = R"(The operation failed because the application is not part of an application package.)"},
	{.status = 3221225899, .dos_err = 809, .posix_err = 0, .msg = R"(File metadata optimization is already in progress.)"},
	{.status = 3221225900, .dos_err = 1656, .posix_err = 0, .msg = R"(The objects are not identical.)"},
	{.status = 3221225901, .dos_err = 8, .posix_err = 12, .msg = R"(The process has terminated because it could not allocate additional memory.)"},
	{.status = 3221225902, .dos_err = 759, .posix_err = 0, .msg = R"(The process is not part of a job.)"},
	{.status = 3221225903, .dos_err = 813, .posix_err = 0, .msg = R"(The specified CPU Set IDs are invalid.)"},
	{.status = 3221225904, .dos_err = 13, .posix_err = 0, .msg = R"(The device reported an invalid data error.)"},
	{.status = 3221225905, .dos_err = 13, .posix_err = 0, .msg = R"(The device reported an unaligned write error.)"},
	{.status = 3221225906, .dos_err = 998, .posix_err = 13, .msg = R"(An invalid address was found on the control flow stack.)"},
	{.status = 3221225985, .dos_err = 65, .posix_err = 0, .msg = R"(A remote open failed because the network open restrictions were not satisfied.)"},
	{.status = 3221225986, .dos_err = 1394, .posix_err = 0, .msg = R"(There is no user session key for the specified logon session.)"},
	{.status = 3221225987, .dos_err = 59, .posix_err = 0, .msg = R"(The remote user session has been deleted.)"},
	{.status = 3221225988, .dos_err = 1815, .posix_err = 0, .msg = R"(Indicates the specified resource language ID cannot be found in the

image file.)"},
	{.status = 3221225989, .dos_err = 1130, .posix_err = 0, .msg = R"(Insufficient server resources exist to complete the request.)"},
	{.status = 3221225990, .dos_err = 1784, .posix_err = 0, .msg = R"(The size of the buffer is invalid for the specified operation.)"},
	{.status = 3221225991, .dos_err = 1214, .posix_err = 0, .msg = R"(The transport rejected the network address specified as invalid.)"},
	{.status = 3221225992, .dos_err = 1214, .posix_err = 0, .msg = R"(The transport rejected the network address specified due to an invalid use of a wildcard.)"},
	{.status = 3221225993, .dos_err = 68, .posix_err = 0, .msg = R"(The transport address could not be opened because all the available addresses are in use.)"},
	{.status = 3221225994, .dos_err = 52, .posix_err = 0, .msg = R"(The transport address could not be opened because it already exists.)"},
	{.status = 3221225995, .dos_err = 64, .posix_err = 0, .msg = R"(The transport address is now closed.)"},
	{.status = 3221225996, .dos_err = 64, .posix_err = 0, .msg = R"(The transport connection is now disconnected.)"},
	{.status = 3221225997, .dos_err = 64, .posix_err = 0, .msg = R"(The transport connection has been reset.)"},
	{.status = 3221225998, .dos_err = 68, .posix_err = 0, .msg = R"(The transport cannot dynamically acquire any more nodes.)"},
	{.status = 3221225999, .dos_err = 59, .posix_err = 0, .msg = R"(The transport aborted a pending transaction.)"},
	{.status = 3221226000, .dos_err = 59, .posix_err = 0, .msg = R"(The transport timed out a request waiting for a response.)"},
	{.status = 3221226001, .dos_err = 59, .posix_err = 0, .msg = R"(The transport did not receive a release for a pending response.)"},
	{.status = 3221226002, .dos_err = 59, .posix_err = 0, .msg = R"(The transport did not find a transaction matching the specific token.)"},
	{.status = 3221226003, .dos_err = 59, .posix_err = 0, .msg = R"(The transport had previously responded to a transaction request.)"},
	{.status = 3221226004, .dos_err = 59, .posix_err = 0, .msg = R"(The transport does not recognized the transaction request identifier specified.)"},
	{.status = 3221226005, .dos_err = 59, .posix_err = 0, .msg = R"(The transport does not recognize the transaction request type specified.)"},
	{.status = 3221226006, .dos_err = 50, .posix_err = 129, .msg = R"(The transport can only process the specified request on the server side of a session.)"},
	{.status = 3221226007, .dos_err = 50, .posix_err = 129, .msg = R"(The transport can only process the specified request on the client side of a session.)"},
	{.status = 3221226008, .dos_err = 589, .posix_err = 0, .msg = R"({Registry File Failure}

The registry cannot load the hive (file):

%hs

or its log or alternate.

It is corrupt, absent, or not writable.)"},
	{.status = 3221226009, .dos_err = 590, .posix_err = 0, .msg = R"({Unexpected Failure in DebugActiveProcess}

An unexpected failure occurred while processing a DebugActiveProcess API request. You may choose OK to terminate the process, or Cancel to ignore the error.)"},
	{.status = 3221226010, .dos_err = 591, .posix_err = 0, .msg = R"({Fatal System Error}

The %hs system process terminated unexpectedly with a status of 0x)"},
	{.status = 3221226011, .dos_err = 592, .posix_err = 0, .msg = R"({Data Not Accepted}

The TDI client could not handle the data received during an indication.)"},
	{.status = 3221226012, .dos_err = 6118, .posix_err = 0, .msg = R"({Unable to Retrieve Browser Server List}

The list of servers for this workgroup is not currently available.)"},
	{.status = 3221226013, .dos_err = 593, .posix_err = 0, .msg = R"(NTVDM encountered a hard error.)"},
	{.status = 3221226014, .dos_err = 594, .posix_err = 0, .msg = R"({Cancel Timeout}

The driver %hs failed to complete a cancelled I/O request in the allotted time.)"},
	{.status = 3221226015, .dos_err = 595, .posix_err = 0, .msg = R"({Reply Message Mismatch}

An attempt was made to reply to an LPC message, but the thread specified by the client ID in the message was not waiting on that message.)"},
	{.status = 3221226016, .dos_err = 1132, .posix_err = 0, .msg = R"({Mapped View Alignment Incorrect}

An attempt was made to map a view of a file, but either the specified base address or the offset into the file were not aligned on the proper allocation granularity.)"},
	{.status = 3221226017, .dos_err = 193, .posix_err = 0, .msg = R"({Bad Image Checksum}

The image %hs is possibly corrupt. The header checksum does not match the computed checksum.)"},
	{.status = 3221226018, .dos_err = 596, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs. The data has been lost. This error may be caused by a failure of your computer hardware or network connection. Please try to save this file elsewhere.)"},
	{.status = 3221226019, .dos_err = 597, .posix_err = 0, .msg = R"(The parameter(s) passed to the server in the client/server shared memory window were invalid. Too much data may have been put in the shared memory window.)"},
	{.status = 3221226020, .dos_err = 1907, .posix_err = 0, .msg = R"(The user's password must be changed before signing in.)"},
	{.status = 3221226021, .dos_err = 1168, .posix_err = 0, .msg = R"(The object was not found.)"},
	{.status = 3221226022, .dos_err = 598, .posix_err = 0, .msg = R"(The stream is not a tiny stream.)"},
	{.status = 3221226023, .dos_err = 1279, .posix_err = 0, .msg = R"(A transaction recover failed.)"},
	{.status = 3221226024, .dos_err = 599, .posix_err = 0, .msg = R"(The request must be handled by the stack overflow code.)"},
	{.status = 3221226025, .dos_err = 87, .posix_err = 22, .msg = R"(A consistency check failed.)"},
	{.status = 3221226026, .dos_err = 5010, .posix_err = 0, .msg = R"(The attempt to insert the ID in the index failed because the ID is already in the index.)"},
	{.status = 3221226027, .dos_err = 5010, .posix_err = 0, .msg = R"(The attempt to set the object's ID failed because the object already has an ID.)"},
	{.status = 3221226028, .dos_err = 600, .posix_err = 0, .msg = R"(Internal OFS status codes indicating how an allocation operation is handled. Either it is retried after the containing onode is moved or the extent stream is converted to a large stream.)"},
	{.status = 3221226029, .dos_err = 1237, .posix_err = 11, .msg = R"(The request needs to be retried.)"},
	{.status = 3221226030, .dos_err = 601, .posix_err = 0, .msg = R"(The attempt to find the object found an object matching by ID on the volume but it is out of the scope of the handle used for the operation.)"},
	{.status = 3221226031, .dos_err = 602, .posix_err = 0, .msg = R"(The bucket array must be grown. Retry transaction after doing so.)"},
	{.status = 3221226032, .dos_err = 1170, .posix_err = 0, .msg = R"(The property set specified does not exist on the object.)"},
	{.status = 3221226033, .dos_err = 603, .posix_err = 0, .msg = R"(The user/kernel marshalling buffer has overflowed.)"},
	{.status = 3221226034, .dos_err = 604, .posix_err = 0, .msg = R"(The supplied variant structure contains invalid data.)"},
	{.status = 3221226035, .dos_err = 1908, .posix_err = 0, .msg = R"(Could not find a domain controller for this domain.)"},
	{.status = 3221226036, .dos_err = 1909, .posix_err = 0, .msg = R"(The user account has been automatically locked because too many invalid logon attempts or password change attempts have been requested.)"},
	{.status = 3221226037, .dos_err = 6, .posix_err = 22, .msg = R"(NtClose was called on a handle that was protected from close via NtSetInformationObject.)"},
	{.status = 3221226038, .dos_err = 1225, .posix_err = 0, .msg = R"(The transport connection attempt was refused by the remote system.)"},
	{.status = 3221226039, .dos_err = 1226, .posix_err = 0, .msg = R"(The transport connection was gracefully closed.)"},
	{.status = 3221226040, .dos_err = 1227, .posix_err = 0, .msg = R"(The transport endpoint already has an address associated with it.)"},
	{.status = 3221226041, .dos_err = 1228, .posix_err = 0, .msg = R"(An address has not yet been associated with the transport endpoint.)"},
	{.status = 3221226042, .dos_err = 1229, .posix_err = 0, .msg = R"(An operation was attempted on a nonexistent transport connection.)"},
	{.status = 3221226043, .dos_err = 1230, .posix_err = 0, .msg = R"(An invalid operation was attempted on an active transport connection.)"},
	{.status = 3221226044, .dos_err = 1231, .posix_err = 0, .msg = R"(The remote network is not reachable by the transport.)"},
	{.status = 3221226045, .dos_err = 1232, .posix_err = 0, .msg = R"(The remote system is not reachable by the transport.)"},
	{.status = 3221226046, .dos_err = 1233, .posix_err = 0, .msg = R"(The remote system does not support the transport protocol.)"},
	{.status = 3221226047, .dos_err = 1234, .posix_err = 0, .msg = R"(No service is operating at the destination port of the transport on the remote system.)"},
	{.status = 3221226048, .dos_err = 1235, .posix_err = 0, .msg = R"(The request was aborted.)"},
	{.status = 3221226049, .dos_err = 1236, .posix_err = 0, .msg = R"(The transport connection was aborted by the local system.)"},
	{.status = 3221226050, .dos_err = 605, .posix_err = 0, .msg = R"(The specified buffer contains ill-formed data.)"},
	{.status = 3221226051, .dos_err = 1224, .posix_err = 0, .msg = R"(The requested operation cannot be performed on a file with a user mapped section open.)"},
	{.status = 3221226052, .dos_err = 606, .posix_err = 0, .msg = R"({Audit Failed}

An attempt to generate a security audit failed.)"},
	{.status = 3221226053, .dos_err = 607, .posix_err = 0, .msg = R"(The timer resolution was not previously set by the current process.)"},
	{.status = 3221226054, .dos_err = 1238, .posix_err = 0, .msg = R"(A connection to the server could not be made because the limit on the number of concurrent connections for this account has been reached.)"},
	{.status = 3221226055, .dos_err = 1239, .posix_err = 0, .msg = R"(Attempting to login during an unauthorized time of day for this account.)"},
	{.status = 3221226056, .dos_err = 1240, .posix_err = 0, .msg = R"(The account is not authorized to login from this station.)"},
	{.status = 3221226057, .dos_err = 193, .posix_err = 0, .msg = R"({UP/MP Image Mismatch}

The image %hs has been modified for use on a uniprocessor system, but you are running it on a multiprocessor machine.

Please reinstall the image file.)"},
	{.status = 3221226064, .dos_err = 608, .posix_err = 0, .msg = R"(There is insufficient account information to log you on.)"},
	{.status = 3221226065, .dos_err = 609, .posix_err = 0, .msg = R"({Invalid DLL Entrypoint}

The dynamic link library %hs is not written correctly. The stack pointer has been left in an inconsistent state. The entrypoint should be declared as WINAPI or STDCALL. Select YES to fail the DLL load. Select NO to continue execution. Selecting NO may cause the application to operate incorrectly.)"},
	{.status = 3221226066, .dos_err = 610, .posix_err = 0, .msg = R"({Invalid Service Callback Entrypoint}

The %hs service is not written correctly. The stack pointer has been left in an inconsistent state. The callback entrypoint should be declared as WINAPI or STDCALL. Selecting OK will cause the service to continue operation. However, the service process may operate incorrectly.)"},
	{.status = 3221226067, .dos_err = 1236, .posix_err = 0, .msg = R"(The server received the messages but did not send a reply.)"},
	{.status = 3221226068, .dos_err = 611, .posix_err = 0, .msg = R"(There is an IP address conflict with another system on the network)"},
	{.status = 3221226069, .dos_err = 612, .posix_err = 0, .msg = R"(There is an IP address conflict with another system on the network)"},
	{.status = 3221226070, .dos_err = 613, .posix_err = 0, .msg = R"({Low On Registry Space}

The system has reached the maximum size allowed for the system part of the registry. Additional storage requests will be ignored.)"},
	{.status = 3221226071, .dos_err = 1232, .posix_err = 0, .msg = R"(The contacted server does not support the indicated part of the DFS namespace.)"},
	{.status = 3221226072, .dos_err = 614, .posix_err = 0, .msg = R"(A callback return system service cannot be executed when no callback is active.)"},
	{.status = 3221226073, .dos_err = 1395, .posix_err = 0, .msg = R"(The service being accessed is licensed for a particular number of connections. No more connections can be made to the service at this time because there are already as many connections as the service can accept.)"},
	{.status = 3221226074, .dos_err = 615, .posix_err = 0, .msg = R"(The password provided is too short to meet the policy of your user account. Please choose a longer password.)"},
	{.status = 3221226075, .dos_err = 616, .posix_err = 0, .msg = R"(The policy of your user account does not allow you to change passwords too frequently. This is done to prevent users from changing back to a familiar, but potentially discovered, password. If you feel your password has been compromised then please contact your administrator immediately to have a new one assigned.)"},
	{.status = 3221226076, .dos_err = 617, .posix_err = 0, .msg = R"(You have attempted to change your password to one that you have used in the past. The policy of your user account does not allow this. Please select a password that you have not previously used.)"},
	{.status = 3221226078, .dos_err = 1058, .posix_err = 0, .msg = R"(You have attempted to load a legacy device driver while its device instance had been disabled.)"},
	{.status = 3221226079, .dos_err = 618, .posix_err = 0, .msg = R"(The specified compression format is unsupported.)"},
	{.status = 3221226080, .dos_err = 619, .posix_err = 0, .msg = R"(The specified hardware profile configuration is invalid.)"},
	{.status = 3221226081, .dos_err = 620, .posix_err = 0, .msg = R"(The specified Plug and Play registry device path is invalid.)"},
	{.status = 3221226082, .dos_err = 182, .posix_err = 0, .msg = R"({Driver Entry Point Not Found}

The %hs device driver could not locate the ordinal %ld in driver %hs.)"},
	{.status = 3221226083, .dos_err = 127, .posix_err = 0, .msg = R"({Driver Entry Point Not Found}

The %hs device driver could not locate the entry point %hs in driver %hs.)"},
	{.status = 3221226084, .dos_err = 288, .posix_err = 0, .msg = R"({Application Error}

The application attempted to release a resource it did not own. Click OK to terminate the application.)"},
	{.status = 3221226085, .dos_err = 1142, .posix_err = 0, .msg = R"(An attempt was made to create more links on a file than the file system supports.)"},
	{.status = 3221226086, .dos_err = 621, .posix_err = 0, .msg = R"(The specified quota list is internally inconsistent with its descriptor.)"},
	{.status = 3221226087, .dos_err = 4350, .posix_err = 0, .msg = R"(The specified file has been relocated to offline storage.)"},
	{.status = 3221226088, .dos_err = 622, .posix_err = 0, .msg = R"({Windows Evaluation Notification}

The evaluation period for this installation of Windows has expired. This system will shutdown in 1 hour. To restore access to this installation of Windows, please upgrade this installation using a licensed distribution of this product.)"},
	{.status = 3221226089, .dos_err = 623, .posix_err = 0, .msg = R"({Illegal System DLL Relocation}

The system DLL %hs was relocated in memory. The application will not run properly. The relocation occurred because the DLL %hs occupied an address range reserved for Windows system DLLs. The vendor supplying the DLL should be contacted for a new DLL.)"},
	{.status = 3221226090, .dos_err = 7054, .posix_err = 0, .msg = R"({License Violation}

The system has detected tampering with your registered product type. This is a violation of your software license. Tampering with product type is not permitted.)"},
	{.status = 3221226091, .dos_err = 624, .posix_err = 0, .msg = R"({DLL Initialization Failed}

The application failed to initialize because the window station is shutting down.)"},
	{.status = 3221226092, .dos_err = 2001, .posix_err = 0, .msg = R"({Unable to Load Device Driver}

%hs device driver could not be loaded.

Error Status was 0x%x)"},
	{.status = 3221226093, .dos_err = 1201, .posix_err = 0, .msg = R"(DFS is unavailable on the contacted server.)"},
	{.status = 3221226094, .dos_err = 21, .posix_err = 11, .msg = R"(An operation was attempted to a volume after it was dismounted.)"},
	{.status = 3221226095, .dos_err = 540, .posix_err = 0, .msg = R"(An internal error occurred in the Win32 x86 emulation subsystem.)"},
	{.status = 3221226096, .dos_err = 540, .posix_err = 0, .msg = R"(Win32 x86 emulation subsystem Floating-point stack check.)"},
	{.status = 3221226097, .dos_err = 625, .posix_err = 0, .msg = R"(The validation process needs to continue on to the next step.)"},
	{.status = 3221226098, .dos_err = 1169, .posix_err = 0, .msg = R"(There was no match for the specified key in the index.)"},
	{.status = 3221226099, .dos_err = 626, .posix_err = 0, .msg = R"(There are no more matches for the current index enumeration.)"},
	{.status = 3221226101, .dos_err = 4390, .posix_err = 0, .msg = R"(The file or directory is not a reparse point.)"},
	{.status = 3221226102, .dos_err = 4393, .posix_err = 22, .msg = R"(The Windows I/O reparse tag passed for the reparse point is invalid.)"},
	{.status = 3221226103, .dos_err = 4394, .posix_err = 0, .msg = R"(The Windows I/O reparse tag does not match the one present in the reparse point.)"},
	{.status = 3221226104, .dos_err = 4392, .posix_err = 0, .msg = R"(The user data passed for the reparse point is invalid.)"},
	{.status = 3221226105, .dos_err = 1920, .posix_err = 0, .msg = R"(The layered file system driver for this IO tag did not handle it when needed.)"},
	{.status = 3221226106, .dos_err = 657, .posix_err = 0, .msg = R"(The password provided is too long to meet the policy of your user account. Please choose a shorter password.)"},
	{.status = 3221226107, .dos_err = 1359, .posix_err = 0, .msg = R"(An application-internal exception has occurred.)"},
	{.status = 3221226108, .dos_err = 1359, .posix_err = 0, .msg = R"(An application-internal exception has occurred.)"},
	{.status = 3221226112, .dos_err = 1921, .posix_err = 0, .msg = R"(The symbolic link could not be resolved even though the initial file name is valid.)"},
	{.status = 3221226113, .dos_err = 161, .posix_err = 0, .msg = R"(The directory is a reparse point.)"},
	{.status = 3221226114, .dos_err = 627, .posix_err = 0, .msg = R"(The range could not be added to the range list because of a conflict.)"},
	{.status = 3221226115, .dos_err = 1160, .posix_err = 0, .msg = R"(The specified medium changer source element contains no media.)"},
	{.status = 3221226116, .dos_err = 1161, .posix_err = 0, .msg = R"(The specified medium changer destination element already contains media.)"},
	{.status = 3221226117, .dos_err = 1162, .posix_err = 0, .msg = R"(The specified medium changer element does not exist.)"},
	{.status = 3221226118, .dos_err = 1163, .posix_err = 0, .msg = R"(The specified element is contained within a magazine that is no longer present.)"},
	{.status = 3221226119, .dos_err = 1164, .posix_err = 0, .msg = R"(The device requires reinitialization due to hardware errors.)"},
	{.status = 3221226122, .dos_err = 5, .posix_err = 13, .msg = R"(The file encryption attempt failed.)"},
	{.status = 3221226123, .dos_err = 5, .posix_err = 13, .msg = R"(The file decryption attempt failed.)"},
	{.status = 3221226124, .dos_err = 644, .posix_err = 0, .msg = R"(The specified range could not be found in the range list.)"},
	{.status = 3221226125, .dos_err = 5, .posix_err = 13, .msg = R"(There is no encryption recovery policy configured for this system.)"},
	{.status = 3221226126, .dos_err = 5, .posix_err = 13, .msg = R"(The required encryption driver is not loaded for this system.)"},
	{.status = 3221226127, .dos_err = 5, .posix_err = 13, .msg = R"(The file was encrypted with a different encryption driver than is currently loaded.)"},
	{.status = 3221226128, .dos_err = 5, .posix_err = 13, .msg = R"(There are no EFS keys defined for the user.)"},
	{.status = 3221226129, .dos_err = 6007, .posix_err = 0, .msg = R"(The specified file is not encrypted.)"},
	{.status = 3221226130, .dos_err = 6008, .posix_err = 0, .msg = R"(The specified file is not in the defined EFS export format.)"},
	{.status = 3221226131, .dos_err = 6002, .posix_err = 0, .msg = R"(The specified file is encrypted and the user does not have the ability to decrypt it.)"},
	{.status = 3221226133, .dos_err = 4200, .posix_err = 0, .msg = R"(The guid passed was not recognized as valid by a WMI data provider.)"},
	{.status = 3221226134, .dos_err = 4201, .posix_err = 0, .msg = R"(The instance name passed was not recognized as valid by a WMI data provider.)"},
	{.status = 3221226135, .dos_err = 4202, .posix_err = 0, .msg = R"(The data item id passed was not recognized as valid by a WMI data provider.)"},
	{.status = 3221226136, .dos_err = 4203, .posix_err = 0, .msg = R"(The WMI request could not be completed and should be retried.)"},
	{.status = 3221226137, .dos_err = 8218, .posix_err = 0, .msg = R"(The policy object is shared and can only be modified at the root)"},
	{.status = 3221226138, .dos_err = 8219, .posix_err = 0, .msg = R"(The policy object does not exist when it should)"},
	{.status = 3221226139, .dos_err = 8220, .posix_err = 0, .msg = R"(The requested policy information only lives in the Ds)"},
	{.status = 3221226140, .dos_err = 1, .posix_err = 40, .msg = R"(The volume must be upgraded to enable this feature)"},
	{.status = 3221226141, .dos_err = 4351, .posix_err = 0, .msg = R"(The remote storage service is not operational at this time.)"},
	{.status = 3221226142, .dos_err = 4352, .posix_err = 0, .msg = R"(The remote storage service encountered a media error.)"},
	{.status = 3221226143, .dos_err = 1172, .posix_err = 0, .msg = R"(The tracking (workstation) service is not running.)"},
	{.status = 3221226144, .dos_err = 628, .posix_err = 0, .msg = R"(The server process is running under a SID different than that required by client.)"},
	{.status = 3221226145, .dos_err = 8202, .posix_err = 0, .msg = R"(The specified directory service attribute or value does not exist.)"},
	{.status = 3221226146, .dos_err = 8203, .posix_err = 0, .msg = R"(The attribute syntax specified to the directory service is invalid.)"},
	{.status = 3221226147, .dos_err = 8204, .posix_err = 0, .msg = R"(The attribute type specified to the directory service is not defined.)"},
	{.status = 3221226148, .dos_err = 8205, .posix_err = 0, .msg = R"(The specified directory service attribute or value already exists.)"},
	{.status = 3221226149, .dos_err = 8206, .posix_err = 0, .msg = R"(The directory service is busy.)"},
	{.status = 3221226150, .dos_err = 8207, .posix_err = 0, .msg = R"(The directory service is not available.)"},
	{.status = 3221226151, .dos_err = 8208, .posix_err = 0, .msg = R"(The directory service was unable to allocate a relative identifier.)"},
	{.status = 3221226152, .dos_err = 8209, .posix_err = 0, .msg = R"(The directory service has exhausted the pool of relative identifiers.)"},
	{.status = 3221226153, .dos_err = 8210, .posix_err = 0, .msg = R"(The requested operation could not be performed because the directory service is not the master for that type of operation.)"},
	{.status = 3221226154, .dos_err = 8211, .posix_err = 0, .msg = R"(The directory service was unable to initialize the subsystem that allocates relative identifiers.)"},
	{.status = 3221226155, .dos_err = 8212, .posix_err = 0, .msg = R"(The requested operation did not satisfy one or more constraints associated with the class of the object.)"},
	{.status = 3221226156, .dos_err = 8213, .posix_err = 0, .msg = R"(The directory service can perform the requested operation only on a leaf object.)"},
	{.status = 3221226157, .dos_err = 8214, .posix_err = 0, .msg = R"(The directory service cannot perform the requested operation on the Relatively Defined Name (RDN) attribute of an object.)"},
	{.status = 3221226158, .dos_err = 8215, .posix_err = 0, .msg = R"(The directory service detected an attempt to modify the object class of an object.)"},
	{.status = 3221226159, .dos_err = 8216, .posix_err = 0, .msg = R"(An error occurred while performing a cross domain move operation.)"},
	{.status = 3221226160, .dos_err = 8217, .posix_err = 0, .msg = R"(Unable to Contact the Global Catalog Server.)"},
	{.status = 3221226161, .dos_err = 8478, .posix_err = 0, .msg = R"(The requested operation requires a directory service, and none was available.)"},
	{.status = 3221226162, .dos_err = 4391, .posix_err = 0, .msg = R"(The reparse attribute cannot be set as it is incompatible with an existing attribute.)"},
	{.status = 3221226163, .dos_err = 629, .posix_err = 0, .msg = R"(A group marked use for deny only cannot be enabled.)"},
	{.status = 3221226164, .dos_err = 630, .posix_err = 0, .msg = R"({EXCEPTION}

Multiple floating point faults.)"},
	{.status = 3221226165, .dos_err = 631, .posix_err = 0, .msg = R"({EXCEPTION}

Multiple floating point traps.)"},
	{.status = 3221226166, .dos_err = 1617, .posix_err = 0, .msg = R"(The device has been removed.)"},
	{.status = 3221226167, .dos_err = 1178, .posix_err = 0, .msg = R"(The volume change journal is being deleted.)"},
	{.status = 3221226168, .dos_err = 1179, .posix_err = 0, .msg = R"(The volume change journal is not active.)"},
	{.status = 3221226169, .dos_err = 632, .posix_err = 0, .msg = R"(The requested interface is not supported.)"},
	{.status = 3221226170, .dos_err = 8263, .posix_err = 0, .msg = R"(The directory service detected the subsystem that allocates relative identifiers is disabled. This can occur as a protective mechanism when the system determines a significant portion of relative identifiers (RIDs) have been exhausted. Please see http://go.microsoft.com/fwlink/?LinkId=228610 for recommended diagnostic steps and the procedure to re-enable account creation.)"},
	{.status = 3221226177, .dos_err = 8228, .posix_err = 0, .msg = R"(A directory service resource limit has been exceeded.)"},
	{.status = 3221226178, .dos_err = 633, .posix_err = 0, .msg = R"({System Standby Failed}

The driver %hs does not support standby mode. Updating this driver may allow the system to go to standby mode.)"},
	{.status = 3221226179, .dos_err = 1397, .posix_err = 0, .msg = R"(Mutual Authentication failed. The server's password is out of date at the domain controller.)"},
	{.status = 3221226180, .dos_err = 634, .posix_err = 0, .msg = R"(The system file %1 has become corrupt and has been replaced.)"},
	{.status = 3221226181, .dos_err = 998, .posix_err = 13, .msg = R"({EXCEPTION}

Alignment Error

A datatype misalignment error was detected in a load or store instruction.)"},
	{.status = 3221226182, .dos_err = 4213, .posix_err = 0, .msg = R"(The WMI data item or data block is read only.)"},
	{.status = 3221226183, .dos_err = 4214, .posix_err = 0, .msg = R"(The WMI data item or data block could not be changed.)"},
	{.status = 3221226184, .dos_err = 635, .posix_err = 0, .msg = R"({Virtual Memory Minimum Too Low}

Your system is low on virtual memory. Windows is increasing the size of your virtual memory paging file. During this process, memory requests for some applications may be denied. For more information, see Help.)"},
	{.status = 3221226185, .dos_err = 1261, .posix_err = 0, .msg = R"({EXCEPTION}

Register NaT consumption faults.

A NaT value is consumed on a non speculative instruction.)"},
	{.status = 3221226186, .dos_err = 4328, .posix_err = 0, .msg = R"(The medium changer's transport element contains media, which is causing the operation to fail.)"},
	{.status = 3221226187, .dos_err = 8504, .posix_err = 0, .msg = R"(Security Accounts Manager initialization failed because of the following error:

%hs

Error Status: 0x%x.

Please shutdown this system and reboot into Directory Services Restore Mode, check the event log for more detailed information.)"},
	{.status = 3221226188, .dos_err = 1251, .posix_err = 0, .msg = R"(This operation is supported only when you are connected to the server.)"},
	{.status = 3221226189, .dos_err = 8505, .posix_err = 0, .msg = R"(Only an administrator can modify the membership list of an administrative group.)"},
	{.status = 3221226190, .dos_err = 636, .posix_err = 0, .msg = R"(A device was removed so enumeration must be restarted.)"},
	{.status = 3221226191, .dos_err = 1181, .posix_err = 0, .msg = R"(The journal entry has been deleted from the journal.)"},
	{.status = 3221226192, .dos_err = 8506, .posix_err = 0, .msg = R"(Cannot change the primary group ID of a domain controller account.)"},
	{.status = 3221226193, .dos_err = 637, .posix_err = 0, .msg = R"({Fatal System Error}

The system image %s is not properly signed. The file has been replaced with the signed file. The system has been shut down.)"},
	{.status = 3221226194, .dos_err = 638, .posix_err = 0, .msg = R"(Device will not start without a reboot.)"},
	{.status = 3221226195, .dos_err = 21, .posix_err = 11, .msg = R"(Current device power state cannot support this request.)"},
	{.status = 3221226196, .dos_err = 8513, .posix_err = 0, .msg = R"(The specified group type is invalid.)"},
	{.status = 3221226197, .dos_err = 8514, .posix_err = 0, .msg = R"(In mixed domain no nesting of global group if group is security enabled.)"},
	{.status = 3221226198, .dos_err = 8515, .posix_err = 0, .msg = R"(In mixed domain, cannot nest local groups with other local groups, if the group is security enabled.)"},
	{.status = 3221226199, .dos_err = 8516, .posix_err = 0, .msg = R"(A global group cannot have a local group as a member.)"},
	{.status = 3221226200, .dos_err = 8517, .posix_err = 0, .msg = R"(A global group cannot have a universal group as a member.)"},
	{.status = 3221226201, .dos_err = 8518, .posix_err = 0, .msg = R"(A universal group cannot have a local group as a member.)"},
	{.status = 3221226202, .dos_err = 8519, .posix_err = 0, .msg = R"(A global group cannot have a cross domain member.)"},
	{.status = 3221226203, .dos_err = 8520, .posix_err = 0, .msg = R"(A local group cannot have another cross domain local group as a member.)"},
	{.status = 3221226204, .dos_err = 8521, .posix_err = 0, .msg = R"(Cannot change to security disabled group because of having primary members in this group.)"},
	{.status = 3221226205, .dos_err = 50, .posix_err = 129, .msg = R"(The WMI operation is not supported by the data block or method.)"},
	{.status = 3221226206, .dos_err = 639, .posix_err = 0, .msg = R"(There is not enough power to complete the requested operation.)"},
	{.status = 3221226207, .dos_err = 8529, .posix_err = 0, .msg = R"(Security Account Manager needs to get the boot password.)"},
	{.status = 3221226208, .dos_err = 8530, .posix_err = 0, .msg = R"(Security Account Manager needs to get the boot key from floppy disk.)"},
	{.status = 3221226209, .dos_err = 8531, .posix_err = 0, .msg = R"(Directory Service cannot start.)"},
	{.status = 3221226210, .dos_err = 8532, .posix_err = 0, .msg = R"(Directory Services could not start because of the following error:

%hs

Error Status: 0x%x.

Please shutdown this system and reboot into Directory Services Restore Mode, check the event log for more detailed information.)"},
	{.status = 3221226211, .dos_err = 8541, .posix_err = 0, .msg = R"(Security Accounts Manager initialization failed because of the following error:

%hs

Error Status: 0x%x.

Please click OK to shutdown this system and reboot into Safe Mode, check the event log for more detailed information.)"},
	{.status = 3221226212, .dos_err = 8547, .posix_err = 0, .msg = R"(The requested operation can be performed only on a global catalog server.)"},
	{.status = 3221226213, .dos_err = 8548, .posix_err = 0, .msg = R"(A local group can only be a member of other local groups in the same domain.)"},
	{.status = 3221226214, .dos_err = 8549, .posix_err = 0, .msg = R"(Foreign security principals cannot be members of universal groups.)"},
	{.status = 3221226215, .dos_err = 8557, .posix_err = 0, .msg = R"(Your computer could not be joined to the domain. You have exceeded the maximum number of computer accounts you are allowed to create in this domain. Contact your system administrator to have this limit reset or increased.)"},
	{.status = 3221226216, .dos_err = 640, .posix_err = 0, .msg = R"(STATUS_MULTIPLE_FAULT_VIOLATION)"},
	{.status = 3221226217, .dos_err = 1399, .posix_err = 0, .msg = R"(This operation cannot be performed on the current domain.)"},
	{.status = 3221226218, .dos_err = 82, .posix_err = 13, .msg = R"(The directory or file cannot be created.)"},
	{.status = 3221226219, .dos_err = 641, .posix_err = 0, .msg = R"(The system is in the process of shutting down.)"},
	{.status = 3221226220, .dos_err = 8561, .posix_err = 0, .msg = R"(Directory Services could not start because of the following error:

%hs

Error Status: 0x%x.

Please click OK to shutdown the system. You can use the recovery console to diagnose the system further.)"},
	{.status = 3221226221, .dos_err = 8562, .posix_err = 0, .msg = R"(Security Accounts Manager initialization failed because of the following error:

%hs

Error Status: 0x%x.

Please click OK to shutdown the system. You can use the recovery console to diagnose the system further.)"},
	{.status = 3221226222, .dos_err = 2148074291, .posix_err = 0, .msg = R"(A security context was deleted before the context was completed. This is considered a logon failure.)"},
	{.status = 3221226223, .dos_err = 2148074292, .posix_err = 0, .msg = R"(The client is trying to negotiate a context and the server requires user-to-user but didn't send a TGT reply.)"},
	{.status = 3221226224, .dos_err = 2, .posix_err = 2, .msg = R"(An object ID was not found in the file.)"},
	{.status = 3221226225, .dos_err = 2148074293, .posix_err = 0, .msg = R"(Unable to accomplish the requested task because the local machine does not have any IP addresses.)"},
	{.status = 3221226226, .dos_err = 2148074294, .posix_err = 0, .msg = R"(The supplied credential handle does not match the credential associated with the security context.)"},
	{.status = 3221226227, .dos_err = 2148074295, .posix_err = 0, .msg = R"(The crypto system or checksum function is invalid because a required function is unavailable.)"},
	{.status = 3221226228, .dos_err = 2148074296, .posix_err = 0, .msg = R"(The number of maximum ticket referrals has been exceeded.)"},
	{.status = 3221226229, .dos_err = 2148074297, .posix_err = 0, .msg = R"(The local machine must be a Kerberos KDC (domain controller) and it is not.)"},
	{.status = 3221226230, .dos_err = 2148074298, .posix_err = 0, .msg = R"(The other end of the security negotiation is requires strong crypto but it is not supported on the local machine.)"},
	{.status = 3221226231, .dos_err = 2148074299, .posix_err = 0, .msg = R"(The KDC reply contained more than one principal name.)"},
	{.status = 3221226232, .dos_err = 2148074300, .posix_err = 0, .msg = R"(Expected to find PA data for a hint of what etype to use, but it was not found.)"},
	{.status = 3221226233, .dos_err = 2148074301, .posix_err = 0, .msg = R"(The client certificate does not contain a valid UPN, or does not match the client name in the logon request. Please contact your administrator.)"},
	{.status = 3221226234, .dos_err = 2148074302, .posix_err = 0, .msg = R"(Smartcard logon is required and was not used.)"},
	{.status = 3221226235, .dos_err = 2148074304, .posix_err = 0, .msg = R"(An invalid request was sent to the KDC.)"},
	{.status = 3221226236, .dos_err = 2148074305, .posix_err = 0, .msg = R"(The KDC was unable to generate a referral for the service requested.)"},
	{.status = 3221226237, .dos_err = 2148074306, .posix_err = 0, .msg = R"(The encryption type requested is not supported by the KDC.)"},
	{.status = 3221226238, .dos_err = 1115, .posix_err = 0, .msg = R"(A system shutdown is in progress.)"},
	{.status = 3221226239, .dos_err = 1255, .posix_err = 0, .msg = R"(The server machine is shutting down.)"},
	{.status = 3221226240, .dos_err = 1254, .posix_err = 0, .msg = R"(This operation is not supported on a computer running Windows Server 2003 for Small Business Server)"},
	{.status = 3221226241, .dos_err = 4207, .posix_err = 0, .msg = R"(The WMI GUID is no longer available)"},
	{.status = 3221226242, .dos_err = 4212, .posix_err = 0, .msg = R"(Collection or events for the WMI GUID is already disabled.)"},
	{.status = 3221226243, .dos_err = 4206, .posix_err = 0, .msg = R"(Collection or events for the WMI GUID is already enabled.)"},
	{.status = 3221226244, .dos_err = 302, .posix_err = 0, .msg = R"(The Master File Table on the volume is too fragmented to complete this operation.)"},
	{.status = 3221226245, .dos_err = 2147681029, .posix_err = 0, .msg = R"(Copy protection failure.)"},
	{.status = 3221226246, .dos_err = 2147681030, .posix_err = 0, .msg = R"(Copy protection error - DVD CSS Authentication failed.)"},
	{.status = 3221226247, .dos_err = 2147681031, .posix_err = 0, .msg = R"(Copy protection error - The given sector does not contain a valid key.)"},
	{.status = 3221226248, .dos_err = 2147681032, .posix_err = 0, .msg = R"(Copy protection error - DVD session key not established.)"},
	{.status = 3221226249, .dos_err = 2147681033, .posix_err = 0, .msg = R"(Copy protection error - The read failed because the sector is encrypted.)"},
	{.status = 3221226250, .dos_err = 2147681034, .posix_err = 0, .msg = R"(Copy protection error - The given DVD's region does not correspond to the

region setting of the drive.)"},
	{.status = 3221226251, .dos_err = 2147681035, .posix_err = 0, .msg = R"(Copy protection error - The drive's region setting may be permanent.)"},
	{.status = 3221226252, .dos_err = 1938, .posix_err = 0, .msg = R"(EAS policy requires that the user change their password before this operation can be performed.)"},
	{.status = 3221226253, .dos_err = 1939, .posix_err = 0, .msg = R"(An administrator has restricted sign in. To sign in, make sure your device is connected to the Internet, and have your administrator sign in first.)"},
	{.status = 3221226272, .dos_err = 1263, .posix_err = 0, .msg = R"(The Kerberos protocol encountered an error while validating the KDC certificate during logon. There is more information in the system event log.)"},
	{.status = 3221226273, .dos_err = 1264, .posix_err = 0, .msg = R"(The Kerberos protocol encountered an error while attempting to utilize the smartcard subsystem.)"},
	{.status = 3221226274, .dos_err = 2148074312, .posix_err = 0, .msg = R"(The target server does not have acceptable Kerberos credentials.)"},
	{.status = 3221226320, .dos_err = 1256, .posix_err = 0, .msg = R"(The transport determined that the remote system is down.)"},
	{.status = 3221226321, .dos_err = 2148074307, .posix_err = 0, .msg = R"(An unsupported preauthentication mechanism was presented to the Kerberos package.)"},
	{.status = 3221226322, .dos_err = 6013, .posix_err = 0, .msg = R"(The encryption algorithm used on the source file needs a bigger key buffer than the one used on the destination file.)"},
	{.status = 3221226323, .dos_err = 642, .posix_err = 0, .msg = R"(An attempt to remove a process's DebugPort was made, but a port was not already associated with the process.)"},
	{.status = 3221226324, .dos_err = 1284, .posix_err = 0, .msg = R"(Debugger Inactive: Windows may have been started without kernel debugging enabled.)"},
	{.status = 3221226325, .dos_err = 643, .posix_err = 0, .msg = R"(This version of Windows is not compatible with the behavior version of directory forest, domain or domain controller.)"},
	{.status = 3221226326, .dos_err = 3221815297, .posix_err = 0, .msg = R"(The specified event is currently not being audited.)"},
	{.status = 3221226327, .dos_err = 8572, .posix_err = 0, .msg = R"(The machine account was created pre-NT4. The account needs to be recreated.)"},
	{.status = 3221226328, .dos_err = 8578, .posix_err = 0, .msg = R"(A account group cannot have a universal group as a member.)"},
	{.status = 3221226329, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format, it appears to be a 32-bit Windows image.)"},
	{.status = 3221226330, .dos_err = 193, .posix_err = 0, .msg = R"(The specified image file did not have the correct format, it appears to be a 64-bit Windows image.)"},
	{.status = 3221226331, .dos_err = 2148074310, .posix_err = 0, .msg = R"(Client's supplied SSPI channel bindings were incorrect.)"},
	{.status = 3221226332, .dos_err = 1394, .posix_err = 0, .msg = R"(The client's session has expired, so the client must reauthenticate to continue accessing the remote resources.)"},
	{.status = 3221226333, .dos_err = 1259, .posix_err = 0, .msg = R"(AppHelp dialog canceled thus preventing the application from starting.)"},
	{.status = 3221226334, .dos_err = 3221815298, .posix_err = 0, .msg = R"(The SID filtering operation removed all SIDs.)"},
	{.status = 3221226335, .dos_err = 646, .posix_err = 0, .msg = R"(The driver was not loaded because the system is booting into safe mode.)"},
	{.status = 3221226337, .dos_err = 1260, .posix_err = 0, .msg = R"(Access to %1 has been restricted by your Administrator by the default software restriction policy level.)"},
	{.status = 3221226338, .dos_err = 1260, .posix_err = 0, .msg = R"(Access to %1 has been restricted by your Administrator by location with policy rule %2 placed on path %3)"},
	{.status = 3221226339, .dos_err = 1260, .posix_err = 0, .msg = R"(Access to %1 has been restricted by your Administrator by software publisher policy.)"},
	{.status = 3221226340, .dos_err = 1260, .posix_err = 0, .msg = R"(Access to %1 has been restricted by your Administrator by policy rule %2.)"},
	{.status = 3221226341, .dos_err = 647, .posix_err = 0, .msg = R"(The driver was not loaded because it failed its initialization call.)"},
	{.status = 3221226342, .dos_err = 648, .posix_err = 0, .msg = R"(The "%hs" encountered an error while applying power or reading the device configuration. This may be caused by a failure of your hardware or by a poor connection.)"},
	{.status = 3221226344, .dos_err = 649, .posix_err = 0, .msg = R"(The create operation failed because the name contained at least one mount point which resolves to a volume to which the specified device object is not attached.)"},
	{.status = 3221226345, .dos_err = 650, .posix_err = 0, .msg = R"(The device object parameter is either not a valid device object or is not attached to the volume specified by the file name.)"},
	{.status = 3221226346, .dos_err = 651, .posix_err = 0, .msg = R"(A Machine Check Error has occurred. Please check the system eventlog for additional information.)"},
	{.status = 3221226347, .dos_err = 1275, .posix_err = 0, .msg = R"(Driver %2 has been blocked from loading.)"},
	{.status = 3221226348, .dos_err = 1275, .posix_err = 0, .msg = R"(Driver %2 has been blocked from loading.)"},
	{.status = 3221226349, .dos_err = 652, .posix_err = 0, .msg = R"(There was error [%2] processing the driver database.)"},
	{.status = 3221226350, .dos_err = 653, .posix_err = 0, .msg = R"(System hive size has exceeded its limit.)"},
	{.status = 3221226351, .dos_err = 1276, .posix_err = 0, .msg = R"(A dynamic link library (DLL) referenced a module that was neither a DLL nor the process's executable image.)"},
	{.status = 3221226353, .dos_err = 8620, .posix_err = 0, .msg = R"(The local account store does not contain secret material for the specified account.)"},
	{.status = 3221226354, .dos_err = 786, .posix_err = 0, .msg = R"(Access to %1 has been restricted by your Administrator by policy rule %2.)"},
	{.status = 3221226355, .dos_err = 8, .posix_err = 12, .msg = R"(The system was not able to allocate enough memory to perform a stack switch.)"},
	{.status = 3221226356, .dos_err = 1359, .posix_err = 0, .msg = R"(A heap has been corrupted.)"},
	{.status = 3221226368, .dos_err = 2148532331, .posix_err = 0, .msg = R"(An incorrect PIN was presented to the smart card)"},
	{.status = 3221226369, .dos_err = 2148532332, .posix_err = 0, .msg = R"(The smart card is blocked)"},
	{.status = 3221226370, .dos_err = 2148532335, .posix_err = 0, .msg = R"(No PIN was presented to the smart card)"},
	{.status = 3221226371, .dos_err = 2148532236, .posix_err = 0, .msg = R"(No smart card available)"},
	{.status = 3221226372, .dos_err = 2148073485, .posix_err = 0, .msg = R"(The requested key container does not exist on the smart card)"},
	{.status = 3221226373, .dos_err = 2148532268, .posix_err = 0, .msg = R"(The requested certificate does not exist on the smart card)"},
	{.status = 3221226374, .dos_err = 2148073494, .posix_err = 0, .msg = R"(The requested keyset does not exist)"},
	{.status = 3221226375, .dos_err = 2148532271, .posix_err = 0, .msg = R"(A communication error with the smart card has been detected.)"},
	{.status = 3221226376, .dos_err = 1265, .posix_err = 0, .msg = R"(The system cannot contact a domain controller to service the authentication request. Please try again later.)"},
	{.status = 3221226377, .dos_err = 2148074321, .posix_err = 0, .msg = R"(The smartcard certificate used for authentication has been revoked. Please contact your system administrator. There may be additional information in the event log.)"},
	{.status = 3221226378, .dos_err = 2148074322, .posix_err = 0, .msg = R"(An untrusted certificate authority was detected while processing the certificate used for authentication.)"},
	{.status = 3221226379, .dos_err = 2148074323, .posix_err = 0, .msg = R"(The revocation status of the certificate used for authentication could not be determined.)"},
	{.status = 3221226380, .dos_err = 2148074324, .posix_err = 0, .msg = R"(The client certificate used for authentication was not trusted.)"},
	{.status = 3221226381, .dos_err = 2148074325, .posix_err = 0, .msg = R"(The smartcard certificate used for authentication has expired. Please

contact your system administrator.)"},
	{.status = 3221226382, .dos_err = 654, .posix_err = 0, .msg = R"(The driver could not be loaded because a previous version of the driver is still in memory.)"},
	{.status = 3221226383, .dos_err = 2148073506, .posix_err = 0, .msg = R"(The smartcard provider could not perform the action since the context was acquired as silent.)"},
	{.status = 3221226497, .dos_err = 1932, .posix_err = 0, .msg = R"(The current user's delegated trust creation quota has been exceeded.)"},
	{.status = 3221226498, .dos_err = 1933, .posix_err = 0, .msg = R"(The total delegated trust creation quota has been exceeded.)"},
	{.status = 3221226499, .dos_err = 1934, .posix_err = 0, .msg = R"(The current user's delegated trust deletion quota has been exceeded.)"},
	{.status = 3221226500, .dos_err = 8571, .posix_err = 0, .msg = R"(The requested name already exists as a unique identifier.)"},
	{.status = 3221226501, .dos_err = 8605, .posix_err = 0, .msg = R"(The requested object has a non-unique identifier and cannot be retrieved.)"},
	{.status = 3221226502, .dos_err = 8607, .posix_err = 0, .msg = R"(The group cannot be converted due to attribute restrictions on the requested group type.)"},
	{.status = 3221226503, .dos_err = 655, .posix_err = 0, .msg = R"({Volume Shadow Copy Service}

Please wait while the Volume Shadow Copy Service prepares volume %hs for hibernation.)"},
	{.status = 3221226504, .dos_err = 1326, .posix_err = 0, .msg = R"(Kerberos sub-protocol User2User is required.)"},
	{.status = 3221226505, .dos_err = 1282, .posix_err = 0, .msg = R"(The system detected an overrun of a stack-based buffer in this application. This overrun could potentially allow a malicious user to gain control of this application.)"},
	{.status = 3221226506, .dos_err = 2148074326, .posix_err = 0, .msg = R"(The Kerberos subsystem encountered an error. A service for user protocol request was made against a domain controller which does not support service for user.)"},
	{.status = 3221226507, .dos_err = 2148074327, .posix_err = 0, .msg = R"(An attempt was made by this server to make a Kerberos constrained delegation request for a target outside of the server's realm. This is not supported, and indicates a misconfiguration on this server's allowed to delegate to list. Please contact your administrator.)"},
	{.status = 3221226508, .dos_err = 2148074328, .posix_err = 0, .msg = R"(The revocation status of the domain controller certificate used for authentication could not be determined. There is additional information in the system event log.)"},
	{.status = 3221226509, .dos_err = 2148074329, .posix_err = 0, .msg = R"(An untrusted certificate authority was detected while processing the domain controller certificate used for authentication. There is additional information in the system event log. Please contact your system administrator.)"},
	{.status = 3221226510, .dos_err = 2148074330, .posix_err = 0, .msg = R"(The domain controller certificate used for logon has expired. There is additional information in the system event log.)"},
	{.status = 3221226511, .dos_err = 2148074331, .posix_err = 0, .msg = R"(The domain controller certificate used for logon has been revoked. There is additional information in the system event log.)"},
	{.status = 3221226512, .dos_err = 1283, .posix_err = 0, .msg = R"(Data present in one of the parameters is more than the function can operate on.)"},
	{.status = 3221226513, .dos_err = 656, .posix_err = 0, .msg = R"(The system has failed to hibernate (The error code is %hs). Hibernation will be disabled until the system is restarted.)"},
	{.status = 3221226514, .dos_err = 1285, .posix_err = 0, .msg = R"(An attempt to delay-load a .dll or get a function address in a delay-loaded .dll failed.)"},
	{.status = 3221226515, .dos_err = 1935, .posix_err = 0, .msg = R"(Logon Failure: The machine you are logging onto is protected by an authentication firewall. The specified account is not allowed to authenticate to the machine.)"},
	{.status = 3221226516, .dos_err = 1286, .posix_err = 0, .msg = R"(%hs is a 16-bit application. You do not have permissions to execute 16-bit applications. Check your permissions with your system administrator.)"},
	{.status = 3221226517, .dos_err = 2149974017, .posix_err = 0, .msg = R"({Display Driver Stopped Responding}

The %hs display driver has stopped working normally. Save your work and reboot the system to restore full display functionality. The next time you reboot the machine a dialog will be displayed giving you a chance to report this failure to Microsoft.)"},
	{.status = 3221226518, .dos_err = 8, .posix_err = 12, .msg = R"(The Desktop heap encountered an error while allocating session memory. There is more information in the system event log.)"},
	{.status = 3221226519, .dos_err = 1288, .posix_err = 0, .msg = R"(An invalid parameter was passed to a C runtime function.)"},
	{.status = 3221226520, .dos_err = 1937, .posix_err = 0, .msg = R"(The authentication failed since NTLM was blocked.)"},
	{.status = 3221226521, .dos_err = 8539, .posix_err = 0, .msg = R"(The source object's SID already exists in destination forest.)"},
	{.status = 3221226522, .dos_err = 8634, .posix_err = 0, .msg = R"(The domain name of the trusted domain already exists in the forest.)"},
	{.status = 3221226523, .dos_err = 8635, .posix_err = 0, .msg = R"(The flat name of the trusted domain already exists in the forest.)"},
	{.status = 3221226524, .dos_err = 8636, .posix_err = 0, .msg = R"(The User Principal Name (UPN) is invalid.)"},
	{.status = 3221226525, .dos_err = 713, .posix_err = 0, .msg = R"(An unhandled exception was encountered during a user callback.)"},
	{.status = 3221226528, .dos_err = 668, .posix_err = 0, .msg = R"(An assertion failure has occurred.)"},
	{.status = 3221226529, .dos_err = 537, .posix_err = 0, .msg = R"(Application verifier has found an error in the current process.)"},
	{.status = 3221226531, .dos_err = 768, .posix_err = 0, .msg = R"(An exception has occurred in a user mode callback and the kernel callback frame should be removed.)"},
	{.status = 3221226532, .dos_err = 1275, .posix_err = 0, .msg = R"(%2 has been blocked from loading due to incompatibility with this system. Please contact your software vendor for a compatible version of the driver.)"},
	{.status = 3221226533, .dos_err = 1018, .posix_err = 0, .msg = R"(Illegal operation attempted on a registry key which has already been unloaded.)"},
	{.status = 3221226534, .dos_err = 769, .posix_err = 0, .msg = R"(Compression is disabled for this volume.)"},
	{.status = 3221226535, .dos_err = 665, .posix_err = 0, .msg = R"(The requested operation could not be completed due to a file system limitation)"},
	{.status = 3221226536, .dos_err = 577, .posix_err = 0, .msg = R"(Windows cannot verify the digital signature for this file. A recent hardware or software change might have installed a file that is signed incorrectly or damaged, or that might be malicious software from an unknown source.)"},
	{.status = 3221226537, .dos_err = 775, .posix_err = 0, .msg = R"(The implementation is not capable of performing the request.)"},
	{.status = 3221226538, .dos_err = 776, .posix_err = 0, .msg = R"(The requested operation is out of order with respect to other operations.)"},
	{.status = 3221226539, .dos_err = 1292, .posix_err = 0, .msg = R"(An operation attempted to exceed an implementation-defined limit.)"},
	{.status = 3221226540, .dos_err = 740, .posix_err = 0, .msg = R"(The requested operation requires elevation.)"},
	{.status = 3221226541, .dos_err = 2148074337, .posix_err = 0, .msg = R"(The required security context does not exist.)"},
	{.status = 3221226543, .dos_err = 2148074338, .posix_err = 0, .msg = R"(The PKU2U protocol encountered an error while attempting to utilize the associated certificates.)"},
	{.status = 3221226546, .dos_err = 1289, .posix_err = 0, .msg = R"(The operation was attempted beyond the valid data length of the file.)"},
	{.status = 3221226547, .dos_err = 170, .posix_err = 16, .msg = R"(The attempted write operation encountered a write already in progress for some portion of the range.)"},
	{.status = 3221226548, .dos_err = 170, .posix_err = 16, .msg = R"(The page fault mappings changed in the middle of processing a fault so the operation must be retried.)"},
	{.status = 3221226549, .dos_err = 1224, .posix_err = 0, .msg = R"(The attempt to purge this file from memory failed to purge some or all the data from memory.)"},
	{.status = 3221226560, .dos_err = 2148102169, .posix_err = 0, .msg = R"(The requested credential requires confirmation.)"},
	{.status = 3221226561, .dos_err = 6017, .posix_err = 0, .msg = R"(The remote server sent an invalid response for a file being opened with Client Side Encryption.)"},
	{.status = 3221226562, .dos_err = 6018, .posix_err = 0, .msg = R"(Client Side Encryption is not supported by the remote server even though it claims to support it.)"},
	{.status = 3221226563, .dos_err = 6019, .posix_err = 0, .msg = R"(File is encrypted and should be opened in Client Side Encryption mode.)"},
	{.status = 3221226564, .dos_err = 6020, .posix_err = 0, .msg = R"(A new encrypted file is being created and a $EFS needs to be provided.)"},
	{.status = 3221226565, .dos_err = 6021, .posix_err = 0, .msg = R"(The SMB client requested a CSE FSCTL on a non-CSE file.)"},
	{.status = 3221226566, .dos_err = 1299, .posix_err = 0, .msg = R"(Indicates a particular Security ID may not be assigned as the label of an object.)"},
	{.status = 3221226576, .dos_err = 1291, .posix_err = 0, .msg = R"(The process hosting the driver for this device has terminated.)"},
	{.status = 3221226577, .dos_err = 15250, .posix_err = 0, .msg = R"(The requested system device cannot be identified due to multiple indistinguishable devices potentially matching the identification criteria.)"},
	{.status = 3221226578, .dos_err = 15299, .posix_err = 0, .msg = R"(The requested system device cannot be found.)"},
	{.status = 3221226579, .dos_err = 1467, .posix_err = 0, .msg = R"(This boot application must be restarted.)"},
	{.status = 3221226580, .dos_err = 1470, .posix_err = 0, .msg = R"(Insufficient NVRAM resources exist to complete the API.  A reboot might be required.)"},
	{.status = 3221226581, .dos_err = 6, .posix_err = 22, .msg = R"(The specified session is invalid.)"},
	{.status = 3221226582, .dos_err = 87, .posix_err = 22, .msg = R"(The specified thread is already in a session.)"},
	{.status = 3221226583, .dos_err = 87, .posix_err = 22, .msg = R"(The specified thread is not in a session.)"},
	{.status = 3221226584, .dos_err = 87, .posix_err = 22, .msg = R"(The specified weight is invalid.)"},
	{.status = 3221226585, .dos_err = 3050, .posix_err = 0, .msg = R"(The operation was paused.)"},
	{.status = 3221226592, .dos_err = 312, .posix_err = 0, .msg = R"(No ranges for the specified operation were able to be processed.)"},
	{.status = 3221226593, .dos_err = 314, .posix_err = 0, .msg = R"(The physical resources of this disk have been exhausted.)"},
	{.status = 3221226594, .dos_err = 15612, .posix_err = 0, .msg = R"(The application cannot be started. Try reinstalling the application to fix the problem.)"},
	{.status = 3221226595, .dos_err = 316, .posix_err = 0, .msg = R"({Device Feature Not Supported}

The device does not support the command feature.)"},
	{.status = 3221226596, .dos_err = 321, .posix_err = 0, .msg = R"({Source/Destination device unreachable}

The device is unreachable.)"},
	{.status = 3221226597, .dos_err = 315, .posix_err = 0, .msg = R"({Invalid Proxy Data Token}

The token representing the data is invalid.)"},
	{.status = 3221226598, .dos_err = 64, .posix_err = 0, .msg = R"(The file server is temporarily unavailable.)"},
	{.status = 3221226599, .dos_err = 32, .posix_err = 13, .msg = R"(The file is temporarily unavailable.)"},
	{.status = 3221226600, .dos_err = 322, .posix_err = 0, .msg = R"({Device Insufficient Resources}

The target device has insufficient resources to complete the operation.)"},
	{.status = 3221226601, .dos_err = 15616, .posix_err = 0, .msg = R"(The application cannot be started because it is currently updating.)"},
	{.status = 3221226602, .dos_err = 337, .posix_err = 0, .msg = R"(The specified copy of the requested data could not be read.)"},
	{.status = 3221226603, .dos_err = 338, .posix_err = 0, .msg = R"(The specified data could not be written to any of the copies.)"},
	{.status = 3221226604, .dos_err = 339, .posix_err = 0, .msg = R"(One or more copies of data on this device may be out of sync. No writes may be performed until a data integrity scan is completed.)"},
	{.status = 3221226605, .dos_err = 342, .posix_err = 0, .msg = R"(This object is not externally backed by any provider.)"},
	{.status = 3221226606, .dos_err = 343, .posix_err = 0, .msg = R"(The external backing provider is not recognized.)"},
	{.status = 3221226607, .dos_err = 344, .posix_err = 0, .msg = R"(Compressing this object would not save space.)"},
	{.status = 3221226608, .dos_err = 323, .posix_err = 0, .msg = R"(A data integrity checksum error occurred. Data in the file stream is corrupt.)"},
	{.status = 3221226609, .dos_err = 324, .posix_err = 0, .msg = R"(An attempt was made to modify both a KERNEL and normal Extended Attribute (EA) in the same operation.)"},
	{.status = 3221226610, .dos_err = 326, .posix_err = 0, .msg = R"({LogicalBlockProvisioningReadZero Not Supported}

The target device does not support read returning zeros from trimmed/unmapped blocks.)"},
	{.status = 3221226611, .dos_err = 331, .posix_err = 0, .msg = R"({Maximum Segment Descriptors Exceeded}

The command specified a number of descriptors that exceeded the maximum supported by the device.)"},
	{.status = 3221226612, .dos_err = 327, .posix_err = 0, .msg = R"({Alignment Violation}

The command specified a data offset that does not align to the device's granularity/alignment.)"},
	{.status = 3221226613, .dos_err = 328, .posix_err = 0, .msg = R"({Invalid Field In Parameter List}

The command specified an invalid field in its parameter list.)"},
	{.status = 3221226614, .dos_err = 329, .posix_err = 0, .msg = R"({Operation In Progress}

An operation is currently in progress with the device.)"},
	{.status = 3221226615, .dos_err = 330, .posix_err = 0, .msg = R"({Invalid I_T Nexus}

An attempt was made to send down the command via an invalid path to the target device.)"},
	{.status = 3221226616, .dos_err = 332, .posix_err = 0, .msg = R"(Scrub is disabled on the specified file.)"},
	{.status = 3221226617, .dos_err = 333, .posix_err = 0, .msg = R"(The storage device does not provide redundancy.)"},
	{.status = 3221226618, .dos_err = 334, .posix_err = 0, .msg = R"(An operation is not supported on a resident file.)"},
	{.status = 3221226619, .dos_err = 335, .posix_err = 0, .msg = R"(An operation is not supported on a compressed file.)"},
	{.status = 3221226620, .dos_err = 336, .posix_err = 0, .msg = R"(An operation is not supported on a directory.)"},
	{.status = 3221226621, .dos_err = 1460, .posix_err = 138, .msg = R"({IO Operation Timeout}

The specified I/O operation failed to complete within the expected time period.)"},
	{.status = 3221226622, .dos_err = 15623, .posix_err = 0, .msg = R"(An error in a system binary was detected. Try refreshing the PC to fix the problem.)"},
	{.status = 3221226623, .dos_err = 15624, .posix_err = 0, .msg = R"(A corrupted CLR NGEN binary was detected on the system.)"},
	{.status = 3221226624, .dos_err = 64, .posix_err = 0, .msg = R"(The share is temporarily unavailable.)"},
	{.status = 3221226625, .dos_err = 126, .posix_err = 0, .msg = R"(The target dll was not found because the apiset %hs is not hosted.)"},
	{.status = 3221226626, .dos_err = 126, .posix_err = 0, .msg = R"(The API set extension contains a host for a non-existent API set.)"},
	{.status = 3221226627, .dos_err = 483, .posix_err = 0, .msg = R"(The request failed due to a fatal device hardware error.)"},
	{.status = 3221226628, .dos_err = 2147680776, .posix_err = 0, .msg = R"(The specified firmware slot is invalid.)"},
	{.status = 3221226629, .dos_err = 2147680777, .posix_err = 0, .msg = R"(The specified firmware image is invalid.)"},
	{.status = 3221226630, .dos_err = 345, .posix_err = 0, .msg = R"(The request failed due to a storage topology ID mismatch.)"},
	{.status = 3221226631, .dos_err = 31, .posix_err = 0, .msg = R"(The specified Windows Image (WIM) is not marked as bootable.)"},
	{.status = 3221226632, .dos_err = 346, .posix_err = 0, .msg = R"(The operation was blocked by parental controls.)"},
	{.status = 3221226633, .dos_err = 15631, .posix_err = 0, .msg = R"(The deployment operation failed because the specified application needs to be registered first.)"},
	{.status = 3221226634, .dos_err = 810, .posix_err = 0, .msg = R"(The requested operation failed due to quota operation is still in progress.)"},
	{.status = 3221226635, .dos_err = 812, .posix_err = 0, .msg = R"(The callback function must be invoked inline.)"},
	{.status = 3221226636, .dos_err = 347, .posix_err = 0, .msg = R"(A file system block being referenced has already reached the maximum reference count and can't be referenced any further.)"},
	{.status = 3221226637, .dos_err = 348, .posix_err = 0, .msg = R"(The requested operation failed because the file stream is marked to disallow writes.)"},
	{.status = 3221226638, .dos_err = 354, .posix_err = 0, .msg = R"(Windows Information Protection policy does not allow access to this network resource.)"},
	{.status = 3221226639, .dos_err = 349, .posix_err = 0, .msg = R"(The requested operation failed with an architecture-specific failure code.)"},
	{.status = 3221226640, .dos_err = 1169, .posix_err = 0, .msg = R"(There are no compatible drivers available for this device.)"},
	{.status = 3221226641, .dos_err = 2, .posix_err = 2, .msg = R"(The specified driver package cannot be found on the system.)"},
	{.status = 3221226642, .dos_err = 1168, .posix_err = 0, .msg = R"(The driver package cannot find a required driver configuration.)"},
	{.status = 3221226643, .dos_err = 1170, .posix_err = 0, .msg = R"(The driver configuration is incomplete for use with this device.)"},
	{.status = 3221226644, .dos_err = 775, .posix_err = 0, .msg = R"(The device requires a driver configuration with a function driver.)"},
	{.status = 3221226645, .dos_err = 21, .posix_err = 11, .msg = R"(The device is pending further configuration.)"},
	{.status = 3221226646, .dos_err = 355, .posix_err = 0, .msg = R"(The device hint name buffer is too small to receive the remaining name.)"},
	{.status = 3221226647, .dos_err = 15706, .posix_err = 0, .msg = R"(The package is currently not available.)"},
	{.status = 3221226649, .dos_err = 359, .posix_err = 0, .msg = R"(The device is in maintenance mode.)"},
	{.status = 3221226650, .dos_err = 360, .posix_err = 0, .msg = R"(This operation is not supported on a DAX volume.)"},
	{.status = 3221226651, .dos_err = 302, .posix_err = 0, .msg = R"(The free space on the volume is too fragmented to complete this operation.)"},
	{.status = 3221226652, .dos_err = 361, .posix_err = 0, .msg = R"(The volume has active DAX mappings.)"},
	{.status = 3221226653, .dos_err = 367, .posix_err = 0, .msg = R"(The process creation has been blocked.)"},
	{.status = 3221226654, .dos_err = 368, .posix_err = 0, .msg = R"(The storage device has lost data or persistence.)"},
	{.status = 3221226655, .dos_err = 1183, .posix_err = 0, .msg = R"(Driver Verifier Volatile settings cannot be set when CFG and IO are enabled.)"},
	{.status = 3221226656, .dos_err = 1184, .posix_err = 0, .msg = R"(An attempt was made to access a partition that has begun termination.)"},
	{.status = 3221226657, .dos_err = 399, .posix_err = 0, .msg = R"(An externally encrypted syskey has been configured, but the system no longer supports this feature.  Please see https://go.microsoft.com/fwlink/?linkid=851152 for more information.)"},
	{.status = 3221226658, .dos_err = 815, .posix_err = 0, .msg = R"(An attempt was made to access protected memory in violation of its secure access policy.)"},
	{.status = 3221226659, .dos_err = 406, .posix_err = 0, .msg = R"(The read or write operation to an encrypted file could not be completed because the file can only be accessed when the device is unlocked.)"},
	{.status = 3221226660, .dos_err = 407, .posix_err = 0, .msg = R"(The volume is not cluster aligned on the disk.)"},
	{.status = 3221226661, .dos_err = 408, .posix_err = 0, .msg = R"(No physically aligned free space was found on the volume.)"},
	{.status = 3221226662, .dos_err = 409, .posix_err = 0, .msg = R"(The APPX file can not be accessed because it is not encrypted as expected.)"},
	{.status = 3221226663, .dos_err = 410, .posix_err = 0, .msg = R"(A read or write of raw encrypted data cannot be performed because the file is not encrypted.)"},
	{.status = 3221226664, .dos_err = 411, .posix_err = 0, .msg = R"(An invalid file offset in the encrypted data info block was passed for read or write operation of file's raw encrypted data.)"},
	{.status = 3221226665, .dos_err = 412, .posix_err = 0, .msg = R"(An invalid offset and length combination in the encrypted data info was passed for read or write operation of file's raw encrypted data.)"},
	{.status = 3221226666, .dos_err = 413, .posix_err = 0, .msg = R"(An invalid parameter in the encrypted data info was passed for read or write operation of file's raw encrypted data.)"},
	{.status = 3221226667, .dos_err = 415, .posix_err = 0, .msg = R"(The specified data could not be read from any of the copies.)"},
	{.status = 3221226668, .dos_err = 33, .posix_err = 39, .msg = R"(A system patch could not be applied due to conflicting accesses to the system image.)"},
	{.status = 3221226669, .dos_err = 416, .posix_err = 0, .msg = R"(The specified storage reserve ID is invalid.)"},
	{.status = 3221226670, .dos_err = 417, .posix_err = 0, .msg = R"(The specified storage reserve does not exist.)"},
	{.status = 3221226671, .dos_err = 418, .posix_err = 0, .msg = R"(The specified storage reserve already exists.)"},
	{.status = 3221226672, .dos_err = 419, .posix_err = 0, .msg = R"(The specified storage reserve is not empty.)"},
	{.status = 3221226673, .dos_err = 420, .posix_err = 0, .msg = R"(This operation requires a DAX volume.)"},
	{.status = 3221226674, .dos_err = 421, .posix_err = 0, .msg = R"(This stream is not DAX mappable.)"},
	{.status = 3221226675, .dos_err = 424, .posix_err = 0, .msg = R"(This directory contains entries whose names differ only in case.)"},
	{.status = 3221226676, .dos_err = 425, .posix_err = 0, .msg = R"(The file cannot be safely opened because it is not supported by this version of Windows.)"},
	{.status = 3221226677, .dos_err = 429, .posix_err = 0, .msg = R"(This operation is not supported with BTT enabled.)"},
	{.status = 3221226678, .dos_err = 430, .posix_err = 0, .msg = R"(This operation cannot be performed because encryption is currently disabled.)"},
	{.status = 3221226679, .dos_err = 431, .posix_err = 0, .msg = R"(This encryption operation cannot be performed on filesystem metadata.)"},
	{.status = 3221226680, .dos_err = 432, .posix_err = 0, .msg = R"(Encryption cannot be cleared on this file/directory because it still has an encrypted attribute.)"},
	{.status = 3221226681, .dos_err = 441, .posix_err = 0, .msg = R"(The operation could not be completed due to one or more unsatisfied dependencies.)"},
	{.status = 3221226682, .dos_err = 442, .posix_err = 0, .msg = R"(The file cannot be opened because the path has a case-sensitive directory.)"},
	{.status = 3221226685, .dos_err = 488, .posix_err = 0, .msg = R"(The volume contains paging, crash dump or other system critical files.)"},
	{.status = 3221226686, .dos_err = 44, .posix_err = 0, .msg = R"(The path cannot be traversed because it contains an untrusted mount point.)"},
	{.status = 3221226752, .dos_err = 1550, .posix_err = 0, .msg = R"(The specified task name is invalid.)"},
	{.status = 3221226753, .dos_err = 1551, .posix_err = 0, .msg = R"(The specified task index is invalid.)"},
	{.status = 3221226754, .dos_err = 1552, .posix_err = 0, .msg = R"(The specified thread is already joining a task.)"},
	{.status = 3221226755, .dos_err = 21, .posix_err = 11, .msg = R"(A callback has requested to bypass native code.)"},
	{.status = 3221226756, .dos_err = 319, .posix_err = 0, .msg = R"(The Central Access Policy specified is not defined on the target machine.)"},
	{.status = 3221226757, .dos_err = 320, .posix_err = 0, .msg = R"(The Central Access Policy obtained from Active Directory is invalid.)"},
	{.status = 3221226758, .dos_err = 1471, .posix_err = 0, .msg = R"(Unable to finish the requested operation because the specified process is not a GUI process.)"},
	{.status = 3221226759, .dos_err = 170, .posix_err = 16, .msg = R"(The device is not responding and cannot be safely removed.)"},
	{.status = 3221226760, .dos_err = 1504, .posix_err = 0, .msg = R"(The specified Job already has a container assigned to it.)"},
	{.status = 3221226761, .dos_err = 1505, .posix_err = 0, .msg = R"(The specified Job does not have a container assigned to it.)"},
	{.status = 3221226762, .dos_err = 2147680778, .posix_err = 0, .msg = R"(The device is unresponsive.)"},
	{.status = 3221226763, .dos_err = 4395, .posix_err = 0, .msg = R"(The object manager encountered a reparse point while retrieving an object.)"},
	{.status = 3221226764, .dos_err = 2156068874, .posix_err = 0, .msg = R"(The requested attribute is not present on the specified file or directory.)"},
	{.status = 3221226765, .dos_err = 2156068873, .posix_err = 0, .msg = R"(This volume is not a tiered volume.)"},
	{.status = 3221226766, .dos_err = 4444, .posix_err = 0, .msg = R"(This file is currently associated with a different stream id.)"},
	{.status = 3221226767, .dos_err = 4307, .posix_err = 0, .msg = R"(The requested operation could not be completed because the specified job has children.)"},
	{.status = 3221226768, .dos_err = 1247, .posix_err = 0, .msg = R"(The specified object has already been initialized.)"},
	{.status = 3221226769, .dos_err = 814, .posix_err = 0, .msg = R"(The specified enclave has not yet been terminated.)"},
	{.status = 3221226770, .dos_err = 5, .posix_err = 13, .msg = R"(An attempt was made to access an enclave that has begun termination.)"},
	{.status = 3221226771, .dos_err = 384, .posix_err = 0, .msg = R"(You can't connect to the file share because it's not secure. This share requires the obsolete SMB1 protocol, which is unsafe and could expose your system to attack.

Your system requires SMB2 or higher. For more info on resolving this issue, see: https://go.microsoft.com/fwlink/?linkid=852747)"},
	{.status = 3221226772, .dos_err = 4445, .posix_err = 0, .msg = R"(The volume must undergo garbage collection.)"},
	{.status = 3221226773, .dos_err = 3221226773, .posix_err = 0, .msg = R"(Execution of the thread was interrupted.)"},
	{.status = 3221226774, .dos_err = 87, .posix_err = 22, .msg = R"(The target thread is not currently running.)"},
	{.status = 3221226776, .dos_err = 510, .posix_err = 0, .msg = R"(The file system encountered a metadata file with inconsistent data.)"},
	{.status = 3221227010, .dos_err = 1653, .posix_err = 0, .msg = R"({Fail Fast Exception}

A fail fast exception occurred. Exception handlers will not be invoked and the process will be terminated immediately.)"},
	{.status = 3221227011, .dos_err = 2148204812, .posix_err = 0, .msg = R"(Windows cannot verify the digital signature for this file. The signing certificate for this file has been revoked.)"},
	{.status = 3221227012, .dos_err = 1655, .posix_err = 0, .msg = R"(The operation was blocked as the process prohibits dynamic code generation.)"},
	{.status = 3221227013, .dos_err = 2148204801, .posix_err = 0, .msg = R"(Windows cannot verify the digital signature for this file. The signing certificate for this file has expired.)"},
	{.status = 3221227014, .dos_err = 1657, .posix_err = 0, .msg = R"(The specified image file was blocked from loading because it does not enable a feature required by the process: Control Flow Guard.)"},
	{.status = 3221227018, .dos_err = 1660, .posix_err = 0, .msg = R"(The thread context could not be updated because this has been restricted for the process.)"},
	{.status = 3221227019, .dos_err = 1661, .posix_err = 0, .msg = R"(An attempt to access another partition's private file/section was rejected.)"},
	{.status = 3221227264, .dos_err = 1359, .posix_err = 0, .msg = R"(The ALPC port is closed.)"},
	{.status = 3221227265, .dos_err = 1359, .posix_err = 0, .msg = R"(The ALPC message requested is no longer available.)"},
	{.status = 3221227266, .dos_err = 87, .posix_err = 22, .msg = R"(The ALPC message supplied is invalid.)"},
	{.status = 3221227267, .dos_err = 1359, .posix_err = 0, .msg = R"(The ALPC message has been canceled.)"},
	{.status = 3221227268, .dos_err = 50, .posix_err = 129, .msg = R"(Invalid recursive dispatch attempt.)"},
	{.status = 3221227269, .dos_err = 87, .posix_err = 22, .msg = R"(No receive buffer has been supplied in a synchronous request.)"},
	{.status = 3221227270, .dos_err = 87, .posix_err = 22, .msg = R"(The connection port is used in an invalid context.)"},
	{.status = 3221227271, .dos_err = 50, .posix_err = 129, .msg = R"(The ALPC port does not accept new request messages.)"},
	{.status = 3221227272, .dos_err = 1359, .posix_err = 0, .msg = R"(The resource requested is already in use.)"},
	{.status = 3221227273, .dos_err = 779, .posix_err = 0, .msg = R"(The hardware has reported an uncorrectable memory error.)"},
	{.status = 3221227274, .dos_err = 6, .posix_err = 22, .msg = R"(Status 0x)"},
	{.status = 3221227275, .dos_err = 6, .posix_err = 22, .msg = R"(After a callback to 0x%p(0x%p), a completion call to SetEvent(0x%p) failed with status 0x)"},
	{.status = 3221227276, .dos_err = 6, .posix_err = 22, .msg = R"(After a callback to 0x%p(0x%p), a completion call to ReleaseSemaphore(0x%p, %d) failed with status 0x)"},
	{.status = 3221227277, .dos_err = 6, .posix_err = 22, .msg = R"(After a callback to 0x%p(0x%p), a completion call to ReleaseMutex(%p) failed with status 0x)"},
	{.status = 3221227278, .dos_err = 6, .posix_err = 22, .msg = R"(After a callback to 0x%p(0x%p), an completion call to FreeLibrary(%p) failed with status 0x)"},
	{.status = 3221227279, .dos_err = 6, .posix_err = 22, .msg = R"(The threadpool 0x%p was released while a thread was posting a callback to 0x%p(0x%p) to it.)"},
	{.status = 3221227280, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread is impersonating a client, after a callback to 0x%p(0x%p).

This is unexpected, indicating that the callback is missing a call to revert the impersonation.)"},
	{.status = 3221227281, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread is impersonating a client, after executing an APC.

This is unexpected, indicating that the APC is missing a call to revert the impersonation.)"},
	{.status = 3221227282, .dos_err = 1293, .posix_err = 0, .msg = R"(Either the target process, or the target thread's containing process, is a protected process.)"},
	{.status = 3221227283, .dos_err = 784, .posix_err = 0, .msg = R"(A Thread is getting dispatched with MCA EXCEPTION because of MCA.)"},
	{.status = 3221227284, .dos_err = 1326, .posix_err = 0, .msg = R"(The client certificate account mapping is not unique.)"},
	{.status = 3221227285, .dos_err = 1463, .posix_err = 0, .msg = R"(The symbolic link cannot be followed because its type is disabled.)"},
	{.status = 3221227286, .dos_err = 123, .posix_err = 2, .msg = R"(Indicates that the specified string is not valid for IDN normalization.)"},
	{.status = 3221227287, .dos_err = 1113, .posix_err = 0, .msg = R"(No mapping for the Unicode character exists in the target multi-byte code page.)"},
	{.status = 3221227288, .dos_err = 1359, .posix_err = 0, .msg = R"(The provided callback is already registered.)"},
	{.status = 3221227289, .dos_err = 1359, .posix_err = 0, .msg = R"(The provided context did not match the target.)"},
	{.status = 3221227290, .dos_err = 1359, .posix_err = 0, .msg = R"(The specified port already has a completion list.)"},
	{.status = 3221227291, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread enter a callback at thread base priority 0x%x and exited at priority 0x%x.

This is unexpected, indicating that the callback missed restoring the priority.)"},
	{.status = 3221227292, .dos_err = 87, .posix_err = 22, .msg = R"(An invalid thread, handle %p, is specified for this operation. Possibly, a threadpool worker thread was specified.)"},
	{.status = 3221227293, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread enter a callback, which left transaction state.

This is unexpected, indicating that the callback missed clearing the transaction.)"},
	{.status = 3221227294, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread enter a callback, which left the loader lock held.

This is unexpected, indicating that the callback missed releasing the lock.)"},
	{.status = 3221227295, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread enter a callback, which left with preferred languages set.

This is unexpected, indicating that the callback missed clearing them.)"},
	{.status = 3221227296, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread enter a callback, which left with background priorities set.

This is unexpected, indicating that the callback missed restoring the original priorities.)"},
	{.status = 3221227297, .dos_err = 1, .posix_err = 40, .msg = R"(A threadpool worker thread enter a callback at thread affinity %p and exited at affinity %p.

This is unexpected, indicating that the callback missed restoring the priority.)"},
	{.status = 3221227298, .dos_err = 1835, .posix_err = 0, .msg = R"(The caller has exceeded the maximum number of handles that may be transmitted in

a single local procedure call.)"},
	{.status = 3221227299, .dos_err = 31, .posix_err = 0, .msg = R"(A write to executable memory occurred for a process that is managing such operations.)"},
	{.status = 3221227300, .dos_err = 31, .posix_err = 0, .msg = R"(A write to executable memory occurred from kernel mode for a process that is managing such operations.)"},
	{.status = 3221227301, .dos_err = 31, .posix_err = 0, .msg = R"(A write to executable memory occurred from kernel mode while attached to a process that is managing such operations.)"},
	{.status = 3221227302, .dos_err = 31, .posix_err = 0, .msg = R"(A write to executable memory was triggered cross-process to a process that is managing such operations.)"},
	{.status = 3221227520, .dos_err = 780, .posix_err = 0, .msg = R"(The attempted operation required self healing to be enabled.)"},
	{.status = 3221227521, .dos_err = 8612, .posix_err = 0, .msg = R"(The Directory Service cannot perform the requested operation because a domain rename operation is in progress.)"},
	{.status = 3221227522, .dos_err = 1295, .posix_err = 0, .msg = R"(The requested file operation failed because the storage quota was exceeded.

To free up disk space, move files to a different location or delete unnecessary files. For more information, contact your system administrator.)"},
	{.status = 3221227524, .dos_err = 1296, .posix_err = 0, .msg = R"(The requested file operation failed because the storage policy blocks that type of file. For more information, contact your system administrator.)"},
	{.status = 3221227525, .dos_err = 6849, .posix_err = 0, .msg = R"(The operation could not be completed due to bad clusters on disk.)"},
	{.status = 3221227526, .dos_err = 6851, .posix_err = 0, .msg = R"(The operation could not be completed because the volume is dirty. Please run chkdsk and try again.)"},
	{.status = 3221227528, .dos_err = 793, .posix_err = 0, .msg = R"(The volume repair was not successful.)"},
	{.status = 3221227529, .dos_err = 794, .posix_err = 0, .msg = R"(One of the volume corruption logs is full. Further corruptions that may be detected won't be logged.)"},
	{.status = 3221227530, .dos_err = 795, .posix_err = 0, .msg = R"(One of the volume corruption logs is internally corrupted and needs to be recreated. The volume may contain undetected corruptions and must be scanned.)"},
	{.status = 3221227531, .dos_err = 796, .posix_err = 0, .msg = R"(One of the volume corruption logs is unavailable for being operated on.)"},
	{.status = 3221227532, .dos_err = 797, .posix_err = 0, .msg = R"(One of the volume corruption logs was deleted while still having corruption records in them. The volume contains detected corruptions and must be scanned.)"},
	{.status = 3221227533, .dos_err = 798, .posix_err = 0, .msg = R"(One of the volume corruption logs was cleared by chkdsk and no longer contains real corruptions.)"},
	{.status = 3221227534, .dos_err = 799, .posix_err = 0, .msg = R"(Orphaned files exist on the volume but could not be recovered because no more new names could be created in the recovery directory. Files must be moved from the recovery directory.)"},
	{.status = 3221227535, .dos_err = 1237, .posix_err = 11, .msg = R"(The operation could not be completed because an instance of Proactive Scanner is currently running.)"},
	{.status = 3221227536, .dos_err = 808, .posix_err = 0, .msg = R"(The read or write operation to an encrypted file could not be completed because the file has not been opened for data access.)"},
	{.status = 3221227537, .dos_err = 1359, .posix_err = 0, .msg = R"(One of the volume corruption logs comes from a newer version of Windows and contains corruption records. The log will be emptied and reset to the current version, and the volume health state will be updated accordingly.)"},
	{.status = 3221227777, .dos_err = 220, .posix_err = 0, .msg = R"(This file is checked out or locked for editing by another user.)"},
	{.status = 3221227778, .dos_err = 221, .posix_err = 0, .msg = R"(The file must be checked out before saving changes.)"},
	{.status = 3221227779, .dos_err = 222, .posix_err = 0, .msg = R"(The file type being saved or retrieved has been blocked.)"},
	{.status = 3221227780, .dos_err = 223, .posix_err = 0, .msg = R"(The file size exceeds the limit allowed and cannot be saved.)"},
	{.status = 3221227781, .dos_err = 224, .posix_err = 0, .msg = R"(Access Denied. Before opening files in this location, you must first browse to the web site and select the option to login automatically.)"},
	{.status = 3221227782, .dos_err = 225, .posix_err = 0, .msg = R"(Operation did not complete successfully because the file contains a virus or potentially unwanted software.)"},
	{.status = 3221227783, .dos_err = 226, .posix_err = 0, .msg = R"(This file contains a virus or potentially unwanted software and cannot be opened. Due to the nature of this virus or potentially unwanted software, the file has been removed from this location.)"},
	{.status = 3221227784, .dos_err = 791, .posix_err = 0, .msg = R"(The resources required for this device conflict with the MCFG table.)"},
	{.status = 3221227785, .dos_err = 802, .posix_err = 0, .msg = R"(The operation did not complete successfully because it would cause an oplock to be broken. The caller has requested that existing oplocks not be broken.)"},
	{.status = 3221227786, .dos_err = 2148073475, .posix_err = 0, .msg = R"(Bad key.)"},
	{.status = 3221227787, .dos_err = 2148073477, .posix_err = 0, .msg = R"(Bad data.)"},
	{.status = 3221227788, .dos_err = 2148073485, .posix_err = 0, .msg = R"(Key does not exist.)"},
	{.status = 3221227792, .dos_err = 806, .posix_err = 0, .msg = R"(Access to the specified file handle has been revoked.)"},
	{.status = 3221264536, .dos_err = 670, .posix_err = 0, .msg = R"(WOW Assertion Error.)"},
	{.status = 3221266432, .dos_err = 2148073478, .posix_err = 0, .msg = R"(The cryptographic signature is invalid.)"},
	{.status = 3221266433, .dos_err = 2148073519, .posix_err = 0, .msg = R"(The cryptographic provider does not support HMAC.)"},
	{.status = 3221266434, .dos_err = 23, .posix_err = 0, .msg = R"(The computed authentication tag did not match the input authentication tag.)"},
	{.status = 3221266435, .dos_err = 5023, .posix_err = 0, .msg = R"(The requested state transition is invalid and cannot be performed.)"},
	{.status = 3221266436, .dos_err = 340, .posix_err = 0, .msg = R"(The supplied kernel information version is invalid.)"},
	{.status = 3221266437, .dos_err = 341, .posix_err = 0, .msg = R"(The supplied PEP information version is invalid.)"},
	{.status = 3221266438, .dos_err = 811, .posix_err = 0, .msg = R"(Access to the specified handle has been revoked.)"},
	{.status = 3221266439, .dos_err = 50, .posix_err = 129, .msg = R"(The file operation will result in the end of file being on a ghosted range.)"},
	{.status = 3221266440, .dos_err = 443, .posix_err = 0, .msg = R"(Filesystem needs to release all the file resources and callback into Cc to drain pending section deletions, before retrying the operation.)"},
	{.status = 3221266448, .dos_err = 234, .posix_err = 0, .msg = R"(The IPSEC queue overflowed.)"},
	{.status = 3221266449, .dos_err = 234, .posix_err = 0, .msg = R"(The neighbor discovery queue overflowed.)"},
	{.status = 3221266450, .dos_err = 1232, .posix_err = 0, .msg = R"(An ICMP hop limit exceeded error was received.)"},
	{.status = 3221266451, .dos_err = 50, .posix_err = 129, .msg = R"(The protocol is not installed on the local machine.)"},
	{.status = 3221266452, .dos_err = 1233, .posix_err = 0, .msg = R"(An operation or data has been rejected while on the network fast path.)"},
	{.status = 3221266560, .dos_err = 788, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs; the data has been lost.

This error may be caused by network connectivity issues. Please try to save this file elsewhere.)"},
	{.status = 3221266561, .dos_err = 789, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs; the data has been lost.

This error was returned by the server on which the file exists. Please try to save this file elsewhere.)"},
	{.status = 3221266562, .dos_err = 790, .posix_err = 0, .msg = R"({Delayed Write Failed}

Windows was unable to save all the data for the file %hs; the data has been lost.

This error may be caused if the device has been removed or the media is write-protected.)"},
	{.status = 3221266563, .dos_err = 1465, .posix_err = 0, .msg = R"(Windows was unable to parse the requested XML data.)"},
	{.status = 3221266564, .dos_err = 1466, .posix_err = 0, .msg = R"(An error was encountered while processing an XML digital signature.)"},
	{.status = 3221266565, .dos_err = 1468, .posix_err = 0, .msg = R"(Indicates that the caller made the connection request in the wrong routing compartment.)"},
	{.status = 3221266566, .dos_err = 1469, .posix_err = 0, .msg = R"(Indicates that there was an AuthIP failure when attempting to connect to the remote host.)"},
	{.status = 3221266567, .dos_err = 8637, .posix_err = 0, .msg = R"(OID mapped groups cannot have members.)"},
	{.status = 3221266568, .dos_err = 8638, .posix_err = 0, .msg = R"(The specified OID cannot be found.)"},
	{.status = 3221266569, .dos_err = 8646, .posix_err = 0, .msg = R"(The system is not authoritative for the specified account and therefore cannot complete the operation. Please retry the operation using the provider associated with this account. If this is an online provider please use the provider's online site.)"},
	{.status = 3221266570, .dos_err = 8653, .posix_err = 0, .msg = R"(The local account policy modification request was rejected because the policy is controlled by a regional authority.)"},
	{.status = 3221266571, .dos_err = 8654, .posix_err = 0, .msg = R"(The account is controlled by external policy and cannot be modified.)"},
	{.status = 3221266572, .dos_err = 8655, .posix_err = 0, .msg = R"(The Local Administrator Password Solution password update operation failed because the legacy LAPS schema needs to be added to Active Directory.)"},
	{.status = 3221266573, .dos_err = 8656, .posix_err = 0, .msg = R"(The Local Administrator Password Solution password update operation failed because the Windows LAPS schema needs to be added to Active Directory.)"},
	{.status = 3221266574, .dos_err = 8657, .posix_err = 0, .msg = R"(The Local Administrator Password Solution encrypted password update operation failed because Active Directory is not yet running at the minimum required domain functional level (2016).)"},
	{.status = 3221266688, .dos_err = 15300, .posix_err = 0, .msg = R"(Hash generation for the specified version and hash type is not enabled on server.)"},
	{.status = 3221266689, .dos_err = 15301, .posix_err = 0, .msg = R"(The hash requests is not present or not up to date with the current file contents.)"},
	{.status = 3221266721, .dos_err = 15321, .posix_err = 0, .msg = R"(The secondary interrupt controller instance that manages the specified interrupt is not registered.)"},
	{.status = 3221266722, .dos_err = 15322, .posix_err = 0, .msg = R"(The information supplied by the GPIO client driver is invalid.)"},
	{.status = 3221266723, .dos_err = 15323, .posix_err = 0, .msg = R"(The version specified by the GPIO client driver is not supported.)"},
	{.status = 3221266724, .dos_err = 15324, .posix_err = 0, .msg = R"(The registration packet supplied by the GPIO client driver is not valid.)"},
	{.status = 3221266725, .dos_err = 15325, .posix_err = 0, .msg = R"(The requested operation is not supported for the specified handle.)"},
	{.status = 3221266726, .dos_err = 15326, .posix_err = 0, .msg = R"(The requested connect mode conflicts with an existing mode on one or more of the specified pins.)"},
	{.status = 3221266753, .dos_err = 15400, .posix_err = 0, .msg = R"(The requested run level switch cannot be completed successfully since

one or more services refused to stop or restart.)"},
	{.status = 3221266754, .dos_err = 15401, .posix_err = 0, .msg = R"(The service has an invalid run level setting. The run level for a service

must not be higher than the run level of its dependent services.)"},
	{.status = 3221266755, .dos_err = 15402, .posix_err = 0, .msg = R"(The requested run level switch cannot be completed successfully since

one or more services will not stop or restart within the specified timeout.)"},
	{.status = 3221266757, .dos_err = 15403, .posix_err = 0, .msg = R"(A run level switch agent did not respond within the specified timeout.)"},
	{.status = 3221266758, .dos_err = 15404, .posix_err = 0, .msg = R"(A run level switch is currently in progress.)"},
	{.status = 3221266944, .dos_err = 4250, .posix_err = 0, .msg = R"(This operation is only valid in the context of an app container.)"},
	{.status = 3221266945, .dos_err = 4252, .posix_err = 0, .msg = R"(This functionality is not supported in the context of an app container.)"},
	{.status = 3221266946, .dos_err = 4253, .posix_err = 0, .msg = R"(The length of the SID supplied is not a valid length for app container SIDs.)"},
	{.status = 3221266947, .dos_err = 5, .posix_err = 13, .msg = R"(Access to the specified resource has been denied for a less privileged app container.)"},
	{.status = 3221266948, .dos_err = 5, .posix_err = 13, .msg = R"(Access to the specified resource has been denied for an adminless system.)"},
	{.status = 3221267073, .dos_err = 4400, .posix_err = 0, .msg = R"(Fast Cache data not found.)"},
	{.status = 3221267074, .dos_err = 4401, .posix_err = 0, .msg = R"(Fast Cache data expired.)"},
	{.status = 3221267075, .dos_err = 4402, .posix_err = 0, .msg = R"(Fast Cache data corrupt.)"},
	{.status = 3221267076, .dos_err = 4403, .posix_err = 0, .msg = R"(Fast Cache data has exceeded its max size and cannot be updated.)"},
	{.status = 3221267077, .dos_err = 4404, .posix_err = 0, .msg = R"(Fast Cache has been ReArmed and requires a reboot until it can be updated.)"},
	{.status = 3221267105, .dos_err = 4440, .posix_err = 0, .msg = R"(The copy offload read operation is not supported by a filter.)"},
	{.status = 3221267106, .dos_err = 4441, .posix_err = 0, .msg = R"(The copy offload write operation is not supported by a filter.)"},
	{.status = 3221267107, .dos_err = 4442, .posix_err = 0, .msg = R"(The copy offload read operation is not supported for the file.)"},
	{.status = 3221267108, .dos_err = 4443, .posix_err = 0, .msg = R"(The copy offload write operation is not supported for the file.)"},
	{.status = 3221267109, .dos_err = 4446, .posix_err = 0, .msg = R"(The WOF driver encountered a corruption in WIM image's Header.)"},
	{.status = 3221267110, .dos_err = 4447, .posix_err = 0, .msg = R"(The WOF driver encountered a corruption in WIM image's Resource Table.)"},
	{.status = 3221267111, .dos_err = 4448, .posix_err = 0, .msg = R"(The WOF driver encountered a corruption in the compressed file's Resource Table.)"},
	{.status = 3221274625, .dos_err = 470, .posix_err = 0, .msg = R"(The CimFS image is corrupted.)"},
	{.status = 3221278209, .dos_err = 369, .posix_err = 0, .msg = R"(The provider that supports file system virtualization is temporarily unavailable.)"},
	{.status = 3221278210, .dos_err = 370, .posix_err = 0, .msg = R"(The metadata for file system virtualization is corrupt and unreadable.)"},
	{.status = 3221278211, .dos_err = 371, .posix_err = 0, .msg = R"(The provider that supports file system virtualization is too busy to complete this operation.)"},
	{.status = 3221278212, .dos_err = 372, .posix_err = 0, .msg = R"(The provider that supports file system virtualization is unknown.)"},
	{.status = 3221278213, .dos_err = 385, .posix_err = 0, .msg = R"(The virtualization operation is not allowed on the file in its current state.)"},
	{.status = 3221278464, .dos_err = 358, .posix_err = 0, .msg = R"(The cloud sync root metadata is corrupted.)"},
	{.status = 3221278465, .dos_err = 362, .posix_err = 0, .msg = R"(The cloud file provider is not running.)"},
	{.status = 3221278466, .dos_err = 363, .posix_err = 0, .msg = R"(The cloud file metadata is corrupt and unreadable.)"},
	{.status = 3221278467, .dos_err = 364, .posix_err = 0, .msg = R"(The cloud file metadata is too large.)"},
	{.status = 3221278470, .dos_err = 375, .posix_err = 0, .msg = R"(The version of the cloud file property store is not supported.)"},
	{.status = 3221278471, .dos_err = 376, .posix_err = 0, .msg = R"(The file is not a cloud file.)"},
	{.status = 3221278472, .dos_err = 377, .posix_err = 0, .msg = R"(The file is not in sync with the cloud.)"},
	{.status = 3221278473, .dos_err = 378, .posix_err = 0, .msg = R"(The cloud sync root is already connected with another cloud sync provider.)"},
	{.status = 3221278474, .dos_err = 379, .posix_err = 0, .msg = R"(The operation is not supported by the cloud sync provider.)"},
	{.status = 3221278475, .dos_err = 380, .posix_err = 0, .msg = R"(The cloud operation is invalid.)"},
	{.status = 3221278476, .dos_err = 381, .posix_err = 0, .msg = R"(The cloud operation is not supported on a read-only volume.)"},
	{.status = 3221278477, .dos_err = 382, .posix_err = 0, .msg = R"(The operation is reserved for a connected cloud sync provider.)"},
	{.status = 3221278478, .dos_err = 383, .posix_err = 0, .msg = R"(The cloud sync provider failed to validate the downloaded data.)"},
	{.status = 3221278479, .dos_err = 386, .posix_err = 0, .msg = R"(The cloud sync provider failed user authentication.)"},
	{.status = 3221278480, .dos_err = 387, .posix_err = 0, .msg = R"(The cloud sync provider failed to perform the operation due to low system resources.)"},
	{.status = 3221278481, .dos_err = 388, .posix_err = 0, .msg = R"(The cloud sync provider failed to perform the operation due to network being unavailable.)"},
	{.status = 3221278482, .dos_err = 389, .posix_err = 0, .msg = R"(The cloud operation was unsuccessful.)"},
	{.status = 3221278483, .dos_err = 390, .posix_err = 0, .msg = R"(The operation is only supported on files under a cloud sync root.)"},
	{.status = 3221278484, .dos_err = 391, .posix_err = 0, .msg = R"(The operation cannot be performed on cloud files in use.)"},
	{.status = 3221278485, .dos_err = 392, .posix_err = 0, .msg = R"(The operation cannot be performed on pinned cloud files.)"},
	{.status = 3221278486, .dos_err = 393, .posix_err = 0, .msg = R"(The cloud operation was aborted.)"},
	{.status = 3221278487, .dos_err = 394, .posix_err = 0, .msg = R"(The cloud file's property store is corrupt.)"},
	{.status = 3221278488, .dos_err = 395, .posix_err = 0, .msg = R"(Access to the cloud file is denied.)"},
	{.status = 3221278489, .dos_err = 396, .posix_err = 0, .msg = R"(The cloud operation cannot be performed on a file with incompatible hardlinks.)"},
	{.status = 3221278490, .dos_err = 397, .posix_err = 0, .msg = R"(The operation failed due to a conflicting cloud file property lock.)"},
	{.status = 3221278491, .dos_err = 398, .posix_err = 0, .msg = R"(The cloud operation was canceled by user.)"},
	{.status = 3221278493, .dos_err = 404, .posix_err = 0, .msg = R"(The cloud file provider exited unexpectedly.)"},
	{.status = 3221278494, .dos_err = 405, .posix_err = 0, .msg = R"(The file is not a cloud sync root.)"},
	{.status = 3221278495, .dos_err = 426, .posix_err = 0, .msg = R"(The cloud operation was not completed before the time-out period expired.)"},
	{.status = 3221278496, .dos_err = 434, .posix_err = 0, .msg = R"(Dehydration of the cloud file is disallowed by the cloud sync provider.)"},
	{.status = 3221278497, .dos_err = 475, .posix_err = 0, .msg = R"(The cloud provider failed to acknowledge a message before the time-out expired.)"},
	{.status = 3221288192, .dos_err = 435, .posix_err = 0, .msg = R"(A file snapshot operation was attempted when one is already in progress.)"},
	{.status = 3221288193, .dos_err = 436, .posix_err = 0, .msg = R"(A snapshot of the file cannot be taken because a user-mapped section is present.)"},
	{.status = 3221288194, .dos_err = 437, .posix_err = 0, .msg = R"(The file snapshot operation was terminated because one of the files was modified in a way incompatible with a snapshot operation.  Please try again.)"},
	{.status = 3221288195, .dos_err = 438, .posix_err = 0, .msg = R"(An I/O request could not be coordinated with a file snapshot operation.)"},
	{.status = 3221288196, .dos_err = 439, .posix_err = 0, .msg = R"(An unexpected error occurred while processing a file snapshot operation.)"},
	{.status = 3221288197, .dos_err = 440, .posix_err = 0, .msg = R"(A file snapshot operation received an invalid parameter.)"},
};
}
