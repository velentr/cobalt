#include <assert.h>

#include "list.h"

int main()
{
    struct list uut0;
    struct list uut1;
    struct list_elem e0;
    struct list_elem e1;

    list_init(&uut0);
    list_init(&uut1);
    list_pushback(&uut0, &e0);
    list_pushback(&uut1, &e1);
    list_cat(&uut0, &uut1);

    assert(list_size(&uut0) == 2);

    return 0;
}
