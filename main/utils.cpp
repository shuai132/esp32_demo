#include "utils.h"

std::shared_ptr<asio::steady_timer>
utils::steady_timer(
        asio::io_context* context,
        std::function<void()> cb,
        uint32_t ms,
        const bool& loop,
        bool run_first) {
    if (run_first) {
        cb();
    }

    auto timer = std::make_shared<asio::steady_timer>(*context, std::chrono::milliseconds(ms));
    timer->async_wait([timer, context, cb = std::move(cb), ms, &loop](const asio::error_code& e) {
        if (not e) {
            cb();
        }

        if (loop) {
            steady_timer(context, cb, ms, loop);
        }
    });
    return timer;
}
