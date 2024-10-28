#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

struct frame *all_frames;

static struct lock scan_lock;

void frame_init (void)
{
  void *addr;
  lock_init (&scan_lock);

  all_frames = malloc (sizeof (struct frame) * init_ram_pages);

  if (all_frames == NULL)
    PANIC ("out of memory allocating page frames");

  for (int i = 0; i < init_ram_pages; i++)
    {
      struct frame *f = &all_frames[i];
      lock_init (&f->frame_lock);
      f->page = NULL;
      f->base_addr = palloc_get_page (PAL_USER);
    }
}

void *try_alloc_frame (int page)
{
  int i;

  lock_acquire (&scan_lock);

  for (i = 0; i < init_ram_pages; i++)
    {
      struct frame *f = &all_frames[i];
      if (lock_held_by_current_thread (&f->frame_lock) || !lock_try_acquire(&f->frame_lock))
        continue;
      if (f->page == NULL)
        {
          f->page = page;
          lock_release (&scan_lock);
          return f->base_addr;
        }
      lock_release (&f->frame_lock);
    }
  lock_release (&scan_lock);
  return NULL;
}