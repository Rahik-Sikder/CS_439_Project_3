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
  void* vaddr;               // Virtual address of the page
  bool loaded;               // Is the page loaded into memory?
  bool writeable;            // Is the page writeable?
  bool swapped;              // Is the page swapped out?
  bool dirty;                // Is the page modified?
  bool reference;            // Is the page modified?
  enum page_loc location;    // page location
  block_sector_t swap_index; // Index in the swap table if
  struct frame* frame;
  struct thread* owning_thread;
  struct hash_elem hash_elem; // Hash table element
  struct file* file;          // File address
  off_t file_offset;          // Offset in file
  off_t file_bytes;           // Bytes to read/write, 1...PGSIZE.
};

hash_hash_func page_hash_func;
hash_less_func page_less_func;

bool handle_load (void* fault_addr, uint8_t* user_esp, bool write);
struct sup_page_table_entry* sup_page_table_insert (void* vaddr,
                                                    bool writeable);

#endif /* vm/page.h */