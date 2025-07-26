#include <stdio.h>

#include "core.cpp"

struct Subseq {
    u64 origin_index;
    u64 compare_index;
    Subseq *next;
};

struct Line {
    u64 start;
    u64 stop;
};

enum Compare_State {
    COMPARE_END,
    COMPARE_NOT_FOUND,
    COMPARE_FOUND,
};

// find longest distance in string
Subseq *get_subsequence(List<meow_u128> origin, List<meow_u128> compare, Allocator alloc) {
    Subseq *seq = NULL;
    Subseq *curr;
    u64 last_index = 0;

    b32 started = false;

    for (u64 origin_index = 0; origin_index < origin.count; origin_index++) {
        Subseq temp = {};
        temp.origin_index = origin_index;

        Compare_State found = COMPARE_END;
        for (u64 compare_index = last_index; compare_index < compare.count; compare_index++) {
            // @todo also check lines for hash failure
            if (!MeowHashesAreEqual(compare[compare_index], origin[origin_index])) {
                found = COMPARE_NOT_FOUND; 
                continue;
            }

            found = COMPARE_FOUND;
            temp.compare_index = compare_index;
            last_index = compare_index + 1;
            break;
        }
        
        switch (found) {
            case COMPARE_END:
            case COMPARE_NOT_FOUND:
                break;

            case COMPARE_FOUND:
                if (!started) {
                    started = true;
                    seq = (Subseq *)mem_alloc(alloc, sizeof(Subseq));
                    assert(seq);
                    curr = seq;
                } else {
                    curr->next = (Subseq *)mem_alloc(alloc, sizeof(Subseq));
                    curr = curr->next;
                }

                *curr = temp;
                break;
        }
    }

    return seq;
}

meow_u128 get_hash(u64 size, void *data) {
    // assert(size > 0);
    assert(data != 0);
    return MeowHash(MeowDefaultSeed, size, data);
    //
    // u32 hash = 216613261u;
    //
    // u8* arr = (u8*)data;
    // for (u64 i = 0; i < size; i++) {
    //     hash ^= arr[i];
    //     hash *= 0x1000193;
    // }
    //
    // return hash;
}

b32 platform_read_file_into_string(String filename, Allocator alloc, String *output) {
    assert(output != NULL);
    assert(filename.data != NULL);
    assert(filename.size > 0);

    FILE *file = fopen(string_to_c_string(filename, get_temporary_allocator()), "rb");

    if (file == NULL) {
        ERRLOG("Could not open file. %.*s", (int)filename.size, filename.data);
        return false;
    }

    fseek(file, 0L, SEEK_END);
    u64 file_size = ftell(file);
    rewind(file);

    if (file_size == 0) {
        fclose(file);
        return false;
    }

    output->data = (u8*)mem_alloc(alloc, file_size);

    u64 bytes_read = fread(output->data, sizeof(u8), file_size, file);

    if (bytes_read < file_size) {
        ERRLOG("Could not read file. %.*s", (int)filename.size, filename.data);
        fclose(file);
        return false;
    }

    output->size = file_size;
    fclose(file);
    return true;
}

List<Line> scan_lines(String file) {
    List<Line> lines = {};
    Line line = {};

    for (u64 i = 0; i < file.size; i++) {
        if (!(file.data[i] == '\n' || file.data[i] == 0)) {
            continue;
        }

        line.stop  = i;
        list_add(&lines, line);
        line.start = i + 1;
    }

    return lines;
}

List<meow_u128> get_hashed_lines(String file, List<Line> lines) {
    List<meow_u128> hashes;

    list_create(&hashes, lines.count);

    for (u64 line_index = 0; line_index < lines.count; line_index++) {
        Line line = lines[line_index];
        meow_u128 hash = get_hash(line.stop - line.start, file.data + line.start);
        list_add(&hashes, hash);
    }

    return hashes;
}

List<meow_u128> get_hashed_lines(String file) {
    List<Line> lines = scan_lines(file);
    List<meow_u128> hashes;

    list_create(&hashes, lines.count);

    for (u64 line_index = 0; line_index < lines.count; line_index++) {
        Line line = lines[line_index];
        meow_u128 hash = get_hash(line.stop - line.start, file.data + line.start);
        list_add(&hashes, hash);
    }

    return hashes;
}

void print_line(String file, Line line) {
    if (line.start == line.stop) return;

    String l = {
        .size = line.stop - line.start, // @todo, as we on windows, we have \r also, so we need to delete it too
        .data = file.data + line.start
    };

    tprint("%s\n", l);
}

int main(int argc, char **argv) {
    String    origin_file = {};
    String    compare_file = {};

    List<Line> origin_lines;
    List<Line> compare_lines;
    List<meow_u128> origin;
    List<meow_u128> compare;

    if (argc == 3) {
        if (!platform_read_file_into_string(STR(argv[1]), get_stdlib_allocator(), &origin_file)) {
            return 2;
        }
        if (!platform_read_file_into_string(STR(argv[2]), get_stdlib_allocator(), &compare_file)) {
            return 2;
        }

        origin_lines  = scan_lines(origin_file);
        compare_lines = scan_lines(compare_file);
        origin  = get_hashed_lines(origin_file, origin_lines);
        compare = get_hashed_lines(compare_file, compare_lines);

    } else {
        ERRLOG("please call with 2 args.\n    %s [old] [new]", argv[0]);
        return 1;
    }

    Subseq *begin = get_subsequence(origin, compare, get_temporary_allocator());

    { // print origin file
        Subseq *temp = begin;
        tprint("> %s\n", STR(argv[1]));
        for (u64 i = 0; i < origin_lines.count; i++) {
            if (temp && temp->origin_index != i) {
                tprint("- ");
                print_line(origin_file, origin_lines[i]);
            } else if (temp) {
                tprint("  ");
                print_line(origin_file, origin_lines[i]);
                temp = temp->next;
            } else {
                tprint("- ");
                print_line(origin_file, origin_lines[i]);
            }
        }
    }

    tprint("\n");

    { // print compared file
        Subseq *temp = begin;
        tprint("> %s\n", STR(argv[2]));
        for (u64 i = 0; i < compare_lines.count; i++) {
            if (temp && temp->compare_index != i) {
                tprint("+ ");
                print_line(compare_file, compare_lines[i]);
            } else if (temp) {
                tprint("  ");
                print_line(compare_file, compare_lines[i]);
                temp = temp->next;
            } else {
                tprint("+ ");
                print_line(compare_file, compare_lines[i]);
            }
        }
    }

    return 0;
}
