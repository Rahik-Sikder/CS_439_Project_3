#include "threads/synch.h"
#include "page.h"

struct frame 
  {
    struct lock frame_lock;       // only one thread can access at a time
    struct sup_page_table_entry *page;  	// page associated with frame
    void *base_addr;                   //base address of the frame
  };

static struct frame *all_frames;

void frame_init (void);
