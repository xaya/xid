#!/usr/bin/env python3

# Copyright (C) 2019-2020 The Xaya developers
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


class EjabberdXidAuth (object):
  """
  The main class for an external authentication script for ejabberd that
  uses xid to authenticate users on the XMPP server.

  Note that XMPP has certain restrictions on the usernames; in particular,
  names are treated in a case-insensitive way (other than Xaya).  Thus,
  names that are not "simple" (lowercase alphanumeric characters) are
  translated to x-hex, where hex is the hex-string of the name encoded as
  UTF-8.
  """

  def __init__ (self, serverNames, xidRpcUrl, logHandler):
    self.serverNames = serverNames
    self.xidRpc = jsonrpclib.ServerProxy (xidRpcUrl)

    if logHandler is not None:
      self.setupLogging (logHandler)

  def setupLogging (self, handler):
    logFmt = "%(asctime)s %(name)s (%(levelname)s): %(message)s"
    handler.setFormatter (logging.Formatter (logFmt))

    self.log = logging.getLogger ("xidauth")
    self.log.setLevel (logging.INFO)
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

  def isUser (self, name, server):
    """
    Checks whether the given XMPP name is a valid user for the given
    XMPP server.  This is the case if and only if there are any signers
    set that could potentially authenticate for the server.
    """

    if server not in self.serverNames:
      self.log.warning ("Server %s is not configured for xidauth" % server)
      return False
    app = self.serverNames[server]

    xayaName = self.decodeXmppName (name)
    if xayaName is None:
      return False

    data = self.xidRpc.getnamestate (name=xayaName)
    state = self.unwrapGameState (data)

    for entry in state["signers"]:
      if len (entry["addresses"]) == 0:
        continue

      if not "application" in entry:
        self.log.debug ("Found global signer key for %s" % xayaName)
        return True

      if entry["application"] == app:
        self.log.debug ("Found signer key for %s and application %s"
                          % (xayaName, app))
        return True

    self.log.debug ("No valid signer keys for %s and application %s"
                      % (xayaName, app))
    return False

  def authenticate (self, name, server, pwd):
    """
    Checks whether the given name/password combination is authorised
    for the given server.
    """

    if server not in self.serverNames:
      self.log.warning ("Server %s is not configured for xidauth" % server)
      return False
    app = self.serverNames[server]

    xayaName = self.decodeXmppName (name)
    if xayaName is None:
      return False

    data = self.xidRpc.verifyauth (name=xayaName, application=app, password=pwd)
    state = self.unwrapGameState (data)

    self.log.debug ("Authentication state from xid: %s" % state["state"])
    return state["valid"]

  def run (self, inp, outp):
    """
    Starts the main loop, reading commands from inp, processing them
    and writing the results to outp.  This method does not return (the
    process should be killed as needed).
    """

    servers = ""
    for s, a in self.serverNames.items ():
      servers += "\n  %s: %s" % (s, a)
    self.log.info ("Running xid authentication script for servers:" + servers)

    data = self.xidRpc.getnamestate (name="xaya")
    self.log.info ("Xid is %s at height %d" % (data["state"], data["height"]))

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
  parser.add_argument ("--xid_rpc_url", required=True,
                       help="JSON-RPC URL for xid's RPC interface")
  parser.add_argument ("--servername", required=True,
                       help="name of the XMPP server")
  parser.add_argument ("--application", required=True,
                       help="application name for xid")
  parser.add_argument ("--logfile", default="/var/log/ejabberd/xidauth.log",
                       help="filename for writing logs to")
  args = parser.parse_args ()

  auth = EjabberdXidAuth ({args.servername: args.application}, args.xid_rpc_url,
                          logging.FileHandler (args.logfile))
  auth.run (sys.stdin.buffer, sys.stdout.buffer)
