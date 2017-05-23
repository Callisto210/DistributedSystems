defmodule Technician do

  def wait_loop (channel) do
	receive do
		{:basic_deliver, payload, %{exchange: "admin"}} -> 
			IO.puts "Message form admin: #{payload}"
			wait_loop(channel)
	
		{:basic_deliver, payload, meta} -> 
			IO.puts "[#{meta.routing_key}] #{payload}"
			response = case :rand.uniform(2) do
				1 -> "#{payload} - bad"
				2 -> "#{payload} - good"
			end
			AMQP.Basic.publish(channel, "hospital", meta.reply_to, "#{response}")
			AMQP.Basic.ack(channel, meta.delivery_tag) 
			wait_loop(channel)
	end 
  end


  def start (skills) do
    {:ok, connection} = AMQP.Connection.open
    {:ok, channel} = AMQP.Channel.open(connection)
    AMQP.Basic.qos(channel, [{:prefetch_count, 1}])
    
    AMQP.Exchange.declare(channel, "hospital", :topic)
    
    Enum.each(skills, fn skill -> 
		AMQP.Queue.declare(channel, skill, auto_delete: true)
		AMQP.Queue.bind(channel, skill, "hospital", routing_key: "tech." <> skill)
		AMQP.Basic.consume(channel, skill)
		IO.puts "Skill #{skill} registered!"

    end)
    
    
    AMQP.Exchange.declare(channel, "admin", :fanout)
    {:ok, %{queue: queue_name_admin}} = AMQP.Queue.declare(channel, "", exclusive: true)
    AMQP.Queue.bind(channel, queue_name_admin, "admin")
    AMQP.Basic.consume(channel, queue_name_admin, nil, no_ack: true)
    
    wait_loop(channel)
    AMQP.Connection.close(connection)
    
  end
end
