#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "scanner.h"
#include "value.h"
#include "object_defs.h"
#include "vm.h"

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method);
ObjClass *newClass(const char *name);
ObjNativeClass *newNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor);
ObjNativeMethod *newNativeMethod(Value receiver, NativeMethod function, bool isStatic);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
Obj *newInstance(VM *vm, ObjClass *klass);
ObjNative *newNativeFunction(NativeFn function);
Value takeString(char *chars, int length);
Value copyString(const char *chars, int length);
ObjUpvalue *newUpvalue(Value *slot);
void printObject(Value value);
const char *objTypeName(ObjType type);
const char *getOperatorString(OPERATOR operator);
OPERATOR getOperatorFromToken(TokenType token);

void registerStringClass(Value klass);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
