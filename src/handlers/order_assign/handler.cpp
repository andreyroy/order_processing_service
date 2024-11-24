#include "handler.hpp"

namespace handlers::order::assign {

std::string Handler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return "your_body";
}

}  // namespace handlers::order::assign
