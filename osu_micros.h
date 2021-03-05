//
// Created by hsyuan on 2021-03-05.
//

#ifndef PROJECT_OSU_MICROS_H
#define PROJECT_OSU_MICROS_H

#define OSU_RETURN_EXP_IF_FAIL(cond, exp) if (!(cond)) { fprintf(stderr, "Assert failed: %s in %s:%d\n", #cond, __FUNCTION__, __LINE__); exp;}

#define OSU_CONTAINER_OF(ptr, type, member)  ((type *) ((char *) (ptr) - offsetof(type, member)))




#endif //PROJECT_OSU_MICROS_H
