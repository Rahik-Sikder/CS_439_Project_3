#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

enum page_loc
{
  LOC_FILE_SYS,
  LOC_SWAP,
  LOC_PHYS,
};

struct sup_page_table_entry
{
  void *vaddr;                // Virtual address of the page
  bool loaded;                // Is the page loaded into memory?
  bool writable;              // Is the page writable?
  bool swapped;               // Is the page swapped out?
  bool dirty;                 // Is the page modified?
  bool reference;             // Is the page modified?
  enum page_loc location;     // page location
  size_t swap_index;          // Index in the swap table if
  struct hash_elem hash_elem; // Hash table element
};

extern struct hash *sup_page_table;

#endif /* vm/page.h */