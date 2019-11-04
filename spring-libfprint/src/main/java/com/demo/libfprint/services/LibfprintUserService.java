package com.demo.libfprint.services;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.stream.Collectors;

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
		List<User> users = userRepository.findAllByFingerprintNotNull();

		ByteArrayOutputStream outputStream = new ByteArrayOutputStream( );

		for(int i = 0; i < users.size(); i++)
		{
			outputStream.writeBytes(users.get(i).getFingerprint());
		}

		return outputStream.toByteArray(); 
	}

	public void save(User user) {
		this.userRepository.save(user);
	}

	public Iterable<User> findAll() {
		return this.userRepository.findAll();
	}

	public String getEnrolledFpSizes() {
		List<User> users = userRepository.findAllByFingerprintNotNull();
		
		String fpSizes = users.stream()
				.map(u -> String.valueOf(u.getFingerprint().length))
				.collect(Collectors.joining(","));
		
		return fpSizes;
	}

	public User getUserByIndex(int index) {
		List<User> users = userRepository.findAllByFingerprintNotNull();
		return users.get(index);
	}


}
