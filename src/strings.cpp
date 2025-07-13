#include <stdarg.h>


#ifdef _WIN32
#define HOST_SYSTEM_SLASH (u8)'\\'
#define SWAP_SLASH (u8)'/'
#else
#define HOST_SYSTEM_SLASH (u8)'/'
#define SWAP_SLASH (u8)'\\'
#endif

#define tformat(cstr, ...) string_format(get_temporary_allocator(), STR(cstr), __VA_ARGS__)
#define tcstr(str) string_to_c_string(str, get_temporary_allocator())
#define tprint(cstr, ...) printf("%s", tcstr(tformat(cstr, __VA_ARGS__)))

struct String {
    u64 size;
    u8 *data;

    u8 &operator [](u64 index) {
        return data[index];
    }
};

#define STR(string) (String) {\
    .size = c_string_length(string),\
    .data = (u8*)const_cast<char*>(string)\
}

#define U8CSTR(string)\
    (u8*)const_cast<char*>(ENSURE_LITERAL_STRING(string))

#define index_of_last_file_slash(input)\
    string_last_index_of(string_swap(input, SWAP_SLASH, HOST_SYSTEM_SLASH, get_temporary_allocator()), HOST_SYSTEM_SLASH)

String string_concat(String a, String b, Allocator alloc);
String string_copy(String a, Allocator alloc);

char *string_to_c_string(String a, Allocator alloc) {
    assert(a.data != NULL);

    u8* data = (u8*)mem_alloc(alloc, a.size + 1);

    if (data == NULL) {
        ERRLOG("string conversion failed, buy more ram, or provide normal allocator...");
        return NULL;
    }

    mem_copy(data, a.data, a.size);
    return (char*)data;
}

u64 c_string_length(const char *c_str) {
    char* s = const_cast<char*>(c_str);

    while (*s != '\0') {
        s++;
    }

    return s - const_cast<char*>(c_str);
}

String string_swap(String input, u8 from, u8 to, Allocator alloc) {
    String output = string_copy(input, alloc);

    for (u64 i = 0; i < input.size; i++) {
        if (output.data[i] != from)
            continue;

        output.data[i] = to;
    }

    return output;
}

s32 string_compare(String a, String b) {
    if (a.size == b.size && a.size == 0) return 0;
    if (a.size == 0) return -1;
    if (b.size == 0) return 1;


    s32 result = mem_compare(a.data, b.data, a.size);

    if (result == 0) {
        if (a.size > b.size) return 1;
        if (a.size < b.size) return -1;
        return 0;
    } else {
        return result;
    }
}

String string_join(List<String> input, String separator, Allocator alloc) {
    String     temp   = {};
    Allocator talloc = get_temporary_allocator();

    if (input.count == 1) {
        return string_copy(*list_get(&input, 0), alloc);
    }

    for (u64 i = 0; i < input.count; i++) {
        String member = *list_get(&input, i);
        if (i == 0) {
            temp = string_concat(temp, member, talloc);
        } else if (i != (input.count - 1)) {
            String separated = string_concat(separator, member, talloc);
            temp = string_concat(temp, separated, talloc);
        } else {
            String separated = string_concat(separator, member, talloc);
            return string_concat(temp, separated, alloc);
        }
    }

    return {};
}

List<String> string_split(String input, String pattern) {
    Allocator talloc = get_temporary_allocator();

    List<String> splits = {};

    if (input.size <= pattern.size) {
        return {};
    }

    if (pattern.size == 0) {
        return {};
    }

    u64 start = 0;

    for (u64 i = 0; i < (input.size - (pattern.size - 1)); i++) {
        if (mem_compare(input.data + i, pattern.data, pattern.size) != 0) {
            continue;
        }

        u64 size = i - start; 

        if (size == 0) {
            start = i + pattern.size;
            continue;
        }

        u8* buffer = (u8*)mem_alloc(talloc, size);
        mem_copy(buffer, input.data + start, size);
        String output = { .size = size, .data = buffer };
        list_add(&splits, output);
        start = i + pattern.size;
    }

    if (start != input.size - pattern.size) {
        if (mem_compare(input.data + start, pattern.data, pattern.size) == 0) {
            return splits;
        }

        u64 size = input.size - start; 
        if (size == 0) {
            return splits;
        }

        u8* buffer = (u8*)mem_alloc(talloc, size);
        mem_copy(buffer, input.data + start, size);
        String output = { .size = size, .data = buffer };
        list_add(&splits, output);
    }

    return splits;
}

