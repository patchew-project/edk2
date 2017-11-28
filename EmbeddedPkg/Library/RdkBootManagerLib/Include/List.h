#ifndef __LIST_H__
#define __LIST_H__

#define OFFSETOF(TYPE, MEMBER) ((long unsigned int) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define CONTAINER_OF(Ptr, Type, Member) ({			\
	const typeof( ((Type *)0)->Member ) *__Mptr = (Ptr);	\
	(Type *)( (char *)__Mptr - OFFSETOF(Type,Member) );})

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

typedef struct ListHead {
    struct ListHead *Next, *Prev;
} LIST_HEAD;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

static inline void INIT_LIST_HEAD(LIST_HEAD *List)
{
    List->Next = List;
    List->Prev = List;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __ListAdd(LIST_HEAD *New,
                             LIST_HEAD *Prev,
                             LIST_HEAD *Next)
{
    Next->Prev = New;
    New->Next = Next;
    New->Prev = Prev;
    Prev->Next = New;
}

/**
 * ListAdd - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void ListAdd(LIST_HEAD *New, LIST_HEAD *Head)
{
    __ListAdd(New, Head, Head->Next);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __ListDel(LIST_HEAD *Prev, LIST_HEAD *Next)
{
    Next->Prev = Prev;
    Prev->Next = Next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void ListDel(LIST_HEAD *Entry)
{
    __ListDel(Entry->Prev, Entry->Next);
    Entry->Next = NULL;
    Entry->Prev = NULL;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int ListEmpty(const LIST_HEAD *Head)
{
    return Head->Next == Head;
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &LIST_HEAD pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_ENTRY(Ptr, Type, Member) \
	CONTAINER_OF(Ptr, Type, Member)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY(Pos, Head, Member)				\
	for (Pos = LIST_ENTRY((Head)->Next, typeof(*Pos), Member);	\
	     &Pos->Member != (Head);					\
	     Pos = LIST_ENTRY(Pos->Member.Next, typeof(*Pos), Member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY_SAFE(Pos, N, Head, Member)			\
	for (Pos = LIST_ENTRY((Head)->Next, typeof(*Pos), Member),	\
		N = LIST_ENTRY(Pos->Member.Next, typeof(*Pos), Member);	\
	     &Pos->Member != (Head);					\
	     Pos = N, N = LIST_ENTRY(N->Member.Next, typeof(*N), Member))

#endif /* __LIST_H__ */
