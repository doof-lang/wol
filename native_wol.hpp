#pragma once

#include "doof_runtime.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace doof_wol {

doof::Result<std::shared_ptr<std::vector<uint8_t>>, std::string> magic_packet(const std::string& macAddress);
doof::Result<void, std::string> wake(const std::string& macAddress, const std::string& broadcastAddress, int32_t port);

} // namespace doof_wol
