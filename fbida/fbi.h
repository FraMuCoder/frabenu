
#include "readers.h"
#include "gfx.h"

void free_image(struct ida_image *img);

struct ida_image* read_image(char *filename);

void shadow_draw_image(gfxstate *gfx, struct ida_image *img, int xoff, int yoff,
          unsigned int first, unsigned int last, int weight);

