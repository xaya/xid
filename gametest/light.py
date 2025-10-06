#!/usr/bin/env python3
# coding=utf8

# Copyright (C) 2020-2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

"""
Tests the light mode of xid.
"""

from xidtest import XidTest

import jsonrpclib

import http.server
import logging
import os
import threading
import subprocess
import time


class XidLight ():
  """
  A context manager that runs a xid-light process while active.
  """

  def __init__ (self, basedir, binary, port, restEndpoint, cafile):
    self.log = logging.getLogger ("xid-light")

    self.basedir = basedir
    self.binary = binary
    self.port = port
    self.restEndpoint = restEndpoint
    self.cafile = cafile

    self.rpcurl = "http://localhost:%d" % self.port
    self.proc = None

  def __enter__ (self):
    assert self.proc is None

    self.log.info ("Starting xid-light at port %d" % self.port)

    args = [self.binary]
    args.extend (["--game_rpc_port", "%d" % self.port])
    args.extend (["--rest_endpoint", self.restEndpoint])
    args.extend (["--cafile", self.cafile])

    envVars = dict (os.environ)
    envVars["GLOG_log_dir"] = self.basedir

    self.proc = subprocess.Popen (args, env=envVars)
    self.rpc = jsonrpclib.ServerProxy (self.rpcurl)

    # Wait a bit for the process to be up (which is fast anyway).
    time.sleep (0.1)

    return self

  def __exit__ (self, exc, value, traceback):
    assert self.proc is not None

    self.log.info ("Stopping xid-light process...")
    self.rpc._notify.stop ()
    self.proc.wait ()
    self.proc = None


class DummyRequestHandler (http.server.BaseHTTPRequestHandler):
  """
  HTTP request handler that just returns given constant data.  This is
  used to test what happens if the REST endpoint returns invalid JSON.
  """

  # Data to return to requests (as string)
  responseData = ""

  # Content type to return
  responseType = ""

  def do_GET (self):
    self.send_response (200)
    self.send_header ("Content-Type", self.responseType)
    self.end_headers ()
    self.wfile.write (self.responseData.encode ("ascii"))


class DummyServer ():
  """
  Context handler that runs a simple HTTP server.  The server just returns
  some dummy data to all requests.  We use that to verify what xid-light
  does if it receives invalid JSON from the REST endpoint.
  """

  def __init__ (self, port):
    self.port = port
    self.srv = None
    self.runner = None

  def __enter__ (self):
    assert self.srv is None
    assert self.runner is None

    self.srv = http.server.HTTPServer (("localhost", self.port),
                                       DummyRequestHandler)

    self.runner = threading.Thread (target=self.srv.serve_forever,
                                    kwargs={"poll_interval": 0.1})
    self.runner.start ()

  def __exit__ (self, exc, value, traceback):
    assert self.srv is not None
    assert self.runner is not None

    self.srv.shutdown ()
    self.runner.join ()
    self.runner = None
    self.srv.server_close ()
    self.srv = None


class LightModeTest (XidTest):

  def startLight (self, endpoint):
    """
    Starts a new xid-light process (as context manager) at our chosen
    light port and with the given REST API endpoint.
    """

    top_builddir = os.getenv ("top_builddir")
    if top_builddir is None:
      top_builddir = ".."
    binary = os.path.join (top_builddir, "src", "xid-light")

    top_srcdir = os.getenv ("top_srcdir")
    if top_srcdir is None:
      top_srcdir = ".."
    cafile = os.path.join (top_srcdir, "data", "letsencrypt.pem")

    return XidLight (self.basedir, binary, self.lightPort, endpoint, cafile)

  def run (self):
    self.generate (101)
    addr = self.env.createSignerAddress ()
    self.sendMove ("domob", {"s": {"g": [addr]}})
    self.generate (1)

    self.lightPort = next (self.ports)
    restPort = next (self.ports)
    invalidPort = next (self.ports)
    dummyPort = next (self.ports)
    restEndpoint = "http://localhost:%d" % restPort

    self.mainLogger.info ("Enabling the REST interface...")
    self.stopGameDaemon ()
    self.startGameDaemon (extraArgs=["--rest_port=%d" % restPort])
    self.syncGame ()

    self.mainLogger.info ("Testing light mode with local REST endpoint...")
    with self.startLight (restEndpoint) as l:
      self.assertEqual (l.rpc.getnullstate (), self.rpc.game.getnullstate ())
      for name in ["domob", "foo/bar", "", "abc def", u"kräfti"]:
        self.assertEqual (l.rpc.getnamestate (name=name),
                          self.rpc.game.getnamestate (name=name))

      authmsg = l.rpc.getauthmessage (name="domob", application="app", data={})
      sgn = self.env.signMessage (addr, authmsg["authmessage"])
      pwd = l.rpc.setauthsignature (password=authmsg["password"], signature=sgn)
      self.assertEqual (self.getRpc ("verifyauth", name="domob",
                                     application="app", password=pwd), {
        "valid": True,
        "state": "valid",
        "expiry": None,
        "extra": {},
      })

    self.mainLogger.info ("Testing connection error on REST endpoint...")
    with self.startLight (restEndpoint + "/invalid") as l:
      self.expectError (-32603, ".*HTTP.*404.*", l.rpc.getnullstate)
    invalidConnection = "http://localhost:%d" % invalidPort
    with self.startLight (invalidConnection) as l:
      self.expectError (-32603, ".*Failed to connect.*", l.rpc.getnullstate)

    self.mainLogger.info ("Testing REST endpoint returning not JSON...")
    DummyRequestHandler.responseType = "text/plain"
    DummyRequestHandler.responseData = "{}"
    with DummyServer (dummyPort), \
         self.startLight ("http://localhost:%d" % dummyPort) as l:
      self.expectError (-32603, ".*expected JSON.*", l.rpc.getnullstate)

    self.mainLogger.info ("Testing REST endpoint returning invalid JSON...")
    DummyRequestHandler.responseType = "application/json"
    DummyRequestHandler.responseData = "invalid JSON"
    with DummyServer (dummyPort), \
         self.startLight ("http://localhost:%d" % dummyPort) as l:
      self.expectError (-32603, ".*JSON parser.*", l.rpc.getnullstate)


if __name__ == "__main__":
  LightModeTest ().main ()
