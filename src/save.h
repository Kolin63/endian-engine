#ifndef ENDIAN_SAVE_H_
#define ENDIAN_SAVE_H_

// writes to save file. returns 0 if ok.
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
int save_write(const char* dir, const char* file, const char* ext, const char* content);

// reads from save file into out. returns 0 if ok
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
int save_read(const char* dir, const char* file, const char* ext, char** out);

// reads from rom file into out. returns 0 if ok
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
int rom_read(const char* mod, const char* dir, const char* file, const char* ext, char** out);

#endif
