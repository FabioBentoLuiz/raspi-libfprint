package com.demo.libfprint.controllers;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.handler.annotation.Payload;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Controller;

import com.demo.libfprint.valueobjects.LibfprintMessage;

@Controller
public class WebsocketController {
 
	private final SimpMessagingTemplate simpMessagingTemplate;
    
	@Autowired
	public WebsocketController(SimpMessagingTemplate simpMessagingTemplate) {
		this.simpMessagingTemplate = simpMessagingTemplate;
	}
 
	@MessageMapping("/topic")
    public void processMessageFromClient(@Payload LibfprintMessage message) throws Exception {
		
		DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
		
		this.simpMessagingTemplate.convertAndSend("/queue/chats-" + message.getId(), "[" + LocalDateTime.now().format(formatter) + "]:" + message.getParameter());
    }

}
