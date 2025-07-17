#!/usr/bin/env python3

# Copyright (C) 2019-2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

import jsonrpclib

import argparse
import codecs
import logging
import string
import struct
import sys

# Characters that are considered to make up "simple names".
SIMPLE_NAME_CHARS = string.digits + string.ascii_lowercase

# Characters for valid hex strings in encoded names.
HEX_CHARS = string.digits + "abcdef"


class EjabberdServer:
  """
  The authentication service for a single XMPP server, which is
  tied to a particular Xid application string and also
  JSON-RPC endpoint (e.g. Xid on a particular network).
  """

  def __init__ (self, application, xidRpcUrl):
    self.application = application
    self.xidRpc = jsonrpclib.ServerProxy (xidRpcUrl)
    self.chain = self.xidRpc.getnullstate ()["chain"]

    # The log is filled in when the service gets added to an instance
    # of EjabberdXidAuth.
    self.log = None

  def unwrapGameState (self, data):
    """
    Verifies that the game-state JSON in data represents an up-to-date
    state (if not, an exception is raised).  Then unwraps the contained
    actual state from the "data" field and returns that.
    """

    if data["state"] != "up-to-date":
      self.log.critical ("xid is not up-to-date: %s" % data["state"])
      raise RuntimeError ("xid is not up-to-date")

    return data["data"]

  def isUser (self, xayaName):
    """
    Checks if the given username (already decoded to Xaya) is
    a registered user of Xid on this service.
    """

    data = self.xidRpc.getnamestate (name=xayaName)
    state = self.unwrapGameState (data)

    for entry in state["signers"]:
      if len (entry["addresses"]) == 0:
        continue

      if not "application" in entry:
        self.log.debug ("Found global signer key for %s" % xayaName)
        return True

      if entry["application"] == self.application:
        self.log.debug ("Found signer key for %s and application %s"
                          % (xayaName, app))
        return True

    self.log.debug ("No valid signer keys for %s and application %s"
                      % (xayaName, self.application))
    return False

  def authenticate (self, xayaName, pwd):
    """
    Checks if the given user (as decoded Xaya name) can be authenticated
    with the given password on this Xid service.
    """

    data = self.xidRpc.verifyauth (name=xayaName, application=self.application,
                                   password=pwd)
    state = self.unwrapGameState (data)

    self.log.debug ("Authentication state from xid: %s" % state["state"])
    return state["valid"]


