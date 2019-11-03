package com.demo.libfprint.controllers;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;

import com.demo.libfprint.entities.User;
import com.demo.libfprint.services.LibfprintUserService;
import com.demo.libfprint.valueobjects.LibfprintMessage;

@RestController
public class WebsocketController {
 
	private final SimpMessagingTemplate simpMessagingTemplate;
	private final LibfprintUserService userService;
    
	@Autowired
	public WebsocketController(SimpMessagingTemplate simpMessagingTemplate, LibfprintUserService userService) {
		this.simpMessagingTemplate = simpMessagingTemplate;
		this.userService = userService;
	}
 
	@RequestMapping(value ="/libfprintMessages", method = RequestMethod.POST)
    public void processMessageFromClient(@RequestBody LibfprintMessage message) throws Exception {
		
		DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
		
		if(message.getMessage().equals("DISCONNECT"))
			this.simpMessagingTemplate.convertAndSend("/queue/user-" + message.getUserId(), message.getMessage());
		else
			this.simpMessagingTemplate.convertAndSend("/queue/user-" + message.getUserId(), "[" + LocalDateTime.now().format(formatter) + "]: " + message.getMessage());
	    
	}
	
	@RequestMapping(value ="/libfprintIdentMessages", method = RequestMethod.POST)
    public void libfprintIdentMessages(@RequestBody LibfprintMessage message) throws Exception {
		
		DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
		
		int index = -1;
		String msg = message.getMessage();
		
		try {
			index = Integer.parseInt(message.getUserId());
		}catch(Exception e) {
			//ignore
		}
		
		if(index >= 0) {
			//that means that the identification service got a match at this index
			//lets find out who is this...
			User user = this.userService.getUserByIndex(index);
			msg = "MATCH - "+user.toString();
		}
		
		if(message.getMessage().equals("DISCONNECT"))
			this.simpMessagingTemplate.convertAndSend("/queue/user-0", msg);
		else
			this.simpMessagingTemplate.convertAndSend("/queue/user-0", "[" + LocalDateTime.now().format(formatter) + "]: " + msg);
	    
	}

}
