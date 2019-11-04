package com.demo.libfprint.repositories;

import org.springframework.data.repository.CrudRepository;
import org.springframework.stereotype.Repository;

import com.demo.libfprint.entities.User;

@Repository
public interface FingerprintUserRepository extends CrudRepository<User, Integer> {

	User findById(int id);
	long countByFingerprintNotNull();
}