class EjabberdXidAuth:
  """
  The main class for an external authentication script for ejabberd that
  uses xid to authenticate users on the XMPP server.  It has a mapping of
  potentially multiple XMPP server names / domains to the associated
  Xid authentication services, e.g. for different networks.

  Note that XMPP has certain restrictions on the usernames; in particular,
  names are treated in a case-insensitive way (other than Xaya).  Thus,
  names that are not "simple" (lowercase alphanumeric characters) are
  translated to x-hex, where hex is the hex-string of the name encoded as
  UTF-8.
  """

  def __init__ (self, services, logHandler, logLevel=logging.INFO):
    if logHandler is not None:
      self.setupLogging (logHandler, logLevel)

    self.serverNames = services
    for s in self.serverNames.values ():
      s.log = self.log

  def setupLogging (self, handler, logLevel):
    logFmt = "%(asctime)s %(name)s (%(levelname)s): %(message)s"
    handler.setFormatter (logging.Formatter (logFmt))

    self.log = logging.getLogger ("xidauth")
    self.log.setLevel (logLevel)
    self.log.addHandler (handler)

  def readCommand (self, inp):
    """
    Reads the next command from ejabberd from the given input stream.  The
    parsed and split command is returned as array.
    """

    length = inp.read (2)
    (n,) = struct.unpack (">h", length)

    cmd = inp.read (n).decode ("ascii")
    self.log.debug ("Got command: %s" % cmd)

    return cmd.split (":")

  def writeResult (self, outp, ok):
    """
    Writes the authentication result as boolean to the given output stream,
    so that ejabberd can parse it.
    """

    self.log.debug ("Writing result: %d" % ok)

    num = 0
    if ok:
      num = 1
    data = struct.pack (">hh", 2, num)

    outp.write (data)
    outp.flush ()

  def decodeXmppName (self, name):
    """
    Decodes the XMPP name to the underlying name on the Xaya platform.
    Returns None if the encoded form is not valid at all.
    """

    if len (name) == 0:
      self.log.warning ("Empty string passed as XMPP name")
      return None

    if name[:2] != "x-":
      for c in name:
        if not c in SIMPLE_NAME_CHARS:
          self.log.warning ("Invalid XMPP name: %s" % name)
          return None
      return name

    hexPart = name[2:]
    for c in hexPart:
      if not c in HEX_CHARS:
        self.log.warning ("Invalid XMPP name: %s" % name)
        return None

    if len (hexPart) % 2 != 0:
      self.log.warning ("Odd-length hex part: %s" % name)
      return None

    utf8Bytes = codecs.decode (hexPart, "hex")
    xayaName = codecs.decode (utf8Bytes, "utf-8")

    # The empty string is a special case of a name that gets encoded
    # with hex to "x-" so that it can be represented by a valid
    # non-empty XMPP name.
    if len (xayaName) == 0:
      return xayaName

    # If the name was hex-encoded, check that there is some non-simple character
    # in it.  Else we may get multiple XMPP names that decode to the same
    # Xaya name.
    for c in xayaName:
      if c not in SIMPLE_NAME_CHARS:
        return xayaName

    self.log.warning ("Simple name was hex-encoded: %s" % name)
    return None

  def isUser (self, name, server):
    """
    Checks whether the given XMPP name is a valid user for the given
    XMPP server.  This is the case if and only if there are any signers
    set that could potentially authenticate for the server.
    """

    if server not in self.serverNames:
      self.log.warning ("Server %s is not configured for xidauth" % server)
      return False

    xayaName = self.decodeXmppName (name)
    if xayaName is None:
      return False

    return self.serverNames[server].isUser (xayaName)

  def authenticate (self, name, server, pwd):
    """
    Checks whether the given name/password combination is authorised
    for the given server.
    """

    if server not in self.serverNames:
      self.log.warning ("Server %s is not configured for xidauth" % server)
      return False

    xayaName = self.decodeXmppName (name)
    if xayaName is None:
      return False

    return self.serverNames[server].authenticate (xayaName, pwd)

  def run (self, inp, outp):
    """
    Starts the main loop, reading commands from inp, processing them
    and writing the results to outp.  This method does not return (the
    process should be killed as needed).
    """

    servers = ""
    for n, s in self.serverNames.items ():
      servers += "\n  %s: %s (%s)" % (n, s.application, s.chain)
    self.log.info ("Running xid authentication script for servers:" + servers)

    self.log.info ("Starting main loop...")
    while True:
      cmd = self.readCommand (inp)

      ok = None
      if cmd[0] == "auth":
        assert len (cmd) == 4
        ok = self.authenticate (cmd[1], cmd[2], cmd[3])
      elif cmd[0] == "isuser":
        assert len (cmd) == 3
        ok = self.isUser (cmd[1], cmd[2])
      else:
        self.log.warning ("Ignoring unsupported command: %s" % cmd[0])
        ok = False

      assert ok is not None
      self.writeResult (outp, ok)


if __name__ == "__main__":
  desc = "ejabberd extauth script for authentication with xid"
  parser = argparse.ArgumentParser (description=desc)
  parser.add_argument ("--servers", required=True, nargs="+",
                       help="Server names to handle as server,application,rpcurl triplets")
  parser.add_argument ("--logfile", default="/var/log/ejabberd/xidauth.log",
                       help="filename for writing logs to")
  parser.add_argument ("--debug", action="store_true",
                       help="If enabled, turn on debug logging")
  args = parser.parse_args ()

  services = {}
  for s in args.servers:
    parts = s.split (",")
    if len (parts) != 3:
      sys.exit ("Invalid server triplet: %s" % s)
    [srv, app, rpcUrl] = parts
    services[srv] = EjabberdServer (app, rpcUrl)

  logLevel = logging.INFO
  if args.debug:
    logLevel = logging.DEBUG

  auth = EjabberdXidAuth (services, logging.FileHandler (args.logfile), logLevel=logLevel)
  auth.run (sys.stdin.buffer, sys.stdout.buffer)
