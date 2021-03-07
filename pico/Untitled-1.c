#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <lfs.h>
#include <ff.h>

static int lfs_mode_conv(int m)
{
    int flags = 0;
    if ((m & 3) == O_RDONLY)
        flags |= LFS_O_RDONLY;
    if ((m & 3) == O_WRONLY)
        flags |= LFS_O_WRONLY;
    if ((m & 3) == O_RDWR)
        flags |= LFS_O_RDWR;
    if (m & O_CREAT)
        flags |= LFS_O_CREAT;
    if (m & O_EXCL)
        flags |= LFS_O_EXCL;
    if (m & O_TRUNC)
        flags |= LFS_O_TRUNC;
    if (m & O_APPEND)
        flags |= LFS_O_APPEND;
    return flags;
}

static int fatfs_mode_conv(int m)
{
    int res = 0;
    int acc_mode = m & O_ACCMODE;
    if (acc_mode == O_RDONLY)
    {
        res |= FA_READ;
    }
    else if (acc_mode == O_WRONLY)
    {
        res |= FA_WRITE;
    }
    else if (acc_mode == O_RDWR)
    {
        res |= FA_READ | FA_WRITE;
    }
    if ((m & O_CREAT) && (m & O_EXCL))
    {
        res |= FA_CREATE_NEW;
    }
    else if ((m & O_CREAT) && (m & O_TRUNC))
    {
        res |= FA_CREATE_ALWAYS;
    }
    else if (m & O_APPEND)
    {
        res |= FA_OPEN_ALWAYS;
    }
    else
    {
        res |= FA_OPEN_EXISTING;
    }
    return res;
}

static int fatfs_fresult_to_errno(FRESULT fr)
{
    switch (fr)
    {
    case FR_DISK_ERR:
        return EIO;
    case FR_INT_ERR:
        return EIO;
    case FR_NOT_READY:
        return ENODEV;
    case FR_NO_FILE:
        return ENOENT;
    case FR_NO_PATH:
        return ENOENT;
    case FR_INVALID_NAME:
        return EINVAL;
    case FR_DENIED:
        return EACCES;
    case FR_EXIST:
        return EEXIST;
    case FR_INVALID_OBJECT:
        return EBADF;
    case FR_WRITE_PROTECTED:
        return EACCES;
    case FR_INVALID_DRIVE:
        return ENXIO;
    case FR_NOT_ENABLED:
        return ENODEV;
    case FR_NO_FILESYSTEM:
        return ENODEV;
    case FR_MKFS_ABORTED:
        return EINTR;
    case FR_TIMEOUT:
        return ETIMEDOUT;
    case FR_LOCKED:
        return EACCES;
    case FR_NOT_ENOUGH_CORE:
        return ENOMEM;
    case FR_TOO_MANY_OPEN_FILES:
        return ENFILE;
    case FR_INVALID_PARAMETER:
        return EINVAL;
    case FR_OK:
        return 0;
    }
    assert(0 && "unhandled FRESULT");
    return ENOTSUP;
}