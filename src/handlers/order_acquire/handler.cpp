#include "handler.hpp"

namespace handlers::order::acquire {

std::string Handler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return "your_body";
}

}  // namespace handlers::order::acquire
