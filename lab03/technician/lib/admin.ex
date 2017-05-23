defmodule Admin do

  def wait_loop (channel) do
	receive do
		{:basic_deliver, payload, meta} -> 
			IO.puts "[#{meta.routing_key}] #{payload}"
			wait_loop(channel)
		:die -> IO.puts "wait_loop died!"
				:ok
	end 
  end

  def start do
	{:ok, connection} = AMQP.Connection.open
    {:ok, channel} = AMQP.Channel.open(connection)
    
    AMQP.Exchange.declare(channel, "hospital", :topic)
    AMQP.Exchange.declare(channel, "admin", :fanout)
    
    {:ok, %{queue: queue_name}} = AMQP.Queue.declare(channel, "", exclusive: true)
    AMQP.Queue.bind(channel, queue_name, "hospital", routing_key: "#")
    AMQP.Basic.consume(channel, queue_name, nil, no_ack: true)
    
    pid = self()
    spawn(fn -> request_loop(channel, queue_name, pid) end)
    wait_loop(channel)
    AMQP.Connection.close(connection)
  end
  
  def request_loop(channel, queue_name, pid) do
	case IO.read(:line) |> String.trim() |> String.split (" ") do
		["q"] -> :quit
		["info", text] -> AMQP.Basic.publish(channel, "admin", "", text)
		_ -> :nothing
	end |> case do
		:quit -> send pid, :die
				 IO.puts "request_loop died!"
				 :ok
		_ -> request_loop(channel, queue_name, pid)
	end
  end
end