String string_copy(String a, Allocator alloc) {
    if (a.data == NULL) {
        return {};
    }

    u8* data = (u8*)mem_alloc(alloc, a.size);

    if (data == NULL) {
        ERRLOG("string copy failed, buy more ram, or provide normal allocator...");
        return {};
    }

    mem_copy(data, a.data, a.size);
    return { .size = a.size, .data = (u8*) data };
}

String string_concat(String a, String b, Allocator alloc) {
    if (a.size == b.size && a.size == 0) {
        return {};
    }

    u8* data = (u8*)mem_alloc(alloc, a.size + b.size);

    if (a.size > 0) {
        assert(a.data != NULL);
        mem_copy(data, a.data, a.size);
    }

    if (b.size > 0) {
        assert(b.data != NULL);
        mem_copy((data + a.size), b.data, b.size);
    }

    return {.size = a.size + b.size, .data = (u8*) data };
}

String string_substring(String input, u64 start, u64 size, Allocator alloc) {
    assert(size > 0);
    if (start > (input.size - size)) {
        ERRLOG("start position with size overlapping the input string.");
        return {};
    }
    u8* data = (u8*)mem_alloc(alloc, size);
    mem_copy(data, input.data + start, size);

    return {.size = size, .data = (u8*) data };
}

s64 string_index_of(String input, u8 value) {
    for (u64 i = 0; i < input.size; i++) {
        if (input.data[i] == value) return i;
    }

    return -1;
}

s64 string_last_index_of(String input, u8 value) {
    s64 index = -1;

    for (u64 i = 0; i < input.size; i++) {
        if (input.data[i] == value) index = i;
    }

    return index;
}

static String format_u64(u64 value) {
    if (value == 0) {
        return STR("0");
    }
 
    Allocator talloc = get_temporary_allocator();
    String     output = {};

    while (value) {
        u8 digit = '0' + (value % 10);
        output = string_concat(output, { 1, &digit }, talloc);
        value /= 10;
    }

    for (u64 l = 0, r = output.size - 1; l < r; l++, r--) {
        u8 t           = output[l];
        output.data[l] = output[r];
        output.data[r] = t;
    }

    return output;
}

static String format_s64(s64 value) {
    if (value == 0) {
        return STR("0");
    }
 
    Allocator talloc = get_temporary_allocator();
    String     output = {};

    b32 negative = false;

    if (value < 0) {
        value    = -value;
        negative = true;
    }

    while (value) {
        u8 digit = '0' + (value % 10);
        output = string_concat(output, { 1, &digit }, talloc);
        value /= 10;
    }

    if (negative) {
        u8 sign = '-';
        output = string_concat(output, { 1, &sign }, talloc);
    }

    for (u64 l = 0, r = output.size - 1; l < r; l++, r--) {
        u8 t           = output[l];
        output.data[l] = output[r];
        output.data[r] = t;
    }

    return output;
}

String string_format(Allocator alloc, String buffer...) {
    va_list args;
    va_start(args, buffer);
    
    Allocator talloc = get_temporary_allocator();

    String o = {};
    
    for (u64 i = 0; i < buffer.size; i++) {
        if (buffer[i] != '%') {
            o = string_concat(o, { 1, buffer.data + i }, talloc);
            continue;
        }

        switch (buffer[++i]) {
            case '%':
            {
                o = string_concat(o, { 1, buffer.data + i }, talloc);
            } break;
            case 'c':
            {
                u32 b = va_arg(args, u32);
                o = string_concat(o, {1, (u8*)&b }, talloc);
            } break;

            case 'u':
            {
                o = string_concat(o, format_u64(va_arg(args, u64)), talloc);
            } break;

            case 'd': 
            {
                o = string_concat(o, format_s64(va_arg(args, s64)), talloc);
            } break;

            case 's':
            {
                o = string_concat(o, va_arg(args, String), talloc);
            } break;
            default: break;
        }
    }
    // va_arg(args, );
    // va_arg(args, long long);

    va_end(args);
    return string_copy(o, alloc);
}

