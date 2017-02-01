#pragma once

namespace Network {
namespace Address {

class IpInstance : public Instance {
public:
  IpInstance(const std::string& address, uint32_t port);

  // Network::Address::Instance
  bool operator==(const Instance& rhs) const override;
  const std::string& asString() const override;
  Type type() const override;
  const Ip* ip() const override;
};

} // Address
} // Network
