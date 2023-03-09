// This is really annoying
// but I do not know of another way
// to do this while using side modules

#ifdef __EMSCRIPTEN__

#include "base/base.h"
#include "os/os.h"

#include <emscripten/emscripten.h>

EM_ASYNC_JS(u8*, os_file_read_impl, (char* file_name), {
    const response = await fetch(UTF8ToString(file_name));
    if (!response.ok) {
        return 0;
    }

    const blob = await response.blob();
    const data = new Uint8Array(await blob.arrayBuffer());

    const len = data.length;
    const lengthArr = new Uint8Array(8);
    for (let i = 0; i < 8; i++) {
        lengthArr[i] = (len >>> (i * 8)) & 0xff;
    }
    
    const ptr = _malloc(data.length + 8);

    HEAP8.set(lengthArr, ptr);
    HEAP8.set(data, ptr + 8);

    return ptr;
})

string8 os_file_read(marena* arena, string8 path) {
    marena_temp temp = marena_temp_begin(arena);

    u8* path_cstr = str8_to_cstr(temp.arena, path);
    u8* data = (u8*)os_file_read_impl((char*)path_cstr);

    marena_temp_end(temp);

    if (data == NULL) {
        log_errorf("Failed to read file from %.*s", (int)path.size, path.str);
        return (string8){ .size = 0 };
    }

    u64 size = 0;
    for (u32 i = 0; i < 8; i++) {
        size |= data[i] << (i * 8);
    }

    string8 file = {
        .str = data + 8,
        .size = size
    };

    string8 out = str8_copy(arena, file);

    free(data);

    return out;
}

EM_ASYNC_JS(int, js_load_lib, (char* file_name), {
    const fileName = UTF8ToString(file_name);

    const response = await fetch(fileName);
    if (!response.ok) {
        return 0;
    }

    const blob = await response.blob();
    const data = new Uint8Array(await blob.arrayBuffer());

    const stream = FS.open(fileName, "w+");
    FS.write(stream, data, 0, data.length, 0);
    FS.close(stream);

    return 1;
})

// I know that this is a bit stupid and convoluted,
// but I do not want to think of a better solution right not
os_library os_lib_load(string8 path) {
    marena_temp temp = marena_scratch_get(NULL, 0);
    
    u8* path_cstr = str8_to_cstr(temp.arena, path);
    
    js_load_lib((char*)path_cstr);
    void* handle = dlopen((char*)path_cstr, RTLD_LAZY);

    marena_scratch_release(temp);

    if (handle == NULL) {
        log_errorf("Failed to dynamic library \"%.*s\"", (int)path.size, path.str);
    }

    return (os_library){
        .handle = handle
    };
}

#endif //__EMSCRIPTEN__