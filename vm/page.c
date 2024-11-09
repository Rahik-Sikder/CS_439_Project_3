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
#include "userprog/process.h"
#include "vm/page.h"
#include <string.h>
#include "userprog/pagedir.h"
// Milan start driving

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
// Milan end driving
// Jake start driving

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

  // Rahik start driving
  new_page->file = NULL;
  new_page->file_offset = 0;
  new_page->file_bytes = 0;
  // Rahik end driving

  // Jake start driving
  if (hash_insert (&cur->page_table, &new_page->hash_elem))
    {
      free (new_page);
      return NULL;
    }
  // Jake end driving

  return new_page;
}

// Jake end driving
// Milan start driving
static struct sup_page_table_entry *get_entry_addr (void *vaddr,
                                                    uint8_t *user_esp)
{

  // Check if page fault is in user space
  if (vaddr >= PHYS_BASE)
    return NULL;

  struct sup_page_table_entry page;
  struct hash_elem *elem;

  page.vaddr = (void *) pg_round_down (vaddr);

  // Page exists
  // Rahik start driving
  if (elem = hash_find (&thread_current ()->page_table, &page.hash_elem))
    return hash_entry (elem, struct sup_page_table_entry, hash_elem);
  // Rahik end driving
  // Rahik start driving  
  // Milan start driving

  // If vaddr is within max stack growth and a 32 bytes from esp, allocate
  if ((uint8_t *) vaddr > (user_esp - 32))
    {
      struct sup_page_table_entry *new_page =
          sup_page_table_insert (page.vaddr, true);
      struct frame *found_frame = try_alloc_frame (new_page);
      if (found_frame == NULL)
        return NULL;

      return new_page;
    }
  // Milan end driving
  // Rahik end driving

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
      // Jake start driving
      memset ((char *) page->frame->base_addr + read_bytes, 0, zero_bytes);
      // Jake end driving
    }
  else
    {
      memset (page->frame->base_addr, 0, PGSIZE);
    }

  return true;
}

bool handle_load (void *fault_addr, uint8_t *user_esp, bool write)
{
  struct sup_page_table_entry *page;
  bool status;
  // Jake start driving
  // Rahik start driving
  if (fault_addr == NULL)
    return false;

  // Rahik end driving
  // Milan start driving
  page = get_entry_addr (fault_addr, user_esp);
  if (page == NULL)
    return false;

  if (write && !page->writeable)
    return false;
  // Milan end driving
  
  // Swap in or load from File
  if (page->frame == NULL && !populate_frame (page))
    return false;
// Jake end driving
  status = pagedir_set_page (thread_current ()->pagedir, page->vaddr,
                             page->frame->base_addr, page->writeable);
  return status;
}

// Milan end driving