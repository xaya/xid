// Copyright (C) 2021 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef XID_LIGHT_HPP
#define XID_LIGHT_HPP

#include <memory>
#include <string>

namespace xid
{

/**
 * A simple wrapper class around the xid-light logic, which allows running
 * a xid-light instance inside another process if desired.
 */
class LightInstance
{

private:

  class Impl;

  /** The actual implementation, hidden from the header.  */
  std::unique_ptr<Impl> impl;

public:

  /**
   * Constructs a new instance with the given base configuration.
   */
  explicit LightInstance (const std::string& endpoint, int rpcPort);

  ~LightInstance ();

  /**
   * Marks the local RPC server to bind to localhost only.
   */
  void EnableListenLocally ();

  /**
   * Sets the trusted root CA file for the TLS connection to the endpoint
   * (in case it is https).
   */
  void SetCaFile (const std::string& path);

  /**
   * Runs the main loop.  It starts the local RPC server (forwarding
   * requests to the configured REST endpoint), and blocks until the
   * server is shut down through RPC.
   */
  void Run ();

};

} // namespace xid

#endif // XID_LIGHT_HPP
