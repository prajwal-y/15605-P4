/** @file list.c
 *  @brief This function implements the list operations
 *         for a doubly linked list.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include<list/list.h>

/** @brief initialize the list
 *
 *  Initialize the head by making it point to itself
 *
 *  @param head the head of the doubly linked circular list
 */
void init_head(list_head *head) {
    head->next = head;
    head->prev = head;
}

/** @brief add a node to the list
 *
 *  This is not thread safe and must be protected by a mutex
 *  by the programmer.
 *
 *  @param new_node The new node to be implemented in the list
 *  @param prev the node previous to the node being inserted
 *  @param next the node next to the node being inserted
 */
void add_to_list(list_head *new_node, list_head *prev, 
                               list_head *next) {
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
    next->prev = new_node;
}

/** @brief add a node to the tail of the list
 *
 *  @param new_node The new node to be implemented in the list
 *  @param head The head of the list
 */
void add_to_tail(list_head *new_node, list_head *head) {
    add_to_list(new_node, head->prev, head);
}

/** @brief add a node to the head of the list
 *
 *  @param new_node The new node to be implemented in the list
 *  @param head The head of the list
 */
void add_to_head(list_head *new_node, list_head *head) {
    add_to_list(new_node, head, head->next);
}

/** @brief delete an entry from the list 
 *
 *  Does not free space associated with this node
 *
 *  @param node node to be deleted
 */
void del_entry(list_head *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/** @brief get the first entry in the list
 *
 *  @param head the head of the list
 *  @return the first node in the list
 */
list_head *get_first(list_head *head) {
    if (head->next == head) {
        return NULL;
    }
    return head->next;
}

/** @brief get the last entry in the list
 *
 *  @param head the head of the list
 *  @return the first node in the list
 */
list_head *get_last(list_head *head) {
    if (head->prev == head) {
        return NULL;
    }
    return head->prev;
}

/** @brief concatenate two lists. 
 *
 *  Add the second list to the first. The head of the new list is the 
 *  head of the first list. Both the lists have to be locked appropriately
 *  by the caller.
 *
 *  @param first_list_head the dummy head of the first list
 *  @param second_list_head the dummy head of the second list
 *  @return the first node in the list
 */
void concat_lists(list_head *first_list_head, list_head *second_list_head) {
    list_head *head_one = get_first(first_list_head);
    list_head *tail_one = get_last(first_list_head);
    list_head *head_two = get_first(second_list_head);
    list_head *tail_two = get_last(second_list_head);

    if ((head_one == NULL && head_two == NULL) || 
        head_two == NULL) {
        return;
    }

    if (head_one == NULL) {
        first_list_head->next = head_two;
        first_list_head->prev = tail_two;
        head_two->prev = first_list_head;
        tail_two->next = first_list_head;
        init_head(second_list_head);
        return;
    }
    tail_one->next = head_two;
    head_two->prev = tail_one;
    tail_two->next = first_list_head;
    first_list_head->prev = tail_two;
}
