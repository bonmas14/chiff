#pragma once

#define ALLOC(x)      calloc(1, x) 
#define FREE(x)       free(x) 
#define STANDART_LIST_SIZE 64

template<typename DataType>
struct List {
    u64 count;
    DataType *data;

    u64 capacity;

    DataType& operator[](u64 index) {
        return data[index];
    }
};

// ----------- Initialization 

template<typename DataType>
b32 list_create(List<DataType> *list, u64 init_size);
template<typename DataType>
List<DataType> list_clone(List<DataType> *list);
template<typename DataType>
b32 list_delete(List<DataType> *list);

// --------- Control

// reserve some amount of memory in list
// return index of a first element

template<typename DataType> 
void list_fill(List<DataType> *list, DataType *data, u64 elements_amount, u64 start_index);

template<typename DataType> 
u64 list_write(List<DataType> *list, DataType *data, u64 elements_amount);

// add element to an list, and advance
template<typename DataType>
void list_add(List<DataType> *list, DataType &data);

// get element by index
template<typename DataType>
DataType *list_get(List<DataType> *list, u64 index);


// ----------- Helpers

template<typename DataType>
b32 list_grow(List<DataType> *list);

// ----------- Implementation

template<typename DataType>
b32 list_create(List<DataType> *list, u64 init_size) {
    list->count        = 0;
    list->capacity = init_size;

    list->data      = (DataType*)ALLOC(init_size * sizeof(DataType));

    if (list->data == NULL) {
        ERRLOG("List: Couldn't create list.");
        return false; 
    }

    return true;
}

template<typename DataType>
List<DataType> list_clone(List<DataType> *list) {
    List<DataType> clone = {};

    if (list->capacity == 0)
        return {};

    if (!list_create(&clone, list->capacity))
        return {};

    mem_copy((u8*)clone.data, (u8*)list->data, sizeof(List<DataType>) * list->count);
    clone.count = list->count;

    return clone;
}

template<typename DataType>
b32 list_delete(List<DataType> *list) {
    if (list == NULL) {
        ERRLOG("List: Reference to list wasn't valid.");
        return false;
    }

    if (list->data == NULL) {
        ERRLOG("List: List was already deleted.");
        return false;
    }

    FREE(list->data); 
    list->data = NULL;

    return true;
}

template<typename DataType> 
void list_fill(List<DataType> *list, DataType *data, u64 elements_amount, u64 start_index) {
    assert(elements_amount > 0);
    list_create_if_needed(list);

    DataType *begin = list->data + start_index;

    mem_copy((u8*)begin, (u8*)data, elements_amount * sizeof(DataType));
}

template<typename DataType> 
u64 list_write(List<DataType> *list, DataType *data, u64 elements_amount) {
    assert(elements_amount > 0);

    list_create_if_needed(list);

    if (!list_grow(list, elements_amount)) {
        assert(false);
    }

    DataType *begin = list->data + list->count;

    if (data) {
        mem_copy((u8*)begin, (u8*)data, elements_amount * sizeof(DataType));
    } else {
        mem_set((u8*)begin, 0, elements_amount * sizeof(DataType));
    }

    u64 begin_index = list->count;
    list->count = list->count + elements_amount;
    return begin_index;
}

// add element to an list, and advance
template<typename DataType>
void list_add(List<DataType> *list, DataType &data) {
    list_create_if_needed(list);

    if (!list_grow(list, 1)) {
        assert(false);
    }

    mem_copy((u8*)(list->data + list->count), (u8*)&data, sizeof(DataType));
    list->count = list->count + 1;
}

// get element by index
template<typename DataType>
DataType *list_get(List<DataType> *list, u64 index) {
    if (!list->data) return NULL;

    if (index >= list->count) {
        ERRLOG("List: Bounce check failed. accessed: %llu, size: %llu", index, list->count);
        return NULL;
    }

    return list->data + index; 
}

// -------------- local functions

template<typename DataType>
void list_create_if_needed(List<DataType> *list) {
    if (list->data) return;
    if (list_create(list, STANDART_LIST_SIZE)) return;

    ERRLOG("tried to create list but failed.");
}

template<typename DataType>
b32 list_grow(List<DataType> *list, u64 fit_elements) {
    assert(list->capacity != 0);
    assert(fit_elements > 0);

    if ((list->count + fit_elements - 1) < list->capacity) {
        return true;
    } else while ((list->count + fit_elements) >= list->capacity) {
        DataType *data = (DataType*)ALLOC(list->capacity * 2 * sizeof(DataType));

        if (!data) {
            ERRLOG("List: Couldn't grow list.");
            return false;
        }

        u64 new_capacity = list->capacity * 2;

        mem_set((u8*)data, 0, new_capacity * sizeof(DataType));
        mem_copy((u8*)data, (u8*)list->data, list->capacity * sizeof(DataType));

        FREE(list->data);
        list->data = data;
        list->capacity = new_capacity;
    }

    return true;
}
