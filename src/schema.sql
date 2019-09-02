-- Copyright (C) 2019 The Xaya developers
-- Distributed under the MIT software license, see the accompanying
-- file COPYING or http://www.opensource.org/licenses/mit-license.php.

-- =============================================================================

-- The mapping between Xaya names that have been registered for Xid and
-- the signer keys that are allowed for them.  Each Xaya name can have
-- multiple valid signer keys, which is represented by multiple rows
-- in the table.
CREATE TABLE IF NOT EXISTS `signers` (

  -- The Xaya name for which the signer key is registered.
  `name` TEXT NOT NULL,

  -- A handle for the application for which the signer is valid.  If this is
  -- NULL, then the key is valid for all applications (the entire account).
  `application` TEXT NULL,

  -- The signer key itself.  This is a Xaya address represented as string, with
  -- which messages are signed to signal intents.
  `address` TEXT NOT NULL

);

-- We need to look up signer keys by name.  That is the main request for the
-- Xid application itself.  We could also add indices that cover individual
-- addresses.  But since the number of addresses and applications for one
-- name is likely very small, such an index is not really necessary.
CREATE INDEX IF NOT EXISTS `signers_name` ON `signers` (`name`);

-- =============================================================================

-- Data about associated crypto addresses with names.
CREATE TABLE IF NOT EXISTS `addresses` (

  -- The Xaya name for which the association is.
  `name` TEXT NOT NULL,

  -- Identifier for the crypto token/coin that this represents.
  `key` TEXT NOT NULL,

  -- The associated address/value.
  `address` TEXT NOT NULL,

  PRIMARY KEY (`name`, `key`)

);

-- =============================================================================
