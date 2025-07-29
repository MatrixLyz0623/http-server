#include "dispatcher.h"

void Dispatcher::registerHandler(const std::string& method, const std::string& path, Handler handler) {
    exactRoutes_[method][path] = std::move(handler);
}

void Dispatcher::registerHandlerPrefix(const std::string& method, const std::string& prefix, Handler handler) {
    prefixRoutes_[method].emplace_back(prefix, std::move(handler));
}

HttpResponse Dispatcher::dispatch(const HttpRequestParser& request) const {
    const auto& method = request.get_method();
    const auto& path = request.get_path();

    auto it = exactRoutes_.find(method);
    if (it != exactRoutes_.end()) {
        const auto& pathMap = it->second;
        auto mit = pathMap.find(path);
        if (mit != pathMap.end()) {
            return mit->second(request);
        }
    }

    auto iit = prefixRoutes_.find(method);
    if (iit != prefixRoutes_.end()) {
        const auto& prefixvec = iit->second;
        for (const auto& [prefix, handler] : prefixvec) {
            if (path.rfind(prefix, 0) == 0) {
                return handler(request);
            }
        }
    }

    return HttpResponse::notFound();
}
