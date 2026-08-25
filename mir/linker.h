#ifndef ENDIAN_LINKER_H_
#define ENDIAN_LINKER_H_

// makes a hard link
// * old should be an absolute path to the file
// * relative_dir is a path relative to the src dir that must exist already
// * new_dir should be a path to the relative_dir. new_dir does not have to
//   exist yet
// * new_name is what the link will be called
int linker_link(const char* old, const char* relative_dir, const char* new_dir, const char* new_name);

// only proceeds with making hard link if file ends in .h
int linker_link_headers_only(const char* old, const char* relative_dir, const char* new_dir, const char* new_name);

#endif
