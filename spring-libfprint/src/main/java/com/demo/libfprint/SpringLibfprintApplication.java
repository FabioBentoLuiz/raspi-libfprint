package com.demo.libfprint;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

import com.demo.libfprint.configurations.LibfprintProperties;

@SpringBootApplication
@EnableConfigurationProperties(LibfprintProperties.class)
public class SpringLibfprintApplication {

	public static void main(String[] args) {
		SpringApplication.run(SpringLibfprintApplication.class, args);
	}

}
