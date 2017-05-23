package pl.edu.agh.dsrg.sr.chat;

/**
 * Created by lab on 3/23/2017.
 */
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.jgroups.JChannel;
import org.jgroups.Message;
import org.jgroups.ReceiverAdapter;
import org.jgroups.protocols.*;
import org.jgroups.protocols.pbcast.FLUSH;
import org.jgroups.protocols.pbcast.GMS;
import org.jgroups.protocols.pbcast.NAKACK2;
import org.jgroups.protocols.pbcast.STABLE;
import org.jgroups.protocols.pbcast.STATE_TRANSFER;
import org.jgroups.stack.ProtocolStack;

import com.google.protobuf.InvalidProtocolBufferException;
import com.sun.xml.internal.ws.resources.SenderMessages;

import pl.edu.agh.dsrg.sr.chat.protos.ChatOperationProtos.ChatMessage;

public class Chat extends ReceiverAdapter {

	public Set<Pair<String, String>> channels = Collections.synchronizedSet( new HashSet<Pair<String, String>>());
	private Management mgm;
	private String nickname;
	
	private Map<String, JChannel> activeChannels = new HashMap<String, JChannel>();
	private JChannel currentChannel = null;
	
	private void channelStack(ProtocolStack stack, String address) throws Exception {		
		UDP udp = new UDP();
        if(address != null) {
            InetAddress addr = InetAddress.getByName(address);
            if (!addr.isMulticastAddress()) {
                throw new UnknownHostException(addr + " not a multicast address");
            }
            udp.setValue("mcast_group_addr", addr);
        }
		
    	stack.addProtocol(udp)
        .addProtocol(new PING())
        .addProtocol(new MERGE3())
        .addProtocol(new FD_SOCK())
        .addProtocol(new FD_ALL().setValue("timeout", 12000).setValue("interval", 3000))
        .addProtocol(new VERIFY_SUSPECT())
        .addProtocol(new BARRIER())
        .addProtocol(new NAKACK2())
        .addProtocol(new UNICAST3())
        .addProtocol(new STABLE())
        .addProtocol(new GMS())
        .addProtocol(new UFC())
        .addProtocol(new MFC())
        .addProtocol(new FRAG2())
        .addProtocol(new STATE_TRANSFER())
        .addProtocol(new FLUSH());
    	stack.init();
    }
	
	private JChannel initChannel(String name) throws Exception{
    	JChannel channel = new JChannel(false);
    	ProtocolStack stack = new ProtocolStack();
    	channel.setProtocolStack(stack);
    	channelStack(stack, name);
    	return channel;
    }
	
	private void joinChannel(String name) throws Exception {
		JChannel channel = initChannel(name);
		channel.setName(nickname);
		channel.setReceiver(new ReceiverAdapter() {
			@Override
		    public void receive(Message msg) {
		    	try {
					ChatMessage m = ChatMessage.parseFrom(msg.getBuffer());
					System.out.println("[" + name + "] <" + msg.getSrc() + ">: " + m.getMessage());
				} catch (InvalidProtocolBufferException e) {
					e.printStackTrace();
				}
		    	
		    }
			
		});
    	channel.connect(name);
		activeChannels.put(name, channel);
		mgm.joinChannel(name);
	}
	
	private void leaveChannel(String name) throws Exception {
		mgm.leaveChannel(name);
		JChannel channel = activeChannels.get(name);
		if (channel == currentChannel) 
			currentChannel = null;
		channel.close();
		activeChannels.remove(name, channel);
	}

	private void setChannel(String name) {
		currentChannel = activeChannels.get(name);
		if(currentChannel == null) {
			System.out.println("Channel not found");
		}
	}
	
    private void sendMessage(String message) throws Exception {
    	if (currentChannel != null) {
	    	ChatMessage m = ChatMessage .newBuilder()
	    								.setMessage(message)
	    								.build();
	    	currentChannel.send(new Message(null, null, m.toByteArray()));
    	}
    	else
    		System.out.println("Channel not set");
    }
    
    private void getUsers() {
    	System.out.println("Channel | User");
    	for (Pair<String, String> u : channels) {
    		System.out.println(u.getLeft() + " | " + u.getRight());
    	}
    }

    public void start() throws Exception {
    	BufferedReader in=new BufferedReader(new InputStreamReader(System.in));
        System.out.println("Enter nickname:");
        nickname = in.readLine();

        mgm = new Management(channels, nickname);
        
        while(true) {
            try {
                System.out.print("> "); System.out.flush();
                String line=in.readLine();
                
                if(line.startsWith("quit") || line.startsWith("exit")) {
                	for(String channel : activeChannels.keySet())
                		leaveChannel(channel);
                	mgm.exit();
                    break;
                }
                
                if(line.startsWith("join ")) {
                	//System.out.println(line.substring(5));
                	joinChannel(line.substring(5));
                	continue;
                }
                
                if(line.startsWith("set ")) {
                	setChannel(line.substring(4));
                	continue;
                }
                
                if(line.startsWith("leave ")) {
                	
                	
                	leaveChannel(line.substring(6));
                	continue;
                }

                if(line.startsWith("view")) {
                	System.out.println("Active channels: " + activeChannels.size());
                	getUsers();
                	continue;
                }
                
                sendMessage(line);
            }
            catch(Exception e) {
            }
        
        
        }//while
    }
    
    public static void main(String[] args) throws Exception {
        System.setProperty("java.net.preferIPv4Stack","true");
        new Chat().start();
        
        
    }//main
}//class