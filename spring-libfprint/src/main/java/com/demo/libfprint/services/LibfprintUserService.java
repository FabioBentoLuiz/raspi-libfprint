package com.demo.libfprint.services;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.demo.libfprint.entities.User;
import com.demo.libfprint.repositories.FingerprintUserRepository;

@Service
public class LibfprintUserService {


	private FingerprintUserRepository userRepository;

	@Autowired
	public LibfprintUserService(FingerprintUserRepository userRepository) {
		this.userRepository = userRepository;
	}

	public int updateUserFingerprint(int userId, InputStream fingerprint) {

		if(fingerprint != null) {
			byte[] fpBytes = null;

			try {
				fpBytes = fingerprint.readAllBytes();
			} catch (IOException e) {
				System.err.println("Error reading fingerprint: " + e.getMessage());
				return 1;
			}

			User user = userRepository.findById(userId);
			user.setFingerprint(fpBytes);
			userRepository.save(user);

		}else {
			System.err.println("Fingerprint data stream is empty.");
			return 1;
		}

		return 0;
	}

	public User getUser(int userId) {
		User user = userRepository.findById(userId);

		if(user == null)
		{
			System.err.println("No user found with ID "+userId);
			return new User();
		}

		return user;
	}

	public byte[] getAllFingerprints() {
		Iterable<User> users = userRepository.findAll();

		ByteArrayOutputStream outputStream = new ByteArrayOutputStream( );

		for(User user : users)
		{
			try {
				outputStream.write(user.getFingerprint());
				//the \t is just a delimiter so it can be 'splited' by the identification service
				//TODO: find out a more elegant way to do so
				outputStream.write('\t');
			} catch (IOException e) {
				System.err.println("Stream error writing for user ID "+user.getId());
				return new byte[] {};
			}
			
		}

		return outputStream.toByteArray(); 
	}


}
