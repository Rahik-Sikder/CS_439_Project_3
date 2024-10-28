#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

void
frame_init (void) 
{
    all_frames = malloc (sizeof(struct frame) * init_ram_pages);

    if (all_frames == NULL)
        PANIC ("out of memory allocating page frames");

    for(int i = 0; i< init_ram_pages; i++){
      struct frame *f = &all_frames[i];
      lock_init (&f->frame_lock);
      f->page = NULL;
    }
}

static struct frame *try_alloc_frame(struct sup_page_table_entry *page){
    int i;

    for (i = 0; i < init_ram_pages; i++){

        struct frame *f = &all_frames[i];
        if (lock_try_acquire (&f->frame_lock) && f->page == NULL){
            f->page = page;
            break;
        }
    }
}