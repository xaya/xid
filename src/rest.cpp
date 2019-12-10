// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rest.hpp"

#include "gamestatejson.hpp"

#include <glog/logging.h>

namespace xid
{

class RestApi::HttpError : public std::runtime_error
{

private:

  /** The HTTP status code to return.  */
  const int httpCode;

public:

  explicit HttpError (const int c, const std::string& msg)
    : runtime_error(msg), httpCode(c)
  {}

  /**
   * Returns the status code.
   */
  int
  GetStatusCode () const
  {
    return httpCode;
  }

};

RestApi::~RestApi ()
{
  if (daemon != nullptr)
    Stop ();
}

namespace
{

/**
 * Encapsulated MHD response.
 */
class Response
{

private:

  /** Underlying MHD response struct.  */
  struct MHD_Response* resp = nullptr;

  /** HTTP status code to use.  */
  int code;

public:

  Response () = default;

  ~Response ()
  {
    if (resp != nullptr)
      MHD_destroy_response (resp);
  }

  Response (const Response&) = delete;
  void operator= (const Response&) = delete;

  /**
   * Constructs the response from given text and with the given content type.
   */
  void
  Set (const int c, const std::string& type, const std::string& body)
  {
    CHECK (resp == nullptr);

    code = c;

    void* data = const_cast<void*> (static_cast<const void*> (body.data ()));
    resp = MHD_create_response_from_buffer (body.size (), data,
                                            MHD_RESPMEM_MUST_COPY);
    CHECK (resp != nullptr);

    CHECK_EQ (MHD_add_response_header (resp, "Content-Type", type.c_str ()),
              MHD_YES);
  }

  /**
   * Enqueues the response for sending by MHD.
   */
  int
  Queue (struct MHD_Connection* conn)
  {
    CHECK (resp != nullptr);
    return MHD_queue_response (conn, code, resp);
  }

};

} // anonymous namespace

int
RestApi::RequestCallback (void* data, struct MHD_Connection* conn,
                          const char* url, const char* method,
                          const char* version,
                          const char* upload, size_t* uploadSize,
                          void** connData)
{
  LOG (INFO) << "REST server: " << method << " request to " << url;

  Response resp;
  try
    {
      if (std::string (method) != "GET")
        throw HttpError (MHD_HTTP_METHOD_NOT_ALLOWED, "only GET is supported");

      RestApi* self = static_cast<RestApi*> (data);
      const Json::Value res = self->Process (url);
      VLOG (1) << "REST response JSON: " << res;

      Json::StreamWriterBuilder wbuilder;
      wbuilder["commentStyle"] = "None";
      wbuilder["indentation"] = "";
      wbuilder["enableYAMLCompatibility"] = false;
      wbuilder["dropNullPlaceholders"] = false;
      wbuilder["useSpecialFloats"] = false;

      const std::string serialised = Json::writeString (wbuilder, res);
      resp.Set (MHD_HTTP_OK, "application/json", serialised);
    }
  catch (const HttpError& exc)
    {
      const int code = exc.GetStatusCode ();
      const std::string& msg = exc.what ();
      LOG (WARNING) << "Returning HTTP error " << code << ": " << msg;
      resp.Set (code, "text/plain", msg);
    }

  return resp.Queue (conn);
}

Json::Value
RestApi::Process (const std::string& url)
{
  if (url == "/state")
    {
      Json::Value res = logic.GetCustomStateData (game,
        [] (Database& db)
          {
            return Json::Value ();
          });
      res.removeMember ("data");
      return res;
    }

  const std::string prefixName = "/name/";
  if (url.substr (0, prefixName.size ()) == prefixName)
    {
      CHECK_GE (url.size (), prefixName.size ());
      const std::string name = url.substr (prefixName.size ());
      return logic.GetCustomStateData (game,
        [&name] (Database& db)
          {
            return GetNameState (db, name);
          });
    }

  throw HttpError (MHD_HTTP_NOT_FOUND, "invalid API endpoint");
}

void
RestApi::Start ()
{
  CHECK (daemon == nullptr);
  daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, port,
                             nullptr, nullptr,
                             &RequestCallback, this, MHD_OPTION_END);
  CHECK (daemon != nullptr) << "Failed to start microhttpd daemon";
}

void
RestApi::Stop ()
{
  CHECK (daemon != nullptr);
  MHD_stop_daemon (daemon);
  daemon = nullptr;
}

} // namespace xid
