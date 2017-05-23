package pl.edu.agh.dsrg.sr.chat;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;
import java.util.Set;

import org.jgroups.Address;
import org.jgroups.JChannel;
import org.jgroups.Message;
import org.jgroups.ReceiverAdapter;
import org.jgroups.View;
import org.jgroups.protocols.BARRIER;
import org.jgroups.protocols.FD_ALL;
import org.jgroups.protocols.FD_SOCK;
import org.jgroups.protocols.FRAG2;
import org.jgroups.protocols.MERGE3;
import org.jgroups.protocols.MFC;
import org.jgroups.protocols.PING;
import org.jgroups.protocols.UDP;
import org.jgroups.protocols.UFC;
import org.jgroups.protocols.UNICAST3;
import org.jgroups.protocols.VERIFY_SUSPECT;
import org.jgroups.protocols.pbcast.FLUSH;
import org.jgroups.protocols.pbcast.GMS;
import org.jgroups.protocols.pbcast.NAKACK2;
import org.jgroups.protocols.pbcast.STABLE;
import org.jgroups.protocols.pbcast.STATE_TRANSFER;
import org.jgroups.stack.ProtocolStack;

import com.google.protobuf.InvalidProtocolBufferException;

import pl.edu.agh.dsrg.sr.chat.protos.ChatOperationProtos.ChatAction;
import pl.edu.agh.dsrg.sr.chat.protos.ChatOperationProtos.ChatAction.ActionType;
import pl.edu.agh.dsrg.sr.chat.protos.ChatOperationProtos.ChatState;

public class Management extends ReceiverAdapter {
	
    private JChannel managementChannel;
    private Set<Pair<String, String>> channels; //Ten set jest thread safe
    private String nickname;
    private View prevView;
    
    private ProtocolStack mgmStack() throws Exception {
    	ProtocolStack stack = new ProtocolStack();
    	stack.addProtocol(new UDP())
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
        return stack;
    }

    private void initManagement(ProtocolStack stack) throws Exception{
    	managementChannel = new JChannel(false);
        managementChannel.setProtocolStack(stack);
        stack.init();
    }

    public Management(Set<Pair<String, String>> channels, String nickname) throws Exception {
    	this.channels = channels;
    	this.nickname = nickname;
    	initManagement(mgmStack());
    	managementChannel.setName(nickname);
        managementChannel.setReceiver(this);
    	managementChannel.connect("ChatManagement321321");
    	managementChannel.getState(null, 10000);
	}
    
    public void joinChannel(String channel) throws Exception {
    	ChatAction action = ChatAction.newBuilder()
							.setAction(ActionType.JOIN)
							.setChannel(channel)
							.setNickname(nickname)
							.build();
		managementChannel.send(new Message(null, null, action.toByteArray()));
		channels.add(new Pair<String, String> (channel, nickname));
	}
    
    public void leaveChannel(String channel) throws Exception {
    	ChatAction action = ChatAction.newBuilder()
							.setAction(ActionType.LEAVE)
							.setChannel(channel)
							.setNickname(nickname)
							.build();
		managementChannel.send(new Message(null, null, action.toByteArray()));
		channels.remove(new Pair<String, String> (channel, nickname));
    }
    
    public void exit() {
    	managementChannel.close();
    }
    
    @Override
    public void viewAccepted(View view) {
    	if (prevView != null) {
    		List<Address> left = View.leftMembers(prevView, view);
    		for(Address a : left) {
    			for(Pair<String, String> p : channels) {
    				if(p.getRight().equals(a.toString())) {
    					channels.remove(p);
    				}
    			}
    		}
    	}
    	
    	
    	prevView = view;
    }
    
    @Override
    public void receive(Message msg) {
    	ChatAction e;
		try {
			e = ChatAction.parseFrom(msg.getBuffer());
	    	if (e.getAction().equals(ActionType.JOIN))
	    		channels.add(new Pair<String, String> (e.getChannel(), e.getNickname()));
	    	else
	    	if (e.getAction().equals(ActionType.LEAVE))
	        	channels.remove(new Pair<String, String> (e.getChannel(), e.getNickname()));
	        else
	        	System.out.println("WTF?!");
		} catch (InvalidProtocolBufferException e1) {
			e1.printStackTrace();
		}
    }
    
    public void getState(OutputStream output) throws Exception {
        ChatState.Builder state = ChatState.newBuilder();
        
        for (Pair<String, String> e : channels) {
    		state.addState(ChatAction.newBuilder()
								.setAction(ActionType.JOIN)
								.setChannel(e.getLeft())
								.setNickname(e.getRight())
								.build()
								);
        }
        state.build().writeTo(output);
    }
    
    public void setState(InputStream input) throws Exception {
    	List<ChatAction> listOfActions = ChatState.parseFrom(input).getStateList();
    	for (ChatAction e : listOfActions) {
    		channels.add(new Pair<String, String> (e.getChannel(), e.getNickname()));
    	}
    }
}
