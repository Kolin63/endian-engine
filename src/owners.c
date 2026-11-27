#include "owners.h"

#include "ref/owners.ref.h"

bool
is_owner(unsigned long uuid) {
  for (const unsigned long* i = server_owners; *i != 0; i++) {
    if (*i == uuid) return true;
  }
  return false;
}
