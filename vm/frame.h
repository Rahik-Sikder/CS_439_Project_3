#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/synch.h"
#include "page.h"
extern struct frame *all_frames;

void frame_init (void);
void *try_alloc_frame (int page);

struct frame
{
  struct lock frame_lock;            // only one thread can access at a time
  struct sup_page_table_entry *page; // page associated with frame
  void *base_addr;                   // base address of the frame
};

#endif /* vm/frame.h */