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
static size_t frame_cnt;
static struct lock scan_lock;

static size_t hand;

void frame_init (void)
{
  void *addr;
  void *base;
  lock_init (&scan_lock);

  all_frames = malloc (sizeof (struct frame) * init_ram_pages);

  if (all_frames == NULL)
    PANIC ("out of memory allocating page frames");
  while ((base = palloc_get_page (PAL_USER)) != NULL)
    {
      struct frame *f = &all_frames[frame_cnt++];
      lock_init (&f->frame_lock);
      f->base_addr = base;
      f->page = NULL;
    }
}

struct frame *try_alloc_frame (struct sup_page_table_entry *page)
{
  uint32_t i;

  lock_acquire (&scan_lock);

  for (i = 0; i < frame_cnt; i++)
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
  // printf ("Not enough frames, starting eviction\n");
  // No frames avaliable, need to evict
  for (i = 0; i < frame_cnt * 2; i++)
    {
      struct frame *cur_frame = &all_frames[hand];
      hand++;
      hand %= frame_cnt;

      // Check if frame is in use
      if (lock_held_by_current_thread (&cur_frame->frame_lock) ||
          !lock_try_acquire (&cur_frame->frame_lock))
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
      // printf ("evicting frame %p\n", cur_frame->base_addr);
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
