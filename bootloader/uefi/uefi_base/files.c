#include "uefi_base.h"

EFI_FILE *open_root(EFI_HANDLE image, EFI_SYSTEM_TABLE *st) {
    EFI_LOADED_IMAGE *loaded;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;

    st->BootServices->HandleProtocol(image, &gEfiLoadedImageProtocolGuid, (void **)&loaded);
    st->BootServices->HandleProtocol(loaded->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&fs);

    EFI_FILE *root;
    fs->OpenVolume(fs, &root);
    return root;
}

EFI_FILE *open_file(EFI_FILE *dir, CHAR16 *path) {
    EFI_FILE *file;
    EFI_STATUS status = dir->Open(dir, &file, path, EFI_FILE_MODE_READ, 0);

    if (status != EFI_SUCCESS)
        return NULL;

    return file;
}

EFI_STATUS read_file(EFI_FILE* file, void** buffer, UINTN* size) {
    EFI_FILE_INFO* info;
    UINTN info_size = SIZE_OF_EFI_FILE_INFO + 200;

    info = AllocatePool(info_size);
    if (!info) return EFI_OUT_OF_RESOURCES;

    if (file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info) != EFI_SUCCESS) return EFI_LOAD_ERROR;

    *size = info->FileSize;

    *buffer = AllocatePool(*size);
    if (!*buffer) {
        FreePool(info);
        return EFI_OUT_OF_RESOURCES;
    }

    if (file->Read(file, size, *buffer) != EFI_SUCCESS) return EFI_LOAD_ERROR;
    
    FreePool(info);
    return EFI_SUCCESS;
}