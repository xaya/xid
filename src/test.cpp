#include <light.hpp>

#include <string>

void
RunXidLight (const std::string& endpoint, const std::string& caFile,
             const int port)
{
  xid::LightInstance srv(endpoint, port);
  srv.EnableListenLocally ();
  srv.SetCaFile (caFile);
  srv.Run ();
}
