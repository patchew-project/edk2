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
	for (Pos = LIST_ENTRY((Head)->ForwardLink, typeof(*Pos), Member);	\
	     &Pos->Member != (Head);					\
	     Pos = LIST_ENTRY(Pos->Member.ForwardLink, typeof(*Pos), Member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY_SAFE(Pos, N, Head, Member)			\
	for (Pos = LIST_ENTRY((Head)->ForwardLink, typeof(*Pos), Member),	\
		N = LIST_ENTRY(Pos->Member.ForwardLink, typeof(*Pos), Member);	\
	     &Pos->Member != (Head);					\
	     Pos = N, N = LIST_ENTRY(N->Member.ForwardLink, typeof(*N), Member))

#endif /* __LIST_H__ */
