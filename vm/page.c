#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

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
unsigned int page_less_func (const struct hash_elem *a,
                             const struct hash_elem *b, void *aux)
{
  struct sup_page_table_entry *entry_a =
      hash_entry (a, struct sup_page_table_entry, hash_elem);
  struct sup_page_table_entry *entry_b =
      hash_entry (b, struct sup_page_table_entry, hash_elem);

  return entry_a->vaddr < entry_b->vaddr;
}