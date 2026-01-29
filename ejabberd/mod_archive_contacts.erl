%% Copyright (C) 2025 The Xaya developers
%% Distributed under the MIT software license, see the accompanying
%% file COPYING or http://www.opensource.org/licenses/mit-license.php.

%% @doc ejabberd module that exposes a ``get_archive_contacts'' command.
%%
%% The command queries the MAM SQL archive for distinct bare JIDs a user
%% has exchanged messages with.  It is registered with ``policy = user'',
%% so callers must authenticate as the user they are querying — ejabberd
%% enforces this automatically via mod_http_api Basic-auth.

-module(mod_archive_contacts).

-behaviour(gen_mod).

%% gen_mod callbacks
-export([start/2, stop/1, reload/3, depends/2, mod_options/1, mod_doc/0]).

%% Command
-export([get_archive_contacts/2]).

-include("ejabberd_commands.hrl").
-include("logger.hrl").

%%--------------------------------------------------------------------
%% gen_mod callbacks
%%--------------------------------------------------------------------

start(_Host, _Opts) ->
    ejabberd_commands:register_commands(get_commands_spec()),
    ok.

stop(_Host) ->
    ejabberd_commands:unregister_commands(get_commands_spec()),
    ok.

reload(_Host, _NewOpts, _OldOpts) ->
    ok.

depends(_Host, _Opts) ->
    [].

mod_options(_Host) ->
    [].

mod_doc() ->
    #{desc => [<<"Query distinct contacts from the MAM archive.">>]}.

%%--------------------------------------------------------------------
%% Command specification
%%--------------------------------------------------------------------

get_commands_spec() ->
    [#ejabberd_commands{
        name = get_archive_contacts,
        tags = [mam],
        desc = "List distinct contacts from a user's MAM archive",
        module = ?MODULE,
        function = get_archive_contacts,
        args = [{user, binary}, {host, binary}],
        result = {contacts, {list, {contact, string}}},
        policy = user
    }].

%%--------------------------------------------------------------------
%% Command implementation
%%--------------------------------------------------------------------

get_archive_contacts(User, Host) ->
    EscUser = ejabberd_sql:escape(User),
    EscHost = ejabberd_sql:escape(Host),
    case ejabberd_sql:sql_query(Host,
             [<<"SELECT DISTINCT bare_peer FROM archive "
                "WHERE username='">>, EscUser,
              <<"' AND server_host='">>, EscHost, <<"'">>]) of
        {selected, _, Rows} ->
            [binary_to_list(Jid) || [Jid] <- Rows];
        {error, Reason} ->
            ?ERROR_MSG("mod_archive_contacts query failed: ~p", [Reason]),
            []
    end.
