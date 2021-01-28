#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "scanner.h"
#include "value.h"
#include "object_defs.h"
#include "vm.h"
#include "native.h"

ObjBoundMethod *newBoundMethod(VM *vm, Value receiver, ObjClosure *method);
ObjClass *newClass(VM *vm, const char *name);
ObjNativeClass *newNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor);
ObjNativeMethod *newNativeMethod(VM *vm, NativeMethod function, uint8_t arity, bool isStatic, Value name);
ObjClosure *newClosure(VM *vm, ObjFunction *function);
ObjFunction *newFunction();
Obj *newInstance(VM *vm, ObjClass *klass);
ObjNative *newNativeFunction(VM *vm, NativeFn function);
Value takeString(VM *vm, char *chars, int length);
Value copyString(VM *vm, const char *chars, int length);
ObjUpvalue *newUpvalue(VM *vm, Value *slot);
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
