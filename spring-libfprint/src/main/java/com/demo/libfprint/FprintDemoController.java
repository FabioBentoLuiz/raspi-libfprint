package com.demo.libfprint;

import java.io.IOException;
import java.io.InputStream;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.handler.annotation.SendTo;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.util.HtmlUtils;

@RestController
public class FprintDemoController {
	
	@Autowired 
	private FingerprintUserRepository userRepository;
	
	@RequestMapping(value ="/save", method = RequestMethod.POST)
    public void save(@RequestHeader("user-id") int userId, InputStream file) throws IOException {
		
        System.err.println("Post fingerprint..." );
        
        if(file != null) {
        	byte[] fingerprint = file.readAllBytes();
        	User user = userRepository.findById(userId);
        	user.setFingerprint(fingerprint);
    		userRepository.save(user);
        }
        
    }
	
	
	@MessageMapping("/hello")
    @SendTo("/topic/greetings")
    public CommandMessage greeting(CommandMessage message) throws Exception {
        Thread.sleep(1000); // simulated delay
        return new CommandMessage("enroll", "start");
    }
	
	@RequestMapping(value ="/startEnroll", method = RequestMethod.GET)
	public void startEnroll() {
		final String uri = "http://192.168.2.124:3000";

	    RestTemplate restTemplate = new RestTemplate();
	    String result = restTemplate.postForObject(uri, new CommandMessage("1", "1"), String.class);

	    System.out.println(result);
	}
	
	@RequestMapping(value ="/verify", method = RequestMethod.GET)
	public byte[] verify(int userId) {
		User user = userRepository.findById(userId);
		
		if(user != null)
			return user.getFingerprint();
		
		return new byte[]{};
	}
	
	@RequestMapping(value ="/startVerification", method = RequestMethod.GET)
	public void startVerification() {
		final String uri = "http://192.168.2.124:3000";

	    RestTemplate restTemplate = new RestTemplate();
	    String result = restTemplate.postForObject(uri, new CommandMessage("1", "1"), String.class);

	    System.out.println(result);
	}
}
