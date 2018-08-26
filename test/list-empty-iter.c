#include <assert.h>

#include "list.h"

int main()
{
    struct list uut;
    struct list_elem *e;

    list_init(&uut);

    for (e = list_begin(&uut); e != list_end(&uut); e = list_next(e))
    {
        assert(0);
    }

    return 0;
}
