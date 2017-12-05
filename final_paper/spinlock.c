NTSTATUS AllocateDictionaryEntry(
    IN PDICTIONARY Dictionary,
    IN ULONGLONG Key,
    _In_range_(0, sizeof(FILE_OBJECT_EXTENSION)) IN ULONG Size,
    IN ULONG Tag,
    OUT PVOID *Entry) 
{
    PDICTIONARY_HEADER header;
    KIRQL oldIrql;
    PDICTIONARY_HEADER *entry;
    NTSTATUS status = STATUS_SUCCESS;

    *Entry = NULL;
    header = ExAllocatePoolWithTag(NonPagedPoolNx,
                                   Size + sizeof(DICTIONARY_HEADER),
                                   Tag);

    if(header == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(header, sizeof(DICTIONARY_HEADER) + Size);
    header->Key = Key;
    KeAcquireSpinLock(&(Dictionary->SpinLock), &oldIrql);
    TRY {
        entry = &(Dictionary->List);

        while(*entry != NULL) {
            if((*entry)->Key == Key) {
                status = STATUS_OBJECT_NAME_COLLISION;
                LEAVE;
            } else if ((*entry)->Key < Key) {
                break;
            } else {
                entry = &((*entry)->Next);
            }
        }

        header->Next = *entry;
        *entry = header;
    } FINALLY {
        KeReleaseSpinLock(&(Dictionary->SpinLock), oldIrql);

        if(!NT_SUCCESS(status)) {
            FREE_POOL(header);
        } else {
            *Entry = (PVOID) header->Data;
        }
    }
    
    return status;
}
