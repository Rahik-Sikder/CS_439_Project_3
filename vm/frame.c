#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"

struct frame *all_frames;

static struct lock scan_lock;

static size_t hand;

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

struct frame *try_alloc_frame (struct sup_page_table_entry *page)
{
  int i;

  lock_acquire (&scan_lock);

  for (i = 0; i < init_ram_pages; i++)
    {
      struct frame *f = &all_frames[i];
      if (lock_held_by_current_thread (&f->frame_lock) ||
          !lock_try_acquire (&f->frame_lock))
        continue;
      if (f->page == NULL)
        {
          f->page = page;
          lock_release (&scan_lock);
          return f;
        }
      lock_release (&f->frame_lock);
    }

  // No frames avaliable, need to evict
  for (int i; i < init_ram_pages * 2; i++)
    {
      struct frame *cur_frame = &all_frames[hand];
      hand++;
      hand %= init_ram_pages;

      // Check if frame is in use
      if (!lock_try_acquire (&cur_frame->frame_lock))
        continue;

      // Check reference bit
      if (pagedir_is_accessed (cur_frame->page->owning_thread->pagedir,
                               cur_frame->page->vaddr))
        {
          pagedir_set_accessed (cur_frame->page->owning_thread->pagedir,
                                cur_frame->page->vaddr, false);
          lock_release (&cur_frame->frame_lock);
          continue;
        }

      lock_release (&scan_lock);

      // Start eviction
      if (!handle_out (cur_frame->page))
        {
          lock_release (&cur_frame->frame_lock);
          return NULL;
        }

      cur_frame->page = page;
      return cur_frame;
    }

  lock_release (&scan_lock);
  return NULL;
}