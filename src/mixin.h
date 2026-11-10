#pragma once

// this is silly i think but it works
#define MIXIN_H_CAT_IMPL(a, b) a##b
#define MIXIN_H_CAT(a, b) MIXIN_H_CAT_IMPL(a, b)

namespace AtType {
    enum AtType {
        HEAD,
        INVOKE,
        TAIL,
        RETURN,
    };
}

struct AtData {
    const AtType::AtType type;
    const char* where;
};

struct InjectData {
    const char* target;
    const AtData at;
};

struct WrapOperationData {
    const char* target;
    const AtData at;
};

inline InjectData InjectFunction(const char* target, AtData at) {
    return { target, at };
}

#define Inject(target, at) \
namespace MIXIN_H_CAT(mixin_namespace_, __LINE__) { \
    static const InjectData inject_ = InjectFunction(target, at); \
    __attribute__((used, retain))
#define EndInject }

inline WrapOperationData WrapOperationFunction(const char* target, AtData at) {
    return { target, at };
}

#define WrapOperation(target, at) \
namespace MIXIN_H_CAT(mixin_namespace_, __LINE__) { \
    static const WrapOperationData wrapop_ = WrapOperationFunction(target, at); \
    __attribute__((used, retain))
#define EndWrapOperation }

inline AtData At(AtType::AtType type, const char* where) {
    return { .type = type, .where = where };
}

inline AtData At(AtType::AtType type) {
    return { .type = type, .where = nullptr };
}