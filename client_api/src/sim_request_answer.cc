#include "sim_request_answer.hh"

namespace panosc
{
sim_request_answer::sim_request_answer(std::string message)
{
    nlohmann::json j = nlohmann::json::parse(message);
    _answer          = j["answer"];
}

} // namespace panosc
