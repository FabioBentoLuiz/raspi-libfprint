package com.demo.libfprint.services;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

import com.demo.libfprint.configurations.LibfprintProperties;
import com.demo.libfprint.valueobjects.LibfprintMessage;

@Service
public class LibfprintApiService {

	private final LibfprintProperties libApiProperties;
	private final LibfprintUserService userService;
	
	@Autowired
	public LibfprintApiService(LibfprintProperties libApiProperties, LibfprintUserService userService) {
		this.libApiProperties = libApiProperties;
		this.userService = userService;
	}
	
	public String startEnroll(Integer userId) {
		final String uri = this.libApiProperties.getUris().getEnroll();
		
		try {
		    RestTemplate restTemplate = new RestTemplate();
		    LibfprintMessage msg = new LibfprintMessage();
		    msg.setUserId(userId.toString());
		    String result = restTemplate.postForObject(uri, msg, String.class);
		    
		    System.out.println("Starting enrollment for user ID "+msg.getUserId());

		    return result;
		}catch(Exception e) {
			return e.getMessage() + " " + uri;
		}
		
	}

	public String startVerification(Integer userId) {
		final String uri = this.libApiProperties.getUris().getVerify();

	    try {
	    	RestTemplate restTemplate = new RestTemplate();
		    LibfprintMessage msg = new LibfprintMessage();
		    msg.setUserId(userId.toString());
		    String result = restTemplate.postForObject(uri, msg, String.class);

		    return result;
	    }catch(Exception e) {
			return e.getMessage() + " " + uri;
		}
	}
	
	public String startIdentification() {
		final String uri = this.libApiProperties.getUris().getIdentify();

	    try {
	    	RestTemplate restTemplate = new RestTemplate();
	    	String fpSizes = this.userService.getEnrolledFpSizes();
	    	LibfprintMessage msg = new LibfprintMessage();
		    msg.setMessage(fpSizes);
		    String result = restTemplate.postForObject(uri, msg, String.class);

		    return result;
	    }catch(Exception e) {
			return e.getMessage() + " " + uri;
		}
	}

}
