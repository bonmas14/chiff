void mem_set(u8 *buffer, u8 value, u64 size) {
    if (size == 0) return;
    assert(buffer != NULL);

    while (size-- > 0) {
        *buffer++ = value;
    }
}

void mem_copy(u8 *dest, u8 *source, u64 size) {
    if (size == 0) return;

    assert(dest   != NULL);
    assert(source != NULL);

#ifdef DEBUG
    if (dest < source) {
        assert((uintptr_t)(source - dest) >= (uintptr_t)size);
    } else {
        assert((uintptr_t)(dest - source) >= (uintptr_t)size);
    }
#endif // DEBUG

    while (size-- > 0) {
        *dest++ = *source++;
    }
}

s32 mem_compare(u8 *left, u8 *right, u64 size) {
    while (size-- > 0) {
        if (*left++ == *right++)
            continue;

        return left[-1] > right[-1] ? 1 : -1;
    }

    return 0;
}

