defmodule Doctor do

  def wait_loop (channel) do
	receive do
		{:basic_deliver, payload, %{exchange: "admin"}} -> 
			IO.puts "Message form admin: #{payload}"
			wait_loop(channel)
	
		{:basic_deliver, payload, _} -> 
			IO.puts "#{payload}"
			wait_loop(channel)
		:die -> IO.puts "wait_loop died!"
				:ok
	end 
  end

  def start do
	{:ok, connection} = AMQP.Connection.open
    {:ok, channel} = AMQP.Channel.open(connection)
    
    AMQP.Exchange.declare(channel, "hospital", :topic)
    {:ok, %{queue: queue_name}} = AMQP.Queue.declare(channel, "", exclusive: true)
    AMQP.Queue.bind(channel, queue_name, "hospital", routing_key: "doc." <> queue_name)
    AMQP.Basic.consume(channel, queue_name, nil, no_ack: true)
    
    AMQP.Exchange.declare(channel, "admin", :fanout)
    {:ok, %{queue: queue_name_admin}} = AMQP.Queue.declare(channel, "", exclusive: true)
    AMQP.Queue.bind(channel, queue_name_admin, "admin")
    AMQP.Basic.consume(channel, queue_name_admin, nil, no_ack: true)
    
    pid = self()
    spawn(fn -> request_loop(channel, queue_name, pid) end)
    wait_loop(channel)
    AMQP.Connection.close(connection)
  end
  
  def request_loop(channel, queue_name, pid) do
	case IO.read(:line) |> String.trim() |> String.split (" ") do
		["q"] -> :quit
		["send", type, name] -> AMQP.Basic.publish(channel, "hospital", "tech." <> type, name, reply_to: "doc." <> queue_name)
		_ -> :nothing
	end |> case do
		:quit -> send pid, :die
				 IO.puts "request_loop died!"
				 :ok
		_ -> request_loop(channel, queue_name, pid)
	end
  end
end
