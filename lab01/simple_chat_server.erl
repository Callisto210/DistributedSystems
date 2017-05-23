%% ----------------------------------
%% @author Patryk Duda
%% @copyright 2017
%% @doc
%% @end
%% ----------------------------------

-module(simple_chat_server).
-export([start/1]).

start(Port) -> 
	%%Creating group
	pg2:create(chat),
	%%Creating socket that listen to incoming connections
	{ok, ListenSocket} = gen_tcp:listen(Port, [binary, {packet, 0}, {active, false}, {reuseaddr, true}]),
	%%spawn(simple_chat_server, accept_loop, [ListenSocket]).
	spawn(fun() -> accept_loop(ListenSocket) end), %%Same as above, but don't have to export accept_loop
	spawn(fun() -> udp_loop(Port, []) end).
	
udp_loop(Port, UserLists) when is_number(Port) -> 
	{ok, UdpSocket} = gen_udp:open(Port, [binary, {active, true}]),
	udp_loop(UdpSocket, UserLists);
	
udp_loop(UdpSocket, UsersList) -> 
	receive
		{udp, UdpSocket, IP, InPortNo, Packet} ->
			case lists:member({IP, InPortNo}, UsersList) of
				false -> NewUsersList = [{IP, InPortNo} | UsersList],
						io:format("New UDP address ~n", []);
				true -> NewUsersList = UsersList
			end,
			case Packet of
				<< >> -> 
					udp_loop(UdpSocket, NewUsersList);
				_ -> 
					io:format("UDP multimedia received ~n", []),
					[ gen_udp:send(UdpSocket, Address, Port, Packet) || {Address, Port} <- NewUsersList, {Address, Port} =/= {IP, InPortNo} ],
					udp_loop(UdpSocket, NewUsersList)
			end;
		{error, _} -> failed
	end.
	
	
accept_loop(ListenSocket) ->
	{ok, Socket} = gen_tcp:accept(ListenSocket),
	spawn(fun() -> accept_loop(ListenSocket) end),
	io:format("New user connected!~n", []),
	pg2:join(chat, self()),
	ok = inet:setopts(Socket, [{active, true}]),
	chatter_loop(Socket).
	
chatter_loop(Socket) ->
	receive
		{send_data, Data} ->
			gen_tcp:send(Socket, Data),
			io:format("Data sent!~n", []),
			chatter_loop(Socket);
			
		{tcp, Socket, Data} ->
			io:format("Received from another process!~n", []),
			[Process ! {send_data, Data} || Process <- pg2:get_members(chat), Process =/= self()],
			chatter_loop(Socket);
			
		{tcp_closed, Socket} ->
			io:format("~p died!~n", [self()]),
			ok
	end.
