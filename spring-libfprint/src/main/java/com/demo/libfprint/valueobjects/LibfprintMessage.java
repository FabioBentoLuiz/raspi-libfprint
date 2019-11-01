package com.demo.libfprint.valueobjects;

public class LibfprintMessage {

	private String parameter;
	private String id;

	public LibfprintMessage() {

	}
	
	public LibfprintMessage(String parameter) {
		this.parameter = parameter;
	}
	
	public String getParameter() {
		return parameter;
	}

	public void setParameter(String parameter) {
		this.parameter = parameter;
	}

	public String getId() {
		return id;
	}

	public void setId(String id) {
		this.id = id;
	}

}
