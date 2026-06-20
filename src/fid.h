#ifndef ENDIAN_FID_H_
#define ENDIAN_FID_H_

// although the namespace and the id have a const qualifier, that is typically
// to improve ease of passing. although it is bad practice to cast between const
// and non-const types, this makes it more convenient for the programmer.
struct fid {
  const char* ns;
  const char* id;
};

// takes a string formatted as namespace:id and replaces the colon with a null
// terminator.
// if the string cannot be properly split, it sets the id to the full string and
// sets the namespace to NULL
struct fid fid_split(char* str);

#endif
