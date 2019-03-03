#include <assert.h>
#include <stdlib.h>
#include "list.h"

#include "common.h"

static uint16_t values[256];


static void splice(struct list_head *head,
                   struct list_head *front,
                   struct list_head *back)
{
    while (1) {
        struct listitem *a = list_first_entry(front, struct listitem, list);
        struct listitem *b = list_first_entry(back, struct listitem, list);
        if (list_empty(front)) {
            list_splice_tail(back, head);
            break;
        }
        if (list_empty(back)) {
            list_splice_tail(front, head);
            break;
        }
        if (a->i <= b->i) {
            list_del(&a->list);
            list_add_tail(&a->list, head);
        } else {
            list_del(&b->list);
            list_add_tail(&b->list, head);
        }
    }
}

static void split(struct list_head *src,
                  struct list_head *front,
                  struct list_head *back)
{
    struct list_head *onestep, *twostep;

    onestep = src->next;
    twostep = src->next->next;

    while (twostep != src) {
        twostep = twostep->next;
        if (twostep != src) {
            onestep = onestep->next;
            twostep = twostep->next;
        }
    }

    list_cut_position(front, src, onestep);
    list_cut_position(back, src, src->prev);
}

static void list_mergesort(struct list_head *head)
{
    /*
      struct list_head list_unsorted;
      struct listitem *item = NULL, *is = NULL;

      INIT_LIST_HEAD(&list_unsorted);
      list_splice_init(head, &list_unsorted);

      list_for_each_entry_safe (item, is, &list_unsorted, list) {
          list_del(&item->list);
          list_insert_sorted(item, head);
      }
    */
    struct list_head front, back;

    INIT_LIST_HEAD(&front);
    INIT_LIST_HEAD(&back);

    if (list_empty(head) || list_is_singular(head))
        return;
    split(head, &front, &back);

    list_mergesort(&front);
    list_mergesort(&back);

    splice(head, &front, &back);
}

int main(void)
{
    struct list_head testlist;
    struct listitem *item = NULL, *is = NULL;
    size_t i;

    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));

    INIT_LIST_HEAD(&testlist);

    assert(list_empty(&testlist));

    for (i = 0; i < ARRAY_SIZE(values); i++) {
        item = (struct listitem *) malloc(sizeof(*item));
        assert(item);
        item->i = values[i];
        list_add_tail(&item->list, &testlist);
    }

    assert(!list_empty(&testlist));

    qsort(values, ARRAY_SIZE(values), sizeof(values[0]), cmpint);
    list_mergesort(&testlist);

    i = 0;
    list_for_each_entry_safe (item, is, &testlist, list) {
        assert(item->i == values[i]);
        list_del(&item->list);
        free(item);
        i++;
    }

    assert(i == ARRAY_SIZE(values));
    assert(list_empty(&testlist));

    return 0;
}
