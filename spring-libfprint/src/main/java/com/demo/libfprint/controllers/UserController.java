package com.demo.libfprint.controllers;

import javax.validation.Valid;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;

import com.demo.libfprint.entities.User;
import com.demo.libfprint.services.LibfprintUserService;

@Controller
public class UserController {
	
	private LibfprintUserService userService;
	
	@Autowired
	public UserController(LibfprintUserService userService) {
		this.userService = userService;
	}
	
	@GetMapping("/")
    public String showSignUpForm(Model model) {
		model.addAttribute("users", userService.findAll());
        return "index";
    }
	
	@GetMapping("/new")
    public String showSignUpForm(User user) {
        return "create-user";
    }
	
	@GetMapping("/identify")
    public String identify(User user) {
        return "identify-user";
    }

	@PostMapping("/create-user")
    public String addUser(@Valid User user, BindingResult result, Model model) {
        if (result.hasErrors()) {
            return "create-user";
        }
         
        userService.save(user);
        model.addAttribute("users", userService.findAll());
        return "index";
    }
	
	@GetMapping("/user-enroll/{id}")
	public String showUpdateForm(@PathVariable("id") int id, Model model) {
	    User user = this.userService.getUser(id);
	     
	    model.addAttribute("user", user);
	    return "enroll-user";
	}
	
}
