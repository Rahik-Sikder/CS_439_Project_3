#include "devices/block.h"
#include "vm/page.h"

void swap_init (void);
void free_swap_page (size_t swap_index);
bool swap_page_in (struct sup_page_table_entry *page);
bool swap_page_out (struct sup_page_table_entry *page);