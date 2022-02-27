#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (q == NULL)
        return NULL;

    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *pos, *next_pos;
    list_for_each_entry_safe (pos, next_pos, l, list)
        q_release_element(pos);

    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_char = malloc(sizeof(element_t));
    if (new_char == NULL)
        return false;
    // Ref: https://stackoverflow.com/questions/481673/make-a-copy-of-a-char
    new_char->value = (char *) malloc(strlen(s) + 1);
    if (!new_char->value) {
        free(new_char);
        return false;
    }
    strncpy(new_char->value, s, strlen(s) + 1);
    list_add(&new_char->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_char = malloc(sizeof(element_t));
    if (new_char == NULL)
        return false;
    // Ref: https://stackoverflow.com/questions/481673/make-a-copy-of-a-char
    new_char->value = (char *) malloc(strlen(s) + 1);
    if (!new_char->value) {
        free(new_char);
        return false;
    }
    strncpy(new_char->value, s, strlen(s) + 1);
    list_add_tail(&new_char->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_ele = list_first_entry(head, element_t, list);
    list_del(head->next);
    if (sp) {
        // copy to *sp from remove_ele->value.
        strncpy(sp, remove_ele->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_ele = list_last_entry(head, element_t, list);
    list_del(head->prev);
    if (sp) {
        // copy to *sp from remove_ele->value.
        strncpy(sp, remove_ele->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */

bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    // use Tortoise and Hare Algorithm to find the middle node.
    struct list_head *slow = head, *fast = head;
    while (fast->next != head && fast->next->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    slow = slow->next;
    element_t *mid_node = list_entry(slow, element_t, list);
    list_del_init(slow);
    q_release_element(mid_node);

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head) || list_is_singular(head))
        return false;
    element_t *pos, *next_pos;
    bool check_val = false;

    list_for_each_entry_safe (pos, next_pos, head, list) {
        // if different
        if ((pos->list.next != head) &&
            !(strcmp(pos->value, next_pos->value))) {
            list_del_init(&pos->list);
            q_release_element(pos);
            check_val = true;
        } else if (check_val) {
            list_del_init(&pos->list);
            q_release_element(pos);
            check_val = false;
        }
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *first = head->next;
    for (; (first != head) && (first->next != head); first = first->next) {
        list_move(first, first->next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *prev_node = head, *node = head->next;
    do {
        prev_node->next = prev_node->prev;
        prev_node->prev = node;
        prev_node = node;
        node = node->next;
    } while (prev_node != head);
}


struct list_head *merge_lists(struct list_head *l1, struct list_head *l2)
{
    struct list_head *head = NULL;
    struct list_head **pptr = &head, **node = NULL;

    // find the element from l1 and l2.
    while (l1 && l2) {
        element_t *e1 = list_entry(l1, element_t, list);
        element_t *e2 = list_entry(l2, element_t, list);

        node = strcmp(e1->value, e2->value) < 0 ? &l1 : &l2;
        *pptr = *node;
        pptr = &(*pptr)->next;
        *node = (*node)->next;
    }

    // connect the rest of the list at the tail
    *pptr = (struct list_head *) ((uintptr_t) l1 | (uintptr_t) l2);
    return head;
}

struct list_head *divide_list(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *fast = head->next, *slow = head;
    for (; fast && fast->next; fast = fast->next->next) {
        slow = slow->next;
    }
    struct list_head *right, *left;
    right = slow->next;
    slow->next = NULL;

    left = divide_list(head);
    right = divide_list(right);

    return merge_lists(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if ((!head || list_empty(head)) || list_is_singular(head))
        return;

    //
    head->prev->next = NULL;
    head->next = divide_list(head->next);

    struct list_head *node = head->next, *prev = head;
    for (; node->next != NULL; node = node->next) {
        node->prev = prev;
        prev = node;
    }
    node->prev = prev;
    node->next = head;
    head->prev = node;
}


