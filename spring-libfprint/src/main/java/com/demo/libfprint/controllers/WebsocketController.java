package com.demo.libfprint.controllers;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;

import com.demo.libfprint.valueobjects.LibfprintMessage;

@RestController
public class WebsocketController {
 
	private final SimpMessagingTemplate simpMessagingTemplate;
    
	@Autowired
	public WebsocketController(SimpMessagingTemplate simpMessagingTemplate) {
		this.simpMessagingTemplate = simpMessagingTemplate;
	}
 
	@RequestMapping(value ="/libfprintMessages", method = RequestMethod.POST)
    public void processMessageFromClient(@RequestBody LibfprintMessage message) throws Exception {
		
		DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
		
		if(message.getMessage().equals("DISCONNECT"))
			this.simpMessagingTemplate.convertAndSend("/queue/user-" + message.getUserId(), message.getMessage());
		else
			this.simpMessagingTemplate.convertAndSend("/queue/user-" + message.getUserId(), "[" + LocalDateTime.now().format(formatter) + "]: " + message.getMessage());
	    
	}

}
