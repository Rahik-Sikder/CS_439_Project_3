#include <debug.h>
#include <bitmap.h>
#include "vm/swap.h"
#include "vm/frame.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

static struct block *swap_devices;
static struct bitmap *swap_bitmap;
static struct lock swap_lock;

void swap_init (void)
{
  swap_devices = block_get_role (BLOCK_SWAP);
  if (swap_devices == NULL)
    {
      swap_bitmap = bitmap_create (0);
    }
  else
    {
      swap_bitmap = bitmap_create (block_size (swap_devices) /
                                   (PGSIZE / BLOCK_SECTOR_SIZE));
    }

  lock_init (&swap_lock);
}

bool swap_page_in (struct sup_page_table_entry *page)
{
  ASSERT (page->location == LOC_SWAP);

  for (int i = 0; i < (PGSIZE / BLOCK_SECTOR_SIZE); i++)
    {
      block_read (swap_devices,
                  page->swap_index * (PGSIZE / BLOCK_SECTOR_SIZE) + i,
                  (char *) page->frame->base_addr + i * BLOCK_SECTOR_SIZE);
    }
  lock_acquire (&swap_lock);
  bitmap_reset (swap_bitmap, page->swap_index);
  lock_release (&swap_lock);
  page->swap_index = -1;
  page->location = LOC_MEMORY;

  return true;
}

bool swap_page_out (struct sup_page_table_entry *page)
{
  ASSERT (page->location == LOC_MEMORY);

  lock_acquire (&swap_lock);
  size_t swap_index = bitmap_scan_and_flip (swap_bitmap, 0, 1, false);
  lock_release (&swap_lock);
  // Return false on failed bitmap scan
  if (swap_index == BITMAP_ERROR)
    {
      return false;
    }

  page->swap_index = swap_index;
  for (int i = 0; i < (PGSIZE / BLOCK_SECTOR_SIZE); i++)
    {
      block_write (swap_devices,
                   page->swap_index * (PGSIZE / BLOCK_SECTOR_SIZE) + i,
                   (char *) page->frame->base_addr + i * BLOCK_SECTOR_SIZE);
    }
  page->location = LOC_SWAP;
  page->file_bytes = 0;
  page->file = NULL;
  page->file_offset = 0;

  return true;
}