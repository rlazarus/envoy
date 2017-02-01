#pragma once

namespace Network {
namespace Address {

enum class Type { Ip, Pipe };

class Ip {
public:
  virtual ~Ip() {}

  virtual const std::string& addressAsString() const PURE;
  virtual uint32_t port() const PURE;
};

class Pipe {};

class Instance;
typedef std::shared_ptr<const Instance> InstancePtr;

class Instance {
public:
  virtual ~Instance() {}

  virtual bool operator==(const Instance& rhs) const PURE;
  virtual const std::string& asString() const PURE;
  virtual Type type() const PURE;
  virtual const Ip* ip() const PURE;
};

} // Address
} // Network
