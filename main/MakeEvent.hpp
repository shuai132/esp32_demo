#pragma once

#ifndef NO_FUNCTIONAL
#include <functional>
#define CB_TYPE(...) std::function<void(__VA_ARGS__)>
#else
#define CB_TYPE(...) void(*)(__VA_ARGS__)
#endif
#include <vector>
#include <utility>

#define _MAKE_EVENT_DEF_SET(name, ...) \
        using name##Cb##_t = CB_TYPE(__VA_ARGS__); \
        private: \
        name##Cb##_t _cb##name = nullptr; \

#define _MAKE_EVENT_FUN_SET(name, ...) \
        public: \
        inline void set##name##Cb(name##Cb##_t cb) { \
            _cb##name = std::move(cb); \
        } \

#define _MAKE_EVENT_TRI_SET(name, ...) \
        public: \
        template<typename... Args> \
        inline void on##name(Args&&... args) { \
            if (_cb##name) \
                _cb##name(std::forward<Args>(args)...); \
        } \

#define MAKE_EVENT_SET(name, ...) \
        _MAKE_EVENT_DEF_SET(name, __VA_ARGS__) \
        _MAKE_EVENT_FUN_SET(name, __VA_ARGS__) \
        _MAKE_EVENT_TRI_SET(name, __VA_ARGS__) \


#define _MAKE_EVENT_DEF_ADD(name, ...) \
        struct name##CbID_t { \
            name##CbID_t(unsigned int value = 0) : value(value) {} /* NOLINT(google-explicit-constructor) */ \
            unsigned int value; \
            name##CbID_t operator++() { \
                value++; \
                return *this; \
            } \
            bool operator==(const name##CbID_t& other) const { \
                return value == other.value; \
            } \
        }; \
        using name##Cb##_t = CB_TYPE(__VA_ARGS__); \
        using name##Cb##Pair_t = std::pair<name##CbID_t, CB_TYPE(__VA_ARGS__)>; \
        private: \
        name##CbID_t _cbs##name##ID##Count = 0; \
        std::vector<name##Cb##Pair_t> _cbs##name; \

#define _MAKE_EVENT_FUN_ADD(name, ...) \
        public: \
        inline name##CbID_t add##name##Cb(name##Cb##_t cb) { \
            auto id = ++_cbs##name##ID##Count; \
            _cbs##name.emplace_back(std::make_pair(id, std::move(cb))); \
            return id; \
        } \
        inline name##CbID_t remove##name##CbById(name##CbID_t id) { \
            for (auto it = _cbs##name.begin(); it != _cbs##name.end();) { \
                const auto& item = *it; \
                if (item.first == id) { \
                    _cbs##name.erase(it); \
                    return id; \
                } else { \
                    it++; \
                } \
            } \
            return 0; \
        } \
        inline void removeAll##name##Cbs(){ \
            _cbs##name.clear(); \
        } \

#define _MAKE_EVENT_TRI_ADD(name, ...) \
        public: \
        template<typename... Args> \
        inline void on##name(Args&&... args) { \
            for (const auto& cb : _cbs##name) { \
                cb.second(std::forward<Args>(args)...); \
            } \
        } \

#define MAKE_EVENT_ADD(name, ...) \
        _MAKE_EVENT_DEF_ADD(name, __VA_ARGS__) \
        _MAKE_EVENT_FUN_ADD(name, __VA_ARGS__) \
        _MAKE_EVENT_TRI_ADD(name, __VA_ARGS__) \


#define MAKE_EVENT MAKE_EVENT_SET

