# Copyright (C) 2025 The Xaya developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from . import auth_pb2

import base64
import time


Protocol = auth_pb2.Protocol


class Credentials:
  """
  This class represents xidauth credentials, which are the data encoded
  inside a "password".  It handles the conversion to/from passwords.
  """

  def __init__ (self, name, app):
    self.name = name
    self.app = app
    self.data = auth_pb2.AuthData ()

  # Password

  @property
  def password (self):
    """Encodes this Credentials instance into a password string."""

    return base64.b64encode (self.data.SerializeToString ())

  @password.setter
  def password (self, pwd):
    """
    Decodes a password and updates the respective data stored
    in this instance from it.
    """

    decoded = base64.b64decode (pwd)
    self.data.ParseFromString (decoded)

  # Signature

  @property
  def raw_signature (self):
    return self.data.signature_bytes

  @raw_signature.setter
  def raw_signature (self, value):
    self.data.signature_bytes = value

  # Expiry

  @property
  def expiry (self):
    if self.data.HasField ("expiry"):
      return self.data.expiry
    return None

  @expiry.setter
  def expiry (self, val):
    self.data.expiry = val

  @expiry.deleter
  def expiry (self):
    self.data.ClearField ("expiry")

  @property
  def expired (self):
    if not self.data.HasField ("expiry"):
      return False
    return self.data.expiry < time.time ()

  # Protocol

  @property
  def protocol (self):
    if not self.data.HasField ("protocol"):
      return Protocol.XID_GSP
    return self.data.protocol

  @protocol.setter
  def protocol (self, val):
    self.data.protocol = val

  @protocol.deleter
  def protocol (self):
    self.data.ClearField ("protocol")

  # Extra fields

  @property
  def extra (self):
    return self.data.extra

  @extra.setter
  def extra (self, value):
    self.data.extra.clear ()
    for k, v in value.items ():
      self.data.extra[k] = v

  @extra.deleter
  def extra (self):
    self.data.extra.clear ()


class InvalidCredentialsError (Exception):
  """
  Exception class that is thrown on validating credentials if they
  are not valid.
  """

  def __init__ (self, msg):
    super ().__init__ (msg)
