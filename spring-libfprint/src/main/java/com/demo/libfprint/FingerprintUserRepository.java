package com.demo.libfprint;

import org.springframework.data.repository.CrudRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface FingerprintUserRepository extends CrudRepository<User, Integer> {

	User findById(int id);
}
