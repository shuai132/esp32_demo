#pragma once

#include <algorithm>
#include <chrono>
#include <memory>

#include "asio.hpp"

namespace utils {

/**
 * 遍历容器
 * @tparam Container
 * @tparam Func
 * @param r
 * @param f
 */
template <typename Container, typename Func>
inline void foreach(Container c, Func f) {
    std::for_each(c.cbegin(), c.cend(), std::move(f));
}

/**
 * 定时器
 * @param context
 * @param cb
 * @param ms
 * @param loop      循环执行
 * @param run_first 首次执行
 * @return
 */
std::shared_ptr<asio::steady_timer> steady_timer(
        asio::io_context* context,
        std::function<void()> cb,
        uint32_t ms,
        const bool& loop = false,
        bool run_first = false);

#define FOR(i, n)   for(std::remove_reference<std::remove_const<typeof(n)>::type>::type i = 0; (i) < (n); (i)++)

}
