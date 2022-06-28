#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "scanner.h"
#include "value.h"
#include "object_defs.h"
#include "vm.h"
#include "native.h"

ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method);
ObjClass *newClass(const char *name, ClassType classType, bool final);
ObjNativeClass *newNativeClass(
    VM *vm,
    const char *name,
    NativeConstructor constructor,
    NativeDestructor destructor,
    MarkNativeObject marker,
    ClassType classType,
    size_t dataSize,
    bool final);
ObjNativeMethod *newNativeMethod(NativeMethod function, uint8_t arity, bool isStatic, Value name);
ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction(const char *filename, Value module);
Obj *newInstance(VM *vm, ObjClass *klass);
ObjNative *newNativeFunction(NativeFn function);
Value takeString(VM *vm, char *chars, int length);
Value copyString(VM *vm, const char *chars, size_t length);
ObjUpvalue *newUpvalue(Value *slot);
void printObject(Value value);
const char *objTypeName(ObjType type);
const char *getOperatorString(OPERATOR operator_);
const char* getClassNameFromInstance(VALUE instance);
OPERATOR getOperatorFromToken(TokenType_t token);

static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

static inline bool isObjOfStdlibClassType(Value value, ClassType type)
{
    return (IS_INSTANCE(value) || IS_NATIVE_INSTANCE(value)) &&
        AS_INSTANCE(value)->klass->classType == type;
}

#endif
