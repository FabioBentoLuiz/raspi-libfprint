package com.demo.libfprint.controllers;

import java.io.IOException;
import java.io.InputStream;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;

import com.demo.libfprint.entities.User;
import com.demo.libfprint.services.LibfprintApiService;
import com.demo.libfprint.services.LibfprintUserService;

@RestController
public class LibfprintDemoController {
	
	private LibfprintUserService userService;
	private LibfprintApiService libApiService;
	
	@Autowired
	public LibfprintDemoController(LibfprintUserService userService, LibfprintApiService libApiService) {
		this.userService = userService;
		this.libApiService = libApiService;
	}
	
	@RequestMapping(value ="/startEnroll", method = RequestMethod.GET)
	public String startEnroll(int userId) {
		return this.libApiService.startEnroll(userId);
	}
	
	@RequestMapping(value ="/saveFingerprint", method = RequestMethod.POST)
    public int saveFingerprint(@RequestHeader("user-id") int userId, InputStream fingerprint) {
		
		int result = this.userService.updateUserFingerprint(userId, fingerprint);
        return result;
    }
	
	
	//LibfprintMessage message
	@RequestMapping(value ="/getFingerprint", method = RequestMethod.GET)
	public byte[] getFingerprint(int userId) {
		User user = this.userService.getUser(userId);
		
		return user.getFingerprint();
	}
	
	@RequestMapping(value ="/startVerification", method = RequestMethod.GET)
	public String startVerification(int userId) {
		return this.libApiService.startVerification(userId);
	}
	
	@RequestMapping(value ="/startIdentification", method = RequestMethod.GET)
	public String startIdentification() {
		return this.libApiService.startIdentification();
	}
	
	@RequestMapping(value ="/getAllFingerprints", method = RequestMethod.GET)
	public byte[] getAllFingerprints() throws IOException {
		byte[] fingerprints = this.userService.getAllFingerprints(); 
		
		return fingerprints;
	}
	
	/*@MessageMapping("/sendMessage")
    @SendTo("/topic/greetings")
    public LibfprintMessage greeting(LibfprintMessage message) throws Exception {
        Thread.sleep(1000); // simulated delay
        return new LibfprintMessage("enroll");
    }*/
}