void string_tests(void) {
#ifdef DEBUG
    temp_reset();

    assert(c_string_length("Hello") == 5);
    assert(c_string_length("")      == 0);
    assert(c_string_length("What")  == 4);

    Allocator alloc = get_temporary_allocator();

    String result = {};

    result = string_concat(STR("Hello "), STR("world!"), alloc);
    assert(!string_compare(result, STR("Hello world!")));

    assert(!string_compare(string_copy(result, alloc), result));
    
    List<String> splits = string_split(STR("Eatin burger wit no honey mustard"), STR(" "));

    assert(splits.count == 6);

    assert(!string_compare(splits.data[0], STR("Eatin")));
    assert(!string_compare(splits.data[1], STR("burger")));
    assert(!string_compare(splits.data[2], STR("wit")));
    assert(!string_compare(splits.data[3], STR("no")));
    assert(!string_compare(splits.data[4], STR("honey")));
    assert(!string_compare(splits.data[5], STR("mustard")));

    list_delete(&splits);

    splits = string_split(STR("Eatin||burger||wit||no||honey||mustard"), STR("||"));

    assert(splits.count == 6);

    assert(!string_compare(splits.data[0], STR("Eatin")));
    assert(!string_compare(splits.data[1], STR("burger")));
    assert(!string_compare(splits.data[2], STR("wit")));
    assert(!string_compare(splits.data[3], STR("no")));
    assert(!string_compare(splits.data[4], STR("honey")));
    assert(!string_compare(splits.data[5], STR("mustard")));

    list_delete(&splits);

    splits = string_split(STR("Eatin||burger||wit||no||honey||mustard||a"), STR("||"));

    assert(splits.count == 7);

    assert(!string_compare(splits.data[0], STR("Eatin")));
    assert(!string_compare(splits.data[1], STR("burger")));
    assert(!string_compare(splits.data[2], STR("wit")));
    assert(!string_compare(splits.data[3], STR("no")));
    assert(!string_compare(splits.data[4], STR("honey")));
    assert(!string_compare(splits.data[5], STR("mustard")));
    assert(!string_compare(splits.data[6], STR("a")));

    list_delete(&splits);

    splits = string_split(STR("a||Eatin||burger||wit||no||honey||mustard||a"), STR("||"));

    assert(splits.count == 8);

    assert(!string_compare(splits.data[0], STR("a")));
    assert(!string_compare(splits.data[1], STR("Eatin")));
    assert(!string_compare(splits.data[2], STR("burger")));
    assert(!string_compare(splits.data[3], STR("wit")));
    assert(!string_compare(splits.data[4], STR("no")));
    assert(!string_compare(splits.data[5], STR("honey")));
    assert(!string_compare(splits.data[6], STR("mustard")));
    assert(!string_compare(splits.data[7], STR("a")));

    list_delete(&splits);

    splits = string_split(STR("||Eatin||burger||wit||no||honey||mustard||"), STR("||"));

    assert(splits.count == 6);

    assert(!string_compare(splits.data[0], STR("Eatin")));
    assert(!string_compare(splits.data[1], STR("burger")));
    assert(!string_compare(splits.data[2], STR("wit")));
    assert(!string_compare(splits.data[3], STR("no")));
    assert(!string_compare(splits.data[4], STR("honey")));
    assert(!string_compare(splits.data[5], STR("mustard")));

    assert(!string_compare(string_join(splits, STR(" "), alloc), STR("Eatin burger wit no honey mustard")));

    list_delete(&splits);

    assert(!string_compare(string_substring(STR("HelloWorld!"), 5, 6, alloc), STR("World!")));
    assert(!string_compare(string_substring(STR("HelloWorld!"), 0, 11, alloc), STR("HelloWorld!")));
    assert(!string_compare(string_substring(STR("HelloWorld!"), 10, 1, alloc), STR("!")));

    assert(string_index_of(STR("CP/M"), (u8)'/') == 2);
    assert(string_last_index_of(STR("https://github.com/bonmas14"), (u8)'/') == 18);

    assert(!string_compare(string_swap(STR("/path/from/unix/systems/"), (u8)'/', (u8) '\\', alloc), STR("\\path\\from\\unix\\systems\\"))); 

    assert(!string_compare(string_format(alloc, STR("/path/%s/unix/a %d %u %%"), STR("test"), (s64)-100, (u64)404), STR("/path/test/unix/a -100 404 %"))); 

    temp_reset();
#endif
}

