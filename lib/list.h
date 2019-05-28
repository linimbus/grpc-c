
#ifndef __GRPC_C_LIST_H__
#define __GRPC_C_LIST_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define GRPC_LIST_EMPTY(head)\
    ((head)->next == (head))

#define GRPC_LIST_NEXT(item)\
    ((item)->next)

#define GRPC_LIST_BEFORE(item)\
    ((item)->prev)

#define GRPC_LIST_INIT(head) \
    (head)->next = (head); \
    (head)->prev = (head);

#define GRPC_LIST_ADD(item,where) \
    (item)->next = (where)->next; \
    (item)->prev = (where); \
    (where)->next->prev = (item); \
    (where)->next = (item);

#define GRPC_LIST_ADD_BEFORE(item,where) \
    (item)->prev = (where)->prev; \
    (item)->next = (where); \
    (where)->prev->next = (item); \
    (where)->prev = (item);

#define GRPC_LIST_REMOVE(item) \
    (item)->next->prev = (item)->prev; \
    (item)->prev->next = (item)->next;

#define GRPC_LIST_OFFSET(item, struct, stitem) \
    (struct *)((char *)item - (char *)(&((struct *)0)->stitem))

#define GRPC_LIST_TRAVERSAL(pstItem, pstHead) \
    for( (pstItem) = (pstHead)->next; (pstItem) != (pstHead) ; (pstItem) = (pstItem)->next )

#define GRPC_LIST_TRAVERSAL_REMOVE(pstItem,pstTemp,pstHead) \
        for( (pstItem) = (pstHead)->next,(pstTemp) = (pstItem)->next;\
             (pstItem) != (pstHead) ;\
             (pstItem) = (pstTemp), (pstTemp) = (pstTemp)->next)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
