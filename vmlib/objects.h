#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "scanner.h"
#include "value.h"
#include "object_defs.h"
#include "vm.h"
#include "native.h"

ObjBoundMethod *newBoundMethod(VM *vm, Value receiver, ObjClosure *method);
ObjClass *newClass(VM *vm, const char *name, ClassType classType, bool final);
ObjNativeClass *newNativeClass(VM *vm, const char *name, NativeConstructor constructor, NativeDestructor destructor, ClassType classType, bool final);
ObjNativeMethod *newNativeMethod(VM *vm, NativeMethod function, uint8_t arity, bool isStatic, Value name);
ObjClosure *newClosure(VM *vm, ObjFunction *function);
ObjFunction *newFunction(VM *vm, const char *filename, Value module);
Obj *newInstance(VM *vm, ObjClass *klass);
ObjNative *newNativeFunction(VM *vm, NativeFn function);
Value takeString(VM *vm, char *chars, int length);
Value copyString(VM *vm, const char *chars, size_t length);
ObjUpvalue *newUpvalue(VM *vm, Value *slot);
void printObject(Value value);
const char *objTypeName(ObjType type);
const char *getOperatorString(OPERATOR operator_);
OPERATOR getOperatorFromToken(TokenType_t token);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
