#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include <string.h>
#include "userprog/pagedir.h"

/* Shifts out PGBITS offset abd returns the address*/
unsigned int page_hash_func (const struct hash_elem *e, void *aux)
{

  struct sup_page_table_entry *page_entry =
      hash_entry (e, struct sup_page_table_entry, hash_elem);
  return ((uint16_t) page_entry->vaddr) >> PGBITS;
}

/* Compares the value of two hash elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
bool page_less_func (const struct hash_elem *a, const struct hash_elem *b,
                     void *aux)
{
  struct sup_page_table_entry *entry_a =
      hash_entry (a, struct sup_page_table_entry, hash_elem);
  struct sup_page_table_entry *entry_b =
      hash_entry (b, struct sup_page_table_entry, hash_elem);

  return entry_a->vaddr < entry_b->vaddr;
}

struct sup_page_table_entry *sup_page_table_insert (void *vaddr, bool writeable)
{
  struct thread *cur = thread_current ();
  struct sup_page_table_entry *new_page =
      malloc (sizeof (struct sup_page_table_entry));

  new_page->vaddr = pg_round_down (vaddr);

  new_page->frame = NULL;

  new_page->writeable = writeable;

  new_page->swap_index = (block_sector_t) -1;

  new_page->owning_thread = cur;

  new_page->file = NULL;
  new_page->file_offset = 0;
  new_page->file_bytes = 0;

  if (hash_insert (&cur->page_table, &new_page->hash_elem))
    {
      free (new_page);
      return NULL;
    }

  return new_page;
}

static struct sup_page_table_entry *get_entry_addr (void *vaddr)
{

  // Check if page fault is in user space
  if (vaddr >= PHYS_BASE)
    return NULL;

  struct sup_page_table_entry page;
  struct hash_elem *elem;

  page.vaddr = (void *) pg_round_down (vaddr);

  // Page exists
  if (elem = hash_find (&thread_current ()->page_table, &page.hash_elem))
    return hash_entry (elem, struct sup_page_table_entry, hash_elem);

  //...Stack Growth

  // Seg Fault
  return NULL;
}

static bool populate_frame (struct sup_page_table_entry *page)
{
  page->frame = try_alloc_frame (page);
  if (page->frame == NULL)
    return false;

  // Swapping...

  if (page->file != NULL)
    {
      off_t read_bytes = file_read_at (page->file, page->frame->base_addr,
                                       page->file_bytes, page->file_offset);
      off_t zero_bytes = PGSIZE - read_bytes;
      memset ((char*)page->frame->base_addr + read_bytes, 0, zero_bytes);
    }
  else
    {
      memset (page->frame->base_addr, 0, PGSIZE);
    }

  return true;
}

bool handle_load (void *fault_addr)
{

  struct sup_page_table_entry *page;
  bool status;

  page = get_entry_addr (fault_addr);
  if (page == NULL)
    {
      return false;
    }

  if (page->frame == NULL)
    {
      // Swap in or load from File
      if (!populate_frame (page))
        return false;
    }

  status = pagedir_set_page (thread_current ()->pagedir, page->vaddr,
                             page->frame->base_addr, page->writeable);
  return status;
}
