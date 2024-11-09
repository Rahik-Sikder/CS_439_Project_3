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
bool page_less_func (const struct hash_elem *a,
                             const struct hash_elem *b, void *aux)
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

  if (hash_insert (cur->sup_page_table, &new_page->hash_elem) == NULL)
    {
      free (new_page);
      return NULL;
    }

  return new_page;
}

// Jake end driving