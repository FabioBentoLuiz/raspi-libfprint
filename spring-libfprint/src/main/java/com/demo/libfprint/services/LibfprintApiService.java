package com.demo.libfprint.services;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

import com.demo.libfprint.configurations.LibfprintProperties;
import com.demo.libfprint.valueobjects.LibfprintMessage;

@Service
public class LibfprintApiService {

	private LibfprintProperties libApiProperties;
	
	@Autowired
	public LibfprintApiService(LibfprintProperties libApiProperties) {
		this.libApiProperties = libApiProperties;
	}
	
	public String startEnroll(Integer userId) {
		final String uri = this.libApiProperties.getUris().getEnroll();
		
		try {
		    RestTemplate restTemplate = new RestTemplate();
		    String result = restTemplate.postForObject(uri, new LibfprintMessage(userId.toString()), String.class);

		    return result;
		}catch(Exception e) {
			return e.getMessage() + " " + uri;
		}
		
	}

	public boolean startVerification(Integer userId) {
		final String uri = this.libApiProperties.getUris().getVerify();

	    RestTemplate restTemplate = new RestTemplate();
	    String result = restTemplate.postForObject(uri, new LibfprintMessage(userId.toString()), String.class);

	    return result.equals("OK");
	}

}
